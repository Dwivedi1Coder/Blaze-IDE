const $ = (selector) => document.querySelector(selector);
const state = { workspace: null, openFiles: new Map(), activePath: null, settings: null };

async function nativeResult(promise) {
  const value = await promise;
  return typeof value === "string" ? JSON.parse(value) : value;
}

function log(message) {
  const line = document.createElement("div");
  line.className = "output-line";
  const stamp = document.createElement("span");
  stamp.className = "output-time";
  stamp.textContent = "[Blaze] ";
  line.append(stamp, document.createTextNode(message));
  $("#output-content").append(line);
}

function applySettings(settings) {
  state.settings = settings;
  document.documentElement.dataset.theme = settings.theme || "dark";
  document.documentElement.style.setProperty("--accent", settings.accent || "#ff7838");
  document.documentElement.style.setProperty("--font-size", (settings.fontSize || 14) + "px");
  document.documentElement.style.setProperty("--radius", (settings.radius ?? 8) + "px");
  const density = { compact: "0.82", comfortable: "1", spacious: "1.18" };
  document.documentElement.style.setProperty("--space", density[settings.density] || "1");
  let custom = document.getElementById("user-custom-styles");
  if (!custom) {
    custom = document.createElement("style");
    custom.id = "user-custom-styles";
    document.head.append(custom);
  }
  custom.textContent = settings.customCss || "";
  $("#theme-select").value = settings.theme || "dark";
  $("#accent-color").value = settings.accent || "#ff7838";
  $("#font-size").value = settings.fontSize || 14;
  $("#font-size-value").textContent = settings.fontSize || 14;
  $("#density").value = settings.density || "comfortable";
  $("#radius").value = settings.radius ?? 8;
  $("#custom-css").value = settings.customCss || "";
}

function loadSettings() {
  const defaults = { theme: "dark", accent: "#ff7838", fontSize: 14, density: "comfortable", radius: 8, customCss: "" };
  try { return { ...defaults, ...JSON.parse(localStorage.getItem("blaze-appearance") || "{}") }; }
  catch { return defaults; }
}

function iconFor(path) {
  const ext = path.split(".").pop().toLowerCase();
  if (["cpp", "cc", "c", "h", "hpp"].includes(ext)) return "C";
  if (ext === "md") return "M";
  if (ext === "json") return "{}";
  if (ext === "css") return "#";
  if (ext === "js") return "JS";
  return "·";
}

function renderFiles() {
  const tree = $("#file-tree");
  tree.replaceChildren();
  if (!state.workspace || !state.workspace.files.length) {
    tree.innerHTML = '<div class="empty-state">No files found in this workspace.</div>';
    return;
  }
  const files = [...state.workspace.files].sort((a, b) => a.path.localeCompare(b.path));
  for (const file of files) {
    const item = document.createElement("div");
    item.className = "tree-item" + (file.path === state.activePath ? " active" : "");
    const depth = Math.max(0, file.path.split("/").length - 1);
    for (let i = 0; i < depth; i++) {
      const indent = document.createElement("span");
      indent.className = "tree-indent";
      item.append(indent);
    }
    const glyph = document.createElement("span");
    glyph.className = "file-glyph";
    glyph.textContent = iconFor(file.path);
    const label = document.createElement("span");
    label.textContent = file.path.split("/").pop();
    label.title = file.path;
    item.append(glyph, label);
    item.addEventListener("click", () => openFile(file.path));
    tree.append(item);
  }
}

