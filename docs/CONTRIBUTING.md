# Contributing

Thanks for improving the boilerplate! This document covers prerequisites,
the local workflow, and what a PR must satisfy.

## Prerequisites

- Visual Studio 2022 (any edition) with the C++ workload
- CMake 3.21+ and Ninja (both ship with VS)
- Rust stable ≥ 1.92 via [rustup](https://rustup.rs)
  - For x86 work: `rustup target add i686-pc-windows-msvc`

Full setup details: [BUILDING_SLINT.md](BUILDING_SLINT.md).

## Setup

```bash
git clone <your-fork-url>
cd cpp-slint-boilerplate
cmake --preset x64-debug
cmake --build --preset x64-debug
ctest --preset x64-debug
```

The first build compiles Slint from source (10–20 min); the shared cache in
`.deps-cache/` makes later builds fast.

VS Code: open the folder and install the recommended extensions (Slint LSP,
CMake Tools, clangd) — tasks and launch configs are provided in `.vscode/`.

## Workflow

1. Branch from `main`: `feat/<topic>`, `fix/<topic>`, `docs/<topic>`
2. Make the change — follow [CODING_STYLE.md](CODING_STYLE.md)
3. Run the checks (below)
4. Conventional-commit messages: `feat:`, `fix:`, `docs:`, `refactor:`,
   `test:`, `chore:`, `build:`
5. Open a PR against `main` with the template filled in

## Before every PR

```bash
# 1. Build
cmake --build --preset x64-debug

# 2. Tests
ctest --preset x64-debug
```

Formatting is checked in CI with `clang-format` (see the `format` job in
`.github/workflows/ci.yml` for the exact invocation); run it on your changed
C++ files before pushing. All checks must pass. CI runs the same matrix
(format, x64/x86 × Debug/Release, tests, smoke run).

## Adding a component

Checklist (details in [COMPONENTS.md](COMPONENTS.md)):

- [ ] New file `src/ui/components/app-<name>.slint` following the API
      contract (enum variants, state props, DOM-convention callbacks, defaults
      for everything, theme tokens only)
- [ ] Exported in `components/library.slint`
- [ ] Catalog entry in `docs/COMPONENTS.md`
- [ ] No hardcoded colors/fonts/sizes — tokens from `Palette` /
      `Typography` / `Metrics` only

## Adding a service

Checklist (details in [ARCHITECTURE.md](ARCHITECTURE.md)):

- [ ] Implements `IService` (`initialize()`/`shutdown()`)
- [ ] Pure-C++ interface — no Slint types (adapters belong in `src/models/`)
- [ ] Registered in `Application::registerServices()`
- [ ] Catch2 tests in `tests/unit/`
- [ ] New structs mirrored: `app-state.slint` struct + POCO + `toSlint()`

## Review criteria

Maintainers look for:

- **Boundary rules held** — no business logic in `.slint`, no UI types in
  services/models
- **API contract** — new components follow the enum/state/callback pattern
- **Tests** — logic changes ship with coverage
- **Docs** — new surface area is documented (COMPONENTS/ARCHITECTURE)

## Code of conduct

Be professional and kind. Critique code, not people. Zero tolerance for
harassment.
