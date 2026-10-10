cmake_minimum_required(VERSION 3.21)

project(
    cpp_slint_boilerplate
    VERSION 0.1.0
    DESCRIPTION "C++20 + Slint desktop application — simple example"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Application identity — the single place to rename the app.
#
# APP_TECHNICAL_NAME is the machine-facing name (window title, About card);
# APP_DISPLAY_NAME is the short brand in the UI header. Both reach C++ as
# compile definitions and, through the AppInfo global, the .slint surfaces.
# The version comes from PROJECT_VERSION above — do not hardcode it again.
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

# AppMetadata.h bakes these values in as macros, so the target that includes it
# must see the same defines.
target_compile_definitions(slint-boilerplate PRIVATE
    APP_TECHNICAL_NAME="${APP_TECHNICAL_NAME}"
    APP_DISPLAY_NAME="${APP_DISPLAY_NAME}"
    APP_VERSION="${PROJECT_VERSION}")
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

# ---------------------------------------------------------------------------
# Packaging — a portable ZIP, produced by the release workflow
# (.github/workflows/release.yml) and usable locally:
#
#   cmake --build --preset x64-release --target package
#   -> build/x64-release/cpp-slint-boilerplate-<version>-windows-<arch>.zip
#
# The ZIP holds the executable and Slint's C++ runtime DLL side by side, so it
# unpacks and runs. It does not bundle the MSVC runtime (the build uses the
# default dynamic CRT) — that is a Visual C++ Redistributable dependency.
# ---------------------------------------------------------------------------

install(TARGETS slint-boilerplate RUNTIME DESTINATION .)

# Slint's C++ API is a shared library; ship it next to the exe (mirrors the
# POST_BUILD copy above, which only covers the build tree).
install(FILES "$<TARGET_FILE:slint_cpp-shared>" DESTINATION .)

# CMAKE_VS_PLATFORM_NAME is only set by the VS generators; the Ninja presets
# carry no generator platform, so fall back to the pointer size.
if(CMAKE_VS_PLATFORM_NAME)
    set(_pkg_arch "${CMAKE_VS_PLATFORM_NAME}")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_pkg_arch "x64")
else()
    set(_pkg_arch "x86")
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "cpp-slint-boilerplate")
set(CPACK_PACKAGE_VENDOR "cpp-slint-boilerplate")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_FILE_NAME
    "cpp-slint-boilerplate-${PROJECT_VERSION}-windows-${_pkg_arch}")
# Extract into a named folder rather than scattering files at the archive root.
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
include(CPack)