function renderTabs() {
  const strip = $("#tab-strip");
  strip.replaceChildren();
  for (const [path] of state.openFiles) {
    const tab = document.createElement("div");
    tab.className = "tab" + (path === state.activePath ? " active" : "");
    const glyph = document.createElement("span");
    glyph.className = "file-icon cpp";
    glyph.textContent = iconFor(path);
    const name = document.createElement("span");
    name.textContent = path.split("/").pop();
    const close = document.createElement("button");
    close.className = "tab-close";
    close.textContent = "×";
    close.title = "Close tab";
    close.addEventListener("click", (event) => {
      event.stopPropagation();
      state.openFiles.delete(path);
      if (state.activePath === path) state.activePath = [...state.openFiles.keys()].pop() || null;
      renderTabs();
      renderActiveEditor();
    });
    tab.append(glyph, name, close);
    tab.addEventListener("click", () => { state.activePath = path; renderTabs(); renderActiveEditor(); });
    strip.append(tab);
  }
  if (!state.openFiles.size) {
    const welcome = document.createElement("div");
    welcome.className = "tab active";
    welcome.textContent = "Welcome";
    strip.append(welcome);
  }
}

function renderActiveEditor() {
  const host = $("#editor-content");
  host.replaceChildren();
  const file = state.openFiles.get(state.activePath);
  if (!file) {
    host.innerHTML = '<div class="welcome-screen"><div class="welcome-logo">ϟ</div><h1>Welcome to <span>Blaze-IDE</span></h1><p>Open a file from the Project panel or create a new one.</p><div class="welcome-actions"><button class="primary-button" id="welcome-new">Create New File</button></div></div>';
    $("#welcome-new").addEventListener("click", showNewFileDialog);
    $("#breadcrumbs").innerHTML = "Blaze-IDE <span>›</span> Welcome";
    return;
  }
  const editor = document.createElement("textarea");
  editor.className = "editor-textarea";
  editor.spellcheck = false;
  editor.value = file.content;
  editor.setAttribute("aria-label", "Source editor");
  editor.addEventListener("input", () => { file.content = editor.value; file.dirty = true; updateCursor(); });
  editor.addEventListener("keydown", (event) => {
    if (event.key === "Tab") {
      event.preventDefault();
      const start = editor.selectionStart;
      editor.setRangeText("    ", start, editor.selectionEnd, "end");
      file.content = editor.value;
      file.dirty = true;
    }
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === "s") {
      event.preventDefault();
      saveActiveFile();
    }
  });
  host.append(editor);
  $("#breadcrumbs").innerHTML = "Blaze-IDE <span>›</span> " + escapeHtml(state.activePath);
  updateCursor();
}

