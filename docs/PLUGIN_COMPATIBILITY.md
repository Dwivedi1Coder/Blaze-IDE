# Blaze IDE Plugin Architecture and JetBrains Compatibility

## Goal

Blaze IDE should have a professional plugin experience comparable to CLion: discoverable extensions, a plugin manager, clear compatibility information, safe installation, enable/disable controls, updates, and a documented developer SDK.

The plugin system must fit the existing architecture:

- **HTML/CSS/JavaScript:** Plugin Center UI, settings, and presentation contributions.
- **C++ core:** Plugin discovery, package validation, lifecycle, permissions, and native services.
- **Optional Python services:** Automation and developer tooling, never required for basic IDE startup.
- **Language/tool protocols:** LSP and DAP integrations for language intelligence and debugging.

## Important: CLion plugins are not drop-in extensions

CLion is built on the IntelliJ Platform. Its plugins are JVM/Java or Kotlin plugins that declare dependencies in `plugin.xml` and expect IntelliJ Platform services, extension points, and sometimes CLion-specific APIs.

A native C++/WebView application cannot load a JetBrains plugin ZIP/JAR directly. Renaming, unpacking, or copying a CLion plugin into a Blaze plugins directory does not make it compatible.

JetBrains documents CLion plugin development, product dependencies, and the CLion-specific modules here:

- [CLion Plugin Development](https://plugins.jetbrains.com/docs/intellij/clion.html)
- [Plugin Compatibility with IntelliJ Platform Products](https://plugins.jetbrains.com/docs/intellij/plugin-compatibility.html)
- [Develop plugins for CLion](https://www.jetbrains.com/help/clion/develop-plugins-for-clion.html)

CLion plugins may depend on modules such as `com.intellij.modules.clion`, `com.intellij.modules.cidr.lang`, and `com.intellij.modules.cidr.debugger`. The C/C++ language engine and internal APIs also vary by CLion version. Therefore compatibility must be checked per plugin and per target version.

## Compatibility strategy

### Tier 1 — Native tool integrations (first priority)

Integrate widely used C/C++ tools through documented protocols and process interfaces:

- clangd through Language Server Protocol (LSP)
- LLDB and GDB through Debug Adapter Protocol (DAP) or dedicated debugger adapters
- CMake and Ninja through their command-line/build interfaces
- Git through the Git executable and repository APIs
- clang-format and clang-tidy through their command-line interfaces

These integrations provide substantial CLion-like functionality without pretending to load JetBrains plugins.

### Tier 2 — Blaze Extension SDK

Create a Blaze-native extension format with:

- A versioned manifest and stable plugin identifier
- Name, version, author, description, license, homepage, and minimum Blaze version
- Declared capabilities and permissions
- UI contributions (commands, panels, themes, editor decorations)
- Tool integrations (LSP, DAP, formatter, build-system adapters)
- Install, update, disable, enable, and uninstall lifecycle
- Signature/checksum verification and clear trust prompts

Untrusted extensions must not receive unrestricted filesystem or shell access. Native extensions, if supported later, must use a documented ABI and explicit user consent. Prefer isolated processes or WebAssembly/WASI for third-party executable code rather than loading arbitrary DLLs into the IDE process.

### Tier 3 — JetBrains plugin compatibility (research / optional runtime)

A future compatibility mode could host an IntelliJ Platform runtime in a separate JVM process and communicate with Blaze through a versioned RPC bridge. This is a substantial separate subsystem, not a small plugin loader.

Before implementing it, evaluate:

1. Which IntelliJ Platform distribution and version can legally and technically be embedded or launched.
2. Which platform APIs are available without CLion's proprietary product modules.
3. Whether a specific plugin depends on CLion/CIDR modules or proprietary C/C++ language engine APIs.
4. Plugin licenses, JetBrains Marketplace terms, redistribution restrictions, and runtime distribution requirements.
5. Memory, startup time, process isolation, update, and security costs.

Do not claim generic CLion Marketplace compatibility. The UI must label plugins as **Blaze native**, **protocol integration**, **IntelliJ Platform compatible (verified)**, or **unsupported** based on actual validation.

## Proposed Plugin Center UX

The Plugins tool window should include:

- Marketplace and Installed tabs
- Search, category filters, and sort controls
- Plugin details: description, publisher, version, license, permissions, dependencies, and compatibility
- Install / Update / Enable / Disable / Uninstall actions
- Local package installation
- Restart-required indicators
- A link to plugin logs and diagnostics
- A setting to allow or block untrusted publishers

A plugin must never be shown as installed or enabled until the native plugin service confirms the operation succeeded.

## Delivery plan

1. Define the manifest schema, package format, compatibility labels, and security model.
2. Implement C++ plugin discovery and metadata validation.
3. Add installed-plugin state and lifecycle operations.
4. Build the HTML/CSS Plugin Center against those native operations.
5. Add the LSP/DAP integration host and first-party C/C++ integrations.
6. Publish the extension SDK and sample plugin.
7. Research an optional isolated IntelliJ Platform runtime only after the native plugin system is stable.

## Current implementation status

This document is an architecture decision and implementation plan. The current CSS/WebView milestone does **not** yet contain a plugin loader, Marketplace client, JVM runtime, or CLion plugin compatibility layer. Build and runtime validation remain separate tasks.
