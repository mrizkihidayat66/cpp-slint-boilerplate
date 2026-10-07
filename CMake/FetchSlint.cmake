# FetchSlint.cmake — acquire Slint (C++ API) via FetchContent.
#
# Building Slint from source is the recommended and most flexible integration
# path, and the only one that supports 32-bit (x86) Windows targets: the
# prebuilt binary packages ship only for x86-64.
#
# Requires a Rust toolchain (1.92+). For x86 builds, additionally run
# `rustup target add i686-pc-windows-msvc` and configure with
# `-DRust_CARGO_TARGET=i686-pc-windows-msvc` (done by the x86 presets).
#
# See docs/BUILDING_SLINT.md for the full story.

include(FetchContent)

# Shared dependency cache across presets/build directories. The Slint checkout
# is ~200 MB and the cargo build is expensive; keeping both in a shared cache
# means presets only pay it once per architecture. Point SLINT_CACHE_DIR at a
# drive-local path on constrained machines.
set(SLINT_CACHE_DIR "${CMAKE_SOURCE_DIR}/.deps-cache" CACHE PATH
    "Shared FetchContent cache directory for Slint and friends")
set(FETCHCONTENT_BASE_DIR "${SLINT_CACHE_DIR}")

# --- Slint build configuration ---------------------------------------------
# Defaults follow Slint's own recommendations; every flag can be overridden on
# the cmake command line.

set(SLINT_STYLE "fluent" CACHE STRING
    "Widget style for Slint's built-in widgets: fluent|material|cupertino|cosmic|native")

# Default backend: winit (windowing). Enable extra backends (e.g. Qt) with
# -DSLINT_FEATURE_BACKEND_QT=ON.
set(SLINT_FEATURE_BACKEND_WINIT ON CACHE BOOL "winit backend (default windowing)")

# Default renderer: femtovg (OpenGL ES 2.0). Switch with:
#   -DSLINT_FEATURE_RENDERER_FEMTOVG=OFF -DSLINT_FEATURE_RENDERER_SKIA=ON   (skia)
#   -DSLINT_FEATURE_RENDERER_FEMTOVG=OFF -DSLINT_FEATURE_RENDERER_SOFTWARE=ON (CPU)
# The software renderer is also selectable at runtime when compiled in:
#   SLINT_BACKEND=winit-software ./slint-boilerplate.exe
set(SLINT_FEATURE_RENDERER_FEMTOVG ON CACHE BOOL "femtovg renderer (default)")
set(SLINT_FEATURE_RENDERER_SKIA OFF CACHE BOOL "skia renderer")
set(SLINT_FEATURE_RENDERER_SOFTWARE OFF CACHE BOOL "software renderer")

# --- Acquire ----------------------------------------------------------------
#
# `release/1` is Slint's stable release branch — it moves forward with patch
# releases. For a fully reproducible build, pin the tag instead:
#   git -C .deps-cache/... checkout v1.18.1
# or set GIT_TAG to a specific tag below before configuring.

set(SLINT_FETCH_TAG "release/1" CACHE STRING
    "Git ref of slint-ui/slint to fetch (release/1 moves; pin e.g. v1.18.1 for reproducibility)")
set(SLINT_VERSION_STRING "1.x (release/1)" CACHE STRING
    "Human-readable Slint version shown on the About page")

FetchContent_Declare(
    slint
    GIT_REPOSITORY https://github.com/slint-ui/slint.git
    GIT_TAG ${SLINT_FETCH_TAG}
    SOURCE_SUBDIR api/cpp
    GIT_SHALLOW FALSE)

# FetchContent_MakeAvailable adds the api/cpp subdirectory directly; it
# defines the Slint::Slint target (and the slint_target_sources command) in
# the current build — no find_package needed.
FetchContent_MakeAvailable(slint)

if(NOT TARGET Slint::Slint)
    message(FATAL_ERROR
        "Slint::Slint target missing after FetchContent — the slint checkout "
        "at ${slint_SOURCE_DIR} did not define its C++ API. Check that "
        "GIT_TAG/SOURCE_SUBDIR in CMake/FetchSlint.cmake still match the "
        "upstream layout.")
endif()
