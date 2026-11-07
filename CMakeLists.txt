cmake_minimum_required(VERSION 3.20)
project(BlazeIDE VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(wxWidgets 3.2 REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(BlazeIDE
    src/main.cpp
    src/ui/MainFrame.cpp
    src/ui/EditorPage.cpp
)

target_include_directories(BlazeIDE PRIVATE src)
target_link_libraries(BlazeIDE PRIVATE ${wxWidgets_LIBRARIES})

if(MSVC)
    target_compile_options(BlazeIDE PRIVATE /W4 /permissive-)
else()
    target_compile_options(BlazeIDE PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(BlazeIDE PROPERTIES
    OUTPUT_NAME "Blaze-IDE"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
