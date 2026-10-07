# cpp-slint-boilerplate

A professional C++20 + [Slint](https://slint.dev) boilerplate for desktop
applications on Windows — x64 **and** x86 (32-bit).

## What you get

- **A layered C++ scaffold** — `app/` (composition root + service locator DI),
  `services/` (business logic behind `IService`), `models/` (pure-C++ POCOs
  with `toSlint()` adapters), `ui/` (Slint markup). See
  [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
- **A Bootstrap-like component library** — 25 `App*` components (buttons,
  inputs, selects, tables, dialogs, navigation, …) with enum variants,
  loading/disabled states, and full theming through design-token globals
  (`Palette`, `Typography`, `Metrics`). See
  [docs/COMPONENTS.md](docs/COMPONENTS.md).
- **x64 + x86 builds from one setup** — CMake presets for both architectures;
  Slint is built from source via `FetchContent`, the only integration that
  supports 32-bit targets. See [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md).
- **Progressive examples** — from a ~30-line app to full CRUD with service
  chains, on separate branches. See [docs/BRANCHES.md](docs/BRANCHES.md).
- **CI** — Windows matrix (x64 + x86 × Debug/Release), formatting check,
  unit tests, GPU-less smoke run.
- **Docs** — coding style, contribution guide, architecture deep-dive,
  component catalog, build instructions.

## Prerequisites

- Visual Studio 2022 (C++20-capable MSVC)
- CMake 3.21+ with Ninja (both ship with VS)
- Rust stable ≥ 1.92 ([rustup](https://rustup.rs)) — Slint is built from source

For 32-bit builds:

```bash
rustup target add i686-pc-windows-msvc
```

## Quickstart

From a VS developer prompt:

```bash
cmake --preset x64-debug
cmake --build --preset x64-debug
./build/x64-debug/slint-boilerplate.exe
```

The first configure clones Slint (~200 MB) and the first build compiles its
Rust core — expect 10–20 minutes once; the shared cache in `.deps-cache/`
makes every later build fast.

| Preset         | Purpose                    |
| -------------- | -------------------------- |
| `x64-debug`    | Day-to-day development     |
| `x64-release`  | Optimized builds           |
| `x86-debug`    | 32-bit development         |
| `x86-release`  | 32-bit optimized builds    |

## Documentation

| Document                                          | Contents                                        |
| ------------------------------------------------- | ----------------------------------------------- |
| [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md) | Building Slint from source, x64 + x86, renderers/backends |
| [docs/CODING_STYLE.md](docs/CODING_STYLE.md)     | C++ and Slint style rules                       |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)     | Layering, state flow, model adapters            |
| [docs/COMPONENTS.md](docs/COMPONENTS.md)         | Component library catalog + theming             |
| [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md)     | Setup, workflow, PR checklist                   |
| [docs/BRANCHES.md](docs/BRANCHES.md)             | Branch-per-level example scheme                 |

## Roadmap

- [ ] Dark palette + runtime theme switching (examples/intermediate)
- [ ] Skia renderer preset
- [ ] Richer icon set (bundled vector assets)
- [ ] Linux/macOS CI legs
- [ ] Release artifact workflow (packaged zip/installer)

## License

No license is applied to this repository — see [NOTICE.md](NOTICE.md) for the
Slint licensing options you must choose between when shipping an app.
