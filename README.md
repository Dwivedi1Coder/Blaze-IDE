# Blaze-IDE

Blaze-IDE is a native C/C++ development environment built primarily in C++20.
The project follows the requirements in `Prompt.md` and is being developed in
small, testable milestones.

## Current milestone: native shell

Implemented in this starter milestone:

- Native wxWidgets desktop window.
- Resizable project-folder explorer and editor/output split layout.
- Multiple editor tabs.
- Open, create, save, save-as, and save-all text files.
- Basic File/Edit/Help menus and status bar.
- CMake build configuration.

Not implemented yet: C/C++ syntax highlighting, code completion, project
wizard, compiler discovery, CMake build orchestration, terminal emulation,
debugging, diagnostics, indexing, and extension support. These are planned
milestones, not completed features.

## Dependencies

- CMake 3.20 or newer
- A C++20 compiler (MSVC 2022, GCC 11+, or Clang 14+ recommended)
- wxWidgets 3.2 or newer (core and base components)
- Ninja (recommended; another CMake generator can be selected manually)

## Build

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

On Windows with a Visual Studio generator:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug --parallel
```

If CMake cannot locate wxWidgets, install the wxWidgets development package
for your platform or provide `wxWidgets_ROOT_DIR` / `CMAKE_PREFIX_PATH`.

## Platform package hints

- Ubuntu/Debian: `sudo apt install build-essential cmake ninja-build libwxgtk3.2-dev`
- macOS (Homebrew): `brew install cmake ninja wxwidgets`
- Windows: install wxWidgets 3.2 using the package manager/toolchain used by
  your compiler, then configure CMake with its package prefix.

## Project structure

```text
src/
  main.cpp
  ui/
    EditorPage.h
    EditorPage.cpp
    MainFrame.h
    MainFrame.cpp
CMakeLists.txt
CMakePresets.json
Prompt.md
```

## Brand colors

- Light primary: `#D64F15`; light background: `#FCFBFA`
- Dark primary: `#DB7916`; dark background: `#1F1D1B`

The initial shell currently uses native platform widget defaults. The theme
system and branded styling are planned for a later milestone.

## License

A project license has not been selected yet.
