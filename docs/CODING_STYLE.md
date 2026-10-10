# Coding style

The goal: a codebase that is **clean, structured, maintainable, consistent, and
professional**. Two languages live here (C++ and Slint markup); each has its own
rules, and a few rules govern the boundary between them.

## C++

### Formatting

[`.clang-format`](../.clang-format) is normative — run
`clang-format -i` on changed files before committing (CI checks this).
Highlights:

- 4-space indentation, no tabs; 100-column limit
- Braces: functions open on a new line, control statements do not
- `PascalCase` types, `camelCase` functions and locals, `m_` member prefix,
  `k` prefix for constants (`kMaxTitleLength`, `kPageSize`)

### File and module organization

- One class per `.h/.cpp` pair; the pair lives in the module directory
  (`src/app/`, `src/services/`, `src/models/`)
- Headers use `#pragma once`
- Include order (enforced by convention, sorted alphabetically within blocks):
  1. The header's own twin (only in `.cpp` files)
  2. Project headers (`"app/…"`, `"services/…"`, `"models/…"`)
  3. Generated Slint headers (`<app-window.h>`)
  4. Slint headers (`<slint/…>`)
  5. Third-party, then standard library

### Naming

| Element            | Convention            | Example                     |
| ------------------ | --------------------- | --------------------------- |
| Namespace          | project-scoped snake  | `cpp_slint_boilerplate`     |
| Class / struct     | `PascalCase`          | `DashboardService`          |
| Function / method  | `camelCase`           | `statCards()`               |
| Member variable    | `m_` + `camelCase`    | `m_services`                |
| Free function      | `camelCase`           | `toSlint()`                 |
| Constant           | `k` + `PascalCase`    | `kMaxTitleLength`           |
| File name          | class name            | `DashboardService.cpp`      |

No type-encoding prefixes in file names. The single tolerated interface
prefix is `IService` (there is exactly one interface).

### Error handling

- Constructors and wiring (composition root) may **throw** — failures there
  are programming errors
- Lookups and parsing return `std::optional` (or throw only at boundaries)
- Services never crash the UI thread: catch, log, degrade

### Comments

Comment the *why*, not the what. Every file starts with a one-line purpose
comment. Doc-comments (`///`) on public APIs in headers.

## Slint

### Formatting

- 4-space indentation; 100-column limit (`.editorconfig`)
- `slint-lsp` formats on save via the VS Code extension

### Naming

| Element            | Convention            | Example                     |
| ------------------ | --------------------- | --------------------------- |
| File               | `kebab-case.slint`    | `app-button.slint`          |
| Component          | `App` + `PascalCase`  | `AppButton`                 |
| Struct / global    | `PascalCase`          | `StatCard`, `Palette`       |
| Property/callback  | `kebab-case`          | `icon-name`, `clicked`      |
| Enum               | `App*` + `PascalCase` | `AppButtonVariant`          |
| Enum value         | `kebab-case`          | `AppButtonVariant.primary`  |

The `App` prefix prevents collisions with Slint's own std-widgets.

### Component contract

Every component in [`components/`](../src/ui/components/) follows:

1. **Variants and sizes are enums**, never bool matrices
   (`variant: AppButtonVariant.danger`, not `danger: true`)
2. **States** are `disabled` and `loading` boolean properties
3. **Events** use DOM-conventional callback names (`clicked`, `accepted`)
4. **Every property has a default** — a component works with zero props
5. Components read colors, fonts, and sizes from the theme globals only —
   **no hardcoded values**

### Structure rules

- Pages import from `components/library.slint` only, never from individual
  component files
- Shared structs and app state live in `app-state.slint`, not in the window
- One component per file, named after the component

## The boundary rules

These two rules keep the architecture testable and are enforced in review:

1. **No business logic in `.slint` files.** Markup lays out and renders; it
   does not compute, validate, or decide. The moment logic appears, it moves
   to a service and the data arrives through a model or property.
2. **No UI types in `services/` or `models/`.** Services return pure C++
   (POCOs, `std::string`, `std::vector`); only the `toSlint()` adapters touch
   generated types. This keeps the entire logic layer unit-testable without
   a windowing backend.

## Git conventions

- **Conventional commits**: `feat:`, `fix:`, `docs:`, `refactor:`,
  `test:`, `chore:`, `build:`
- Branch names: `feat/<topic>`, `fix/<topic>`, `docs/<topic>`
- Keep commits atomic — one concern per commit
