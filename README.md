# cpp-slint-boilerplate

A professional C++20 + [Slint](https://slint.dev) boilerplate for desktop
applications on Windows — x64 **and** x86 (32-bit).

> Status: scaffolding in progress. This README will grow into the full hero
> document (quickstart, feature matrix, branch map) as the buildable core lands.

## What this is

- A layered C++ application scaffold: `app/` (composition root + DI),
  `services/` (business logic), `models/` (pure data), `ui/` (Slint markup).
- A Bootstrap-like **Slint component library** (buttons, inputs, cards,
  dialogs, tables, navigation, ...) themed through design-token globals
  (`Palette`, `Typography`, `Metrics`).
- Build system based on CMake presets for x64/x86 × Debug/Release, with Slint
  built from source via `FetchContent` — the only integration path that also
  supports 32-bit targets.
- Progressive examples on separate branches, from a minimal app to full CRUD
  with a service chain. See [docs/BRANCHES.md](docs/BRANCHES.md).

## Prerequisites

- Visual Studio 2022 (MSVC, C++20)
- CMake 3.21+ with Ninja
- Rust toolchain (stable) — required to build Slint from source

32-bit builds additionally need the i686 Rust target:

```bash
rustup target add i686-pc-windows-msvc
```

## Quickstart

```bash
cmake --preset x64-debug
cmake --build --preset x64-debug
./build/x64-debug/slint-boilerplate.exe
```

The first configure downloads and builds Slint — expect 10–20 minutes on the
initial run; subsequent configurations reuse the cache. See
[docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md) for details, x86 presets, and
renderer/backend options.

## Documentation

| Document                                          | Contents                                        |
| ------------------------------------------------- | ----------------------------------------------- |
| [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md) | Building Slint from source, x64 + x86, presets |
| [docs/CODING_STYLE.md](docs/CODING_STYLE.md)     | C++ and Slint style rules                       |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)     | Layering, state flow, model adapters            |
| [docs/COMPONENTS.md](docs/COMPONENTS.md)         | Component library catalog                       |
| [docs/CONTRIBUTING.md](docs/CONTRIBUTING.md)     | How to contribute                               |
| [docs/BRANCHES.md](docs/BRANCHES.md)             | Branch-per-level example scheme                 |

## License

No license is applied to this repository — see [NOTICE.md](NOTICE.md) for the
Slint licensing options you must choose between when shipping an app.