function escapeHtml(value) {
  return value.replace(/[&<>"']/g, (char) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[char]));
}

function updateCursor() {
  const editor = document.querySelector(".editor-textarea");
  if (!editor) return;
  const before = editor.value.slice(0, editor.selectionStart);
  const line = before.split("\n").length;
  const column = before.length - before.lastIndexOf("\n");
  $("#cursor-status").textContent = "Ln " + line + ", Col " + column;
}

async function openFile(path) {
  if (!state.openFiles.has(path)) {
    try {
      const response = await nativeResult(window.nativeReadFile(path));
      if (!response.ok) throw new Error(response.error || "Unable to read file");
      state.openFiles.set(path, { content: response.content, dirty: false });
    } catch (error) {
      log("Open failed: " + error.message);
      return;
    }
  }
  state.activePath = path;
  renderFiles();
  renderTabs();
  renderActiveEditor();
  log("Opened " + path);
}

async function saveActiveFile() {
  const file = state.openFiles.get(state.activePath);
  if (!file || !state.activePath) return;
  try {
    const response = await nativeResult(window.nativeWriteFile(state.activePath, file.content));
    if (!response.ok) throw new Error(response.error || "Unable to save file");
    file.dirty = false;
    log("Saved " + state.activePath);
    $("#statusbar")?.classList.remove("error");
  } catch (error) {
    log("Save failed: " + error.message);
  }
}

async function refreshWorkspace() {
  try {
    state.workspace = await nativeResult(window.nativeGetWorkspace());
    if (!state.workspace.ok) throw new Error(state.workspace.error || "Workspace unavailable");
    $("#workspace-title").textContent = state.workspace.root;
    $("#project-name").textContent = state.workspace.name || "Workspace";
    $("#connection-status").textContent = "C++ core connected";
    renderFiles();
    log("Workspace loaded: " + state.workspace.name);
  } catch (error) {
    $("#connection-status").textContent = "Native bridge unavailable";
    log("Workspace error: " + error.message);
  }
}

function showNewFileDialog() {
  $("#new-file-modal").classList.remove("hidden");
  $("#new-file-name").focus();
}
function closeNewFileDialog() {
  $("#new-file-modal").classList.add("hidden");
  $("#new-file-name").value = "";
}
async function createFile() {
  const path = $("#new-file-name").value.trim().replace(/\\/g, "/");
  if (!path) return;
  try {
    const response = await nativeResult(window.nativeWriteFile(path, ""));
    if (!response.ok) throw new Error(response.error || "Could not create file");
    closeNewFileDialog();
    await refreshWorkspace();
    await openFile(path);
    log("Created " + path);
  } catch (error) { log("Create failed: " + error.message); }
}

function openSettings() { $("#settings-modal").classList.remove("hidden"); }
function closeSettings() { $("#settings-modal").classList.add("hidden"); }
function applySettingsFromForm() {
  const settings = {
    theme: $("#theme-select").value,
    accent: $("#accent-color").value,
    fontSize: Number($("#font-size").value),
    density: $("#density").value,
    radius: Number($("#radius").value),
    customCss: $("#custom-css").value
  };
  localStorage.setItem("blaze-appearance", JSON.stringify(settings));
  applySettings(settings);
  closeSettings();
  log("Appearance preferences applied and saved locally.");
}

function bindUI() {
  $("#settings-button").addEventListener("click", openSettings);
  $("#rail-settings").addEventListener("click", openSettings);
  $("#close-settings").addEventListener("click", closeSettings);
  $("#save-settings").addEventListener("click", applySettingsFromForm);
  $("#reset-settings").addEventListener("click", () => {
    localStorage.removeItem("blaze-appearance");
    applySettings(loadSettings());
    log("Appearance settings reset.");
  });
  $("#font-size").addEventListener("input", () => $("#font-size-value").textContent = $("#font-size").value);
  $("#theme-toggle").addEventListener("click", () => {
    const next = { ...state.settings, theme: state.settings.theme === "dark" ? "light" : "dark" };
    applySettings(next);
    localStorage.setItem("blaze-appearance", JSON.stringify(next));
  });
  $("#new-file-button").addEventListener("click", showNewFileDialog);
  $("#welcome-new").addEventListener("click", showNewFileDialog);
  $("#close-new-file").addEventListener("click", closeNewFileDialog);
  $("#cancel-new-file").addEventListener("click", closeNewFileDialog);
  $("#create-file").addEventListener("click", createFile);
  $("#new-file-name").addEventListener("keydown", (event) => { if (event.key === "Enter") createFile(); });
  $("#save-button").addEventListener("click", saveActiveFile);
  $("#refresh-files").addEventListener("click", refreshWorkspace);
  $("#hide-bottom").addEventListener("click", () => {
    const panel = $("#bottom-panel");
    panel.style.display = panel.style.display === "none" ? "" : "none";
  });
  $("#build-button").addEventListener("click", () => log("Build orchestration will be connected to the C++ toolchain service in the next milestone."));
  $("#run-button").addEventListener("click", () => log("Run configuration support is planned for the build-system milestone."));
  $("#open-file-button").addEventListener("click", () => log("Choose a file from the Project explorer to open it."));
  $("#quick-search").addEventListener("input", (event) => {
    const query = event.target.value.toLowerCase();
    document.querySelectorAll(".tree-item").forEach((item) => {
      item.style.display = item.textContent.toLowerCase().includes(query) ? "" : "none";
    });
  });
  document.addEventListener("keydown", (event) => {
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === ",") openSettings();
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === "n") showNewFileDialog();
  });
}

applySettings(loadSettings());
bindUI();
refreshWorkspace();
