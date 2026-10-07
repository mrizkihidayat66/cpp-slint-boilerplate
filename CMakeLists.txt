cmake_minimum_required(VERSION 3.21)

project(
    cpp_slint_boilerplate
    VERSION 0.1.0
    DESCRIPTION "Professional C++20 + Slint desktop application boilerplate"
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
# Application executable
# ---------------------------------------------------------------------------

add_executable(slint-boilerplate
    src/main.cpp
    src/app/Application.cpp
    src/app/ServiceLocator.cpp
    src/services/DashboardService.cpp
    src/services/AboutService.cpp)

target_include_directories(slint-boilerplate
    PRIVATE
        src
        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(slint-boilerplate PRIVATE Slint::Slint)

# Compiles every .slint file reachable from app-window.slint (theme,
# components, pages are imported transitively) into generated C++.
slint_target_sources(slint-boilerplate PRIVATE src/ui/app-window.slint)

include(CMake/CompilerWarnings.cmake)
target_apply_warnings(slint-boilerplate)

# ---------------------------------------------------------------------------
# Unit tests (models/services only — UI correctness is enforced by
# slint_target_sources compiling the markup at build time)
# ---------------------------------------------------------------------------

option(BOILERPLATE_BUILD_TESTS "Build unit tests" ON)
if(BOILERPLATE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
