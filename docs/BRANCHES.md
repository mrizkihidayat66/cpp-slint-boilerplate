# Branches — progressive examples

The boilerplate teaches in layers. `main` carries the full scaffold and the
component library; **three example branches** extend it from beginner-simple
to advanced-but-compact. They are parallel demonstration timelines, not a
stack — check out the one that matches your learning goal.

```
main ──┬── examples/simple        (minimal: UI only)
       ├── examples/intermediate  (state + services + persistence)
       └── examples/advanced      (full CRUD + service chains)
```

| Branch                   | Demonstrates                                        |
| ------------------------ | --------------------------------------------------- |
| `main`                  | Clean scaffold, full 25-component library, layered architecture, CI |
| `examples/simple`       | You can ship a Slint app with ~30 lines of C++. No services, no DI — static data inline in `.slint` |
| `examples/intermediate`  | State management + a real service: settings page persisted to `%APPDATA%` JSON, theme toggle, component gallery page showing every control live |
| `examples/advanced`     | CRUD the professional way: `TaskRepository → TaskValidationService → TaskService` chain via ServiceLocator, sortable + paginated `AppTable`, dialog forms with validation errors, delete confirmations, loading states |

## Using a branch

```bash
git switch examples/intermediate
cmake --preset x64-debug
cmake --build --preset x64-debug
./build/x64-debug/slint-boilerplate.exe
```

Each branch builds on its own — same presets, same docs (the branch's README
describes its specific demo).

## Comparing all branches side by side

`scripts/build-matrix.cmd` builds every branch × every preset and
collects the finished binaries into one place for direct comparison:

```
build/bin/Debug/x64/slint-boilerplate-main.exe
build/bin/Debug/x64/slint-boilerplate-simple.exe
build/bin/Debug/x64/slint-boilerplate-intermediate.exe
build/bin/Debug/x64/slint-boilerplate-advanced.exe      + slint_cpp.dll
build/bin/Release/x64/...
build/bin/Debug/x86/...
build/bin/Release/x86/...
```

Intermediate worktrees/build trees live under `build/build-win/<branch>/`
and can be deleted once you are done comparing; the shared Slint cache
(`.deps-cache/`) is reused from the main checkout, so a full matrix run
only compiles each architecture's Rust runtime once.

## Maintenance policy

- The branches are **forks of `main`**, cut after `main` is tagged
- Shared fixes (build breakage, Slint API changes) are **cherry-picked**
  from `main` into each branch and re-tagged
- Nothing merges back into `main` by default — `main` stays the clean
  reference scaffold
- New components land in `main`'s library and are cherry-picked when a branch
  needs them
