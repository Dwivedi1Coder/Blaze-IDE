# Blaze-IDE

Blaze-IDE is a native desktop C/C++ development environment with a C++20 application core and a highly customizable HTML/CSS/JavaScript presentation layer embedded in the operating system's WebView.

The UI is not a website and does not run as a separate browser application. The native executable owns the window, workspace access, and application lifecycle. The frontend communicates with native services through a narrow WebView binding layer.

## Architecture

- **C++20 core:** workspace discovery, filesystem operations, native integration, and future compiler/debugger/Git services.
- **HTML/CSS/JavaScript UI:** layout, editor presentation, panels, themes, settings, and interaction.
- **Native WebView:** WebView2 on Windows, WebKit on macOS, and WebKitGTK on Linux through [webview/webview](https://github.com/webview/webview).
- **CMake:** dependency and build orchestration.

## Current milestone: CSS-driven native shell

Implemented:

- Native desktop window backed by the platform WebView.
- CSS design tokens for colors, surfaces, borders, typography, density, and radii.
- Dark, light, and high-contrast themes.
- Appearance dialog with accent color, font size, density, corner radius, and custom CSS.
- Persistent appearance preferences in local storage.
- Project file tree populated by the C++ workspace service.
- Open, create, read, and save workspace files through native C++ bindings.
- Workspace path validation that rejects absolute paths and traversal outside the workspace.
- Output panel and editor tabs.

Not yet implemented: syntax-aware code editing, language server integration, compiler discovery, real build/run execution, terminal emulation, debugging, Git workflows, plugin loading, and workspace picker. The Build and Run toolbar items are intentionally informational placeholders until those native services exist.

## Build requirements

- CMake 3.20+
- C++20 compiler (Visual Studio 2022, GCC 11+, or Clang 14+ recommended)
- Git (CMake FetchContent downloads webview 0.12.0)
- Platform WebView dependencies:
  - Windows: Windows 10 SDK and Microsoft Edge WebView2 Runtime
  - macOS: Xcode command-line tools and WebKit framework
  - Linux: GTK and WebKitGTK development packages

Linux (Debian/Ubuntu, WebKitGTK 4.1):

```sh
sudo apt install build-essential cmake ninja-build pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Windows (Visual Studio 2022):

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

macOS:

```sh
brew install cmake ninja
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The first configure downloads the pinned WebView dependency. A connected build environment is required for that first configure unless the dependency is already cached.

## Project layout

```text
src/
  main.cpp
  core/
    ApplicationCore.h
    ApplicationCore.cpp
    NativeBridge.h
    NativeBridge.cpp
ui/
  index.html
  styles.css
  app.js
CMakeLists.txt
CMakePresets.json
```

## Customizing the UI

Open **Settings → Appearance** in Blaze-IDE. User preferences are saved locally. The Custom CSS editor accepts CSS selectors and variables; use it to override any UI component.

Example:

```css
:root {
  --accent: #8b5cf6;
  --radius: 12px;
  --panel: #17142a;
}
.tab.active {
  box-shadow: inset 0 2px var(--accent);
}
```

## Security boundary

The native bridge exposes only specific workspace operations. Paths must be relative and resolve inside the selected workspace root. Do not expose arbitrary shell execution or unrestricted filesystem APIs to the UI bridge.

## License

A project license has not been selected yet.
