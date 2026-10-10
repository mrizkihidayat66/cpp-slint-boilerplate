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
- **Progressive examples** — from a ~17-line app to full CRUD with service
  chains, on separate branches. See [docs/BRANCHES.md](docs/BRANCHES.md).
- **CI** — Windows matrix (x64 + x86 × Debug/Release) plus Linux and macOS
  legs, formatting check, unit tests, GPU-less smoke run.
- **Releases** — push a `v*` tag to publish a portable ZIP (executable + Slint
  runtime DLL) to a GitHub Release. See [CHANGELOG.md](CHANGELOG.md).
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

CI also builds and tests on Linux and macOS; those legs use the `native-*`
presets and need a host toolchain (`g++` / `clang++`) plus, on Linux, a few
development packages — see
[docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md#linux-and-macos).

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

- [x] Dark palette + runtime theme switching — see `examples/intermediate`
- [x] Skia renderer preset (`x64-*-skia`; x64 only) — see [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md)
- [ ] Richer icon set (bundled vector assets)
- [x] Linux/macOS CI legs (`native-*` presets) — see [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md#linux-and-macos)
- [x] Release artifact workflow — portable ZIP via CPack, published on `v*` tags by [.github/workflows/release.yml](.github/workflows/release.yml)

## License

This repository's scaffolding code is released under the [MIT License](LICENSE).
That covers this boilerplate only — Slint is licensed separately, and you must
choose one of its licenses for any app you ship. See [NOTICE.md](NOTICE.md).
