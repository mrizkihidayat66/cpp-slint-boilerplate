# cpp-slint-boilerplate — simple example

> **Branch `examples/simple`** — the minimal starting point.
> See [docs/BRANCHES.md](docs/BRANCHES.md) for the other levels
> (`intermediate`, `advanced`) and the full-featured `main` branch.

This branch answers one question: **how little C++ does a Slint app need?**

```
src/
├── main.cpp          # 17 lines: create the window, run the event loop
└── ui/
    ├── app-window.slint   # window shell + inline demo data
    ├── app-state.slint    # shared structs / globals (markup-only)
    ├── theme/             # design tokens (Palette, Typography, Metrics)
    ├── components/        # the full App* component library
    └── pages/             # Dashboard + About
```

No `Application`, no `ServiceLocator`, no services, no POCOs, no tests.
All demo data is array literals inside `dashboard-page.slint`; navigation
is an `in-out property <int>` bound straight to the sidebar.

When this stops scaling — state you want to persist, logic you want to
test headless, data from outside the process — that is the moment the
`main` scaffold (and the intermediate/advanced branches) become worth it.

## Prerequisites

- Visual Studio 2022 (C++20-capable MSVC)
- CMake 3.21+ with Ninja (both ship with VS)
- Rust stable ≥ 1.92 ([rustup](https://rustup.rs)) — Slint is built from source

CI also builds and tests on Linux and macOS; those legs use the `native-*`
presets and need a host toolchain (`g++` / `clang++`) plus, on Linux, a few
development packages — see
[docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md#linux-and-macos).

## Build & run

From a VS developer prompt:

```bash
cmake --preset x64-debug
cmake --build --preset x64-debug
./build/x64-debug/slint-boilerplate.exe
```

The first configure clones Slint (~200 MB) and the first build compiles its
Rust core — expect 10–20 minutes once; the shared cache in `.deps-cache/`
makes every later build fast.

## Documentation

- [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md) — dependency setup,
  x86/x64 details, troubleshooting
- [docs/COMPONENTS.md](docs/COMPONENTS.md) — component catalog
- [docs/CODING_STYLE.md](docs/CODING_STYLE.md) — markup + C++ conventions

## Roadmap

- [x] Dark palette + runtime theme switching — see `examples/intermediate`
- [x] Skia renderer preset (`x64-*-skia`; x64 only) — see [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md)
- [ ] Richer icon set (bundled vector assets)
- [x] Linux/macOS CI legs (`native-*` presets) — see [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md#linux-and-macos)
- [x] Release artifact workflow — portable ZIP via CPack, published on `v*` tags by [.github/workflows/release.yml](.github/workflows/release.yml)

## License

This repository's scaffolding code is released under the [MIT License](LICENSE).
That covers this boilerplate only — Slint is licensed separately, and you must
choose one of its licenses for any app you ship. See [NOTICE.md](NOTICE.md).
