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
# Application executable — the whole app is main.cpp + generated UI code
# ---------------------------------------------------------------------------

add_executable(slint-boilerplate src/main.cpp)
target_include_directories(slint-boilerplate PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(slint-boilerplate PRIVATE Slint::Slint)

# Compiles every .slint file reachable from app-window.slint (theme,
# components, pages are imported transitively) into generated C++.
slint_target_sources(slint-boilerplate src/ui/app-window.slint NAMESPACE app)

include(CMake/CompilerWarnings.cmake)
target_apply_warnings(slint-boilerplate)

# Copy the Slint runtime DLL next to the exe so it runs from the build dir
# without a custom PATH.
add_custom_command(
    TARGET slint-boilerplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:slint_cpp-shared>
        $<TARGET_FILE_DIR:slint-boilerplate>)
