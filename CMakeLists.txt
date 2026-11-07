cmake_minimum_required(VERSION 3.20)
project(BlazeIDE VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
set(WEBVIEW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WEBVIEW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WEBVIEW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(WEBVIEW_ENABLE_CHECKS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview
    GIT_TAG 0.12.0
)
FetchContent_MakeAvailable(webview)

add_executable(BlazeIDE WIN32
    src/main.cpp
    src/core/ApplicationCore.cpp
    src/core/NativeBridge.cpp
)

target_include_directories(BlazeIDE PRIVATE src)
target_link_libraries(BlazeIDE PRIVATE webview::core)
target_compile_definitions(BlazeIDE PRIVATE
    BLAZE_UI_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ui"
    BLAZE_DEFAULT_WORKSPACE="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(MSVC)
    target_compile_options(BlazeIDE PRIVATE /W4 /permissive-)
else()
    target_compile_options(BlazeIDE PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(BlazeIDE PROPERTIES
    OUTPUT_NAME "Blaze-IDE"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
