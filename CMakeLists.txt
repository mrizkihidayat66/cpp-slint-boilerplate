cmake_minimum_required(VERSION 3.21)

project(
    cpp_slint_boilerplate
    VERSION 0.1.0
    DESCRIPTION "Professional C++20 + Slint desktop application boilerplate"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Application identity — the single place to rename the app.
#
# APP_TECHNICAL_NAME is the machine-facing name (window title, About card,
# %APPDATA% directory); APP_DISPLAY_NAME is the short brand in the UI header.
# Both reach C++ as compile definitions and, through the AppInfo global, the
# .slint surfaces. The version comes from PROJECT_VERSION above — do not
# hardcode it again.
# ---------------------------------------------------------------------------
set(APP_TECHNICAL_NAME "cpp-slint-boilerplate")
set(APP_DISPLAY_NAME "Boilerplate")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

include(CMake/FetchSlint.cmake)

# ---------------------------------------------------------------------------
# Core library (everything except main.cpp) — linked by the app and the tests
# ---------------------------------------------------------------------------

add_library(boilerplate-core STATIC
    src/app/Application.cpp
    src/app/ServiceLocator.cpp
    src/models/DashboardModels.cpp
    src/models/TaskModels.cpp
    src/services/DashboardService.cpp
    src/services/AboutService.cpp
    src/services/SettingsService.cpp
    src/services/TaskRepository.cpp
    src/services/TaskService.cpp
    src/services/TaskValidationService.cpp)

target_include_directories(boilerplate-core PUBLIC src)

# PUBLIC: AppMetadata.h is a public header of this target and bakes these values
# in as macros, so consumers (the app, the tests) must see the same defines.
target_compile_definitions(boilerplate-core PUBLIC
    APP_TECHNICAL_NAME="${APP_TECHNICAL_NAME}"
    APP_DISPLAY_NAME="${APP_DISPLAY_NAME}"
    APP_VERSION="${PROJECT_VERSION}")

# Compiles every .slint file reachable from app-window.slint (theme,
# components, pages are imported transitively) into generated C++.
#
# The generated code is built as its own target so the warning policy in
# CMake/CompilerWarnings.cmake judges our sources only: Slint's generated C++
# does not survive -Werror / /WX, and that policy deliberately leaves
# third-party code alone.
add_library(boilerplate-ui STATIC)
slint_target_sources(boilerplate-ui src/ui/app-window.slint NAMESPACE app)
target_link_libraries(boilerplate-ui PUBLIC Slint::Slint)

target_link_libraries(boilerplate-core PUBLIC boilerplate-ui)

# Slint's headers and the generated app-window.h are third-party: pull them in
# as SYSTEM includes so -Wall/-Wextra and /W4 judge our own code only.
get_target_property(_slint_include_dirs Slint::Slint INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(boilerplate-core SYSTEM PUBLIC
    ${_slint_include_dirs}
    ${CMAKE_CURRENT_BINARY_DIR})

include(CMake/CompilerWarnings.cmake)
target_apply_warnings(boilerplate-core)

# std::getenv() is used read-only (settings path, start page); the MSVC CRT
# deprecation warning does not apply to those calls.
if(MSVC)
    target_compile_definitions(boilerplate-core PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ---------------------------------------------------------------------------
# Application executable
# ---------------------------------------------------------------------------

# WIN32 subsystem: no console window popping up alongside the app. This
# matters for capture-ui.ps1 too — the console belongs to the same PID and
# could win the "first visible window" race, capturing cmd instead of the UI.
add_executable(slint-boilerplate WIN32 src/main.cpp)
# The windows subsystem expects WinMain; keep the plain main() entry point.
if(MSVC)
    target_link_options(slint-boilerplate PRIVATE "/ENTRY:mainCRTStartup")
endif()
target_link_libraries(slint-boilerplate PRIVATE boilerplate-core)

# Copy the Slint runtime DLL next to the exe so it runs from the build dir
# without a custom PATH.
add_custom_command(
    TARGET slint-boilerplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:slint_cpp-shared>
        $<TARGET_FILE_DIR:slint-boilerplate>)


# ---------------------------------------------------------------------------
# Unit tests (models/services only — UI correctness is enforced by
# slint_target_sources compiling the markup at build time)
# ---------------------------------------------------------------------------

option(BOILERPLATE_BUILD_TESTS "Build unit tests" ON)
if(BOILERPLATE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
