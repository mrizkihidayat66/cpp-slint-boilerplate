cmake_minimum_required(VERSION 3.21)

project(
    cpp_slint_boilerplate
    VERSION 0.1.0
    DESCRIPTION "C++20 + Slint desktop application — simple example"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

include(CMake/FetchSlint.cmake)

# ---------------------------------------------------------------------------
# Generated UI code — built as its own target so the warning policy below
# judges our own sources only: Slint's generated C++ does not survive /WX.
# ---------------------------------------------------------------------------

add_library(boilerplate-ui STATIC)
slint_target_sources(boilerplate-ui src/ui/app-window.slint NAMESPACE app)
target_link_libraries(boilerplate-ui PUBLIC Slint::Slint)

# ---------------------------------------------------------------------------
# Application executable — main.cpp plus the generated UI code
# ---------------------------------------------------------------------------

add_executable(slint-boilerplate src/main.cpp)
target_include_directories(slint-boilerplate PRIVATE src)
target_link_libraries(slint-boilerplate PRIVATE boilerplate-ui)

# Slint's headers and the generated app-window.h are third-party: pull them in
# as SYSTEM includes so the warning flags judge our own code only.
get_target_property(_slint_include_dirs Slint::Slint INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(slint-boilerplate SYSTEM PRIVATE
    ${_slint_include_dirs}
    ${CMAKE_CURRENT_BINARY_DIR})

include(CMake/CompilerWarnings.cmake)
target_apply_warnings(slint-boilerplate)

# Copy the Slint runtime DLL next to the exe so it runs from the build dir
# without a custom PATH.
add_custom_command(
    TARGET slint-boilerplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:slint_cpp-shared>
        $<TARGET_FILE_DIR:slint-boilerplate>)
