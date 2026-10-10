# cpp-slint-boilerplate — intermediate example

> **Branch `examples/intermediate`** — state management + a real service.
> See [docs/BRANCHES.md](docs/BRANCHES.md) for the other levels.

This branch adds the two things the `simple` example defers:

1. **State that outlives the process** — every settings edit is persisted
   by `SettingsService` to `%APPDATA%\cpp-slint-boilerplate\settings.json`
   (hand-rolled JSON, atomic write via `.tmp` + rename, no dependency).
2. **A live theme system** — the settings page's dark-theme switch flips
   `Palette.dark`, and every token in `theme/colors.slint` reacts
   immediately. No restart, no per-component branches.

```
SettingsService (load/save, %APPDATA% JSON)
      │  AppSettings POCO
      ▼
Application::pushSettings ──► AppState (global: theme, name, flags…)
      │                              │
      │                    Palette.dark ⇐ AppState.theme
      ▼                              ▼
   settings-page.slint      every component re-colors live
```

The pages keep their side of the contract: drafts are local properties,
Save commits them to `AppState`, and `request-save-settings` is the only
bridge to C++. The service normalizes and re-pushes — the UI never trusts
its own unvalidated state.

## What's on the settings page

| Control            | Setting          | Persistence                |
| ------------------ | ---------------- | -------------------------- |
| `AppInput`         | Display name     | `displayName` (string)     |
| `AppSwitch`        | Dark theme       | `theme` ("light"/"dark")   |
| `AppSwitch`        | Notifications    | `notifications` (bool)     |
| `AppSwitch`        | Launch at start  | `autoStart` (bool)         |
| `AppSelect`        | Language         | `language` ("en"/"id")     |

Save/Reset buttons, a success toast, and a theme badge showing the
current mode round it off. The component gallery page shows every other
control live as well.

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

Unit tests cover the JSON round-trip, the reject-invalid-document rule,
and reload-after-restart:

```bash
ctest --preset x64-debug
```

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — layering rules this branch follows
- [docs/COMPONENTS.md](docs/COMPONENTS.md) — component catalog
- [docs/BUILDING_SLINT.md](docs/BUILDING_SLINT.md) — dependency setup, x86/x64

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
