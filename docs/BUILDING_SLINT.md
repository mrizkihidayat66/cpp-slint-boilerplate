# Building Slint — x64 and x86

This boilerplate builds [Slint](https://slint.dev) from source with CMake
`FetchContent` (see [`CMake/FetchSlint.cmake`](../CMake/FetchSlint.cmake)).
Building from source is Slint's recommended integration for C++ and the only
path that supports **32-bit (x86) Windows targets** — the prebuilt binary
packages ship only for x86-64.

## Prerequisites

| Tool                        | Version                  | Notes                                    |
| --------------------------- | ------------------------ | ---------------------------------------- |
| Visual Studio 2022          | any edition              | C++20-capable MSVC (2019 16.6+ also works) |
| CMake                       | 3.21 or newer            | The VS-bundled copy is fine              |
| Ninja                       | any                      | Recommended generator (`-GNinja`)        |
| Rust toolchain (stable)     | 1.92 or newer            | Required to compile Slint's Rust core    |

Install Rust via [rustup](https://rustup.rs). Verify:

```bash
rustc --version   # must be >= 1.92
cargo --version
```

### Extra step for x86 (32-bit)

Add the 32-bit MSVC Rust target:

```bash
rustup target add i686-pc-windows-msvc
```

### Extra step for Skia builds

The `x64-*-skia` presets additionally need a host C compiler — `clang` or `gcc`
on `PATH` — because rust-skia's build scripts use one while building the Skia
C++ library. The VS-bundled clang (`...\VC\Tools\Llvm\x64\bin`) works. See
[Skia renderer](#skia-renderer).

## Presets

All presets use Ninja + MSVC and share a dependency cache
(`.deps-cache/` at the repo root) so the Slint download and cargo build are
paid **once per architecture**, not once per preset.

| Preset              | Architecture | Build type | Renderer | Extra cache vars                         |
| ------------------- | ------------ | ---------- | -------- | ---------------------------------------- |
| `x64-debug`         | x64          | Debug      | femtovg  | —                                        |
| `x64-release`       | x64          | Release    | femtovg  | —                                        |
| `x86-debug`         | x86          | Debug      | femtovg  | `Rust_CARGO_TARGET=i686-pc-windows-msvc` |
| `x86-release`       | x86          | Release    | femtovg  | `Rust_CARGO_TARGET=i686-pc-windows-msvc` |
| `x64-debug-skia`    | x64          | Debug      | skia     | `SLINT_FEATURE_RENDERER_SKIA=ON`         |
| `x64-release-skia`  | x64          | Release    | skia     | `SLINT_FEATURE_RENDERER_SKIA=ON`         |

The two `*-skia` presets are opt-in builds of the [Skia renderer](#skia-renderer);
the default presets stay on femtovg (see [Backends and renderers](#backends-and-renderers)).
There is no x86 Skia preset: rust-skia publishes prebuilt binaries only for
64-bit Windows, so the 32-bit target does not build (see below).

```bash
cmake --preset x64-debug          # configure
cmake --build --preset x64-debug  # build
./build/x64-debug/slint-boilerplate.exe
ctest --preset x64-debug          # unit tests
```

The **first configure** clones the Slint repository (~200 MB) and the first
**build** compiles its Rust core via cargo — expect 10–20 minutes. Every later
configure/build reuses `.deps-cache/`.

> `cmake --preset` must run inside a **matching MSVC environment**. The
> presets' `architecture.strategy: external` only *declares* the target to
> CMake — it does not switch the toolchain. From a plain shell, call the
> matching vcvars first:
>
> - **x64 presets**: `vcvars64.bat` → `Hostx64\x64\cl.exe`
> - **x86 presets**: `vcvars32.bat` → `Hostx86\x86\cl.exe`
>
> Configuring an x86 preset inside a vcvars64 shell silently compiles with the
> x64 compiler and then fails at link time (the Slint DLL is i686, the app is
> x86-64). Verify with `cl` reporting `x86` after vcvars32, or check
> `CMAKE_CXX_COMPILER` in the build's `CMakeCache.txt`.
>
> The dependency cache is also split per target architecture
> (`.deps-cache/<platform>/`), derived from `CMAKE_VS_PLATFORM_NAME` —
> `CMAKE_SYSTEM_PROCESSOR` always reports the *host* (AMD64) and must not be
> used for this.

## Pinning a Slint version

`CMake/FetchSlint.cmake` pins a concrete release tag (`v1.18.1` by default)
for reproducible builds. To upgrade, change `SLINT_FETCH_TAG` and
`SLINT_VERSION_STRING` together:

```cmake
set(SLINT_FETCH_TAG "v1.18.1")   # a release tag, not the moving release/1 branch
```

then delete `.deps-cache/` (or the slint subdirectory inside it) to force a
re-fetch. To track the moving `release/1` branch instead, pass
`-DSLINT_FETCH_TAG=release/1` at configure time.

## Backends and renderers

Slint compiles a *backend* (windowing) and one or more *renderers*. Defaults
in this boilerplate: **winit backend + femtovg renderer** (OpenGL ES 2.0), with
the software renderer also compiled in as a GPU-less fallback.

Override at configure time:

```bash
# Skia renderer
cmake --preset x64-debug -DSLINT_FEATURE_RENDERER_FEMTOVG=OFF -DSLINT_FEATURE_RENDERER_SKIA=ON

# Software renderer (CPU-only — works on GPU-less machines)
cmake --preset x64-debug -DSLINT_FEATURE_RENDERER_FEMTOVG=OFF -DSLINT_FEATURE_RENDERER_SOFTWARE=ON
```

Select a compiled-in renderer at **runtime** with `SLINT_BACKEND`:

```bash
SLINT_BACKEND=winit-software ./slint-boilerplate.exe   # CPU fallback
SLINT_BACKEND=winit-skia     ./slint-boilerplate.exe   # Skia
```

### Skia renderer

The `x64-*-skia` presets build Slint with the Skia renderer instead of
femtovg. Prefer Skia when you need high-quality GPU-accelerated vector and text
rendering, or a single renderer that behaves identically across Windows, Linux
and macOS.

```bash
cmake --preset x64-release-skia
cmake --build --preset x64-release-skia
SLINT_BACKEND=winit-skia ./build/x64-release-skia/slint-boilerplate.exe
```

**x64 only.** rust-skia ships prebuilt Skia binaries for 64-bit targets only;
on `i686-pc-windows-msvc` the download 404s and the build fails, so there is no
x86 Skia preset.

Trade-offs against the default femtovg build:

| | femtovg (default) | Skia |
| --- | --- | --- |
| First build | 10–20 min | **longer** — rust-skia downloads prebuilt Skia binaries, then the Slint renderer still compiles |
| Build prerequisites | Rust + MSVC | Rust + MSVC **+ a host C compiler** (`clang` or `gcc`) on `PATH`, which rust-skia's build scripts require |
| Dependency cache | `.deps-cache/<arch>/` | reuses it, plus the Skia binary download — budget extra disk |
| Binary size | smaller | larger |
| Runtime | OpenGL ES 2.0 | Skia (GPU), also usable on macOS where femtovg's GL path is limited |

Because a Skia build is heavier and needs an extra host toolchain, it is kept in
opt-in presets rather than the defaults, and CI does **not** build it — the
matrix stays on femtovg.

Widget style for Slint's built-in widgets (not the custom `App*` library,
which is fully themed by `src/ui/theme/`):

```bash
cmake --preset x64-debug -DSLINT_STYLE=material   # fluent | material | cupertino | cosmic | native
```

## Troubleshooting

| Symptom                                   | Fix                                                              |
| ----------------------------------------- | ---------------------------------------------------------------- |
| `link.exe` errors during cargo build      | Wrong toolchain: run from the matching VS developer prompt       |
| `can't find crate for i686`               | `rustup target add i686-pc-windows-msvc`, re-configure            |
| Configure re-downloads Slint every time   | Ensure `SLINT_CACHE_DIR` points at the same path across presets  |
| Window opens but is blank/black           | Try the software renderer: `SLINT_BACKEND=winit-software`        |
| Out of disk                               | Point `-DSLINT_CACHE_DIR=D:/cache/slint` at another drive        |

## VS Code live preview

Install the workspace-recommended **Slint** extension (`slint.slint-vscode`).
It provides syntax checking, formatting, and live preview (`Ctrl+Shift+P` →
"Slint: Show Preview") while you edit `.slint` files — no rebuild needed for
markup-only iteration.
