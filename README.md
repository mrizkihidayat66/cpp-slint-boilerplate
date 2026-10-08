# cpp-slint-boilerplate — examples/advanced

CRUD the professional way: a sortable, paginated task table backed by a
three-layer service chain, with dialog forms that validate per-field and
confirm destructive actions.

This branch is the third stop of the progressive tour (see
[docs/BRANCHES.md](docs/BRANCHES.md)):

| Branch | Demonstrates |
| ------ | ------------ |
| `examples/simple` | Ship a Slint app with ~30 lines of C++ |
| `examples/intermediate` | State + a real service: settings persisted to `%APPDATA%`, live theme |
| **`examples/advanced`** | **Full CRUD: `TaskRepository → TaskValidationService → TaskService`** |

## The demo

Open **Tasks** in the sidebar:

- **Sortable table** — click a column header to sort ascending, click again
  to flip. The reorder happens in C++ (the view never sorts).
- **Pagination** — 10 rows per page over a 22-row seed set; the bar
  recalculates after every mutation.
- **Create / edit** — “New task” or click a row. The dialog validates the
  draft through `TaskValidationService` and shows per-field errors under
  the inputs; duplicate titles are rejected with a summary message.
- **Delete confirmation** — a danger dialog before anything is removed.
- **Status feedback** — every mutation reports back through the status
  banner.

## The chain

```
TaskRepository        owns the data (in-memory, seeded demo set)
        ↓
TaskValidationService stateless field rules (title/assignee/description)
        ↓
TaskService           orchestrates: validate → reject duplicates →
                      mutate → sort → paginate
```

The chain is wired in `Application::registerServices()`: dependencies are
registered first (initialization order), then the orchestrating service is
constructed with them explicitly — no service locator look-ups inside the
chain itself. `TaskService` returns a `TaskMutationResult` carrying either
the stored task or per-field error messages; the UI decides what to do with
them (keep the dialog open, light up the inputs).

## Verification

```bash
cmake --preset x64-debug
cmake --build --preset x64-debug
ctest --preset x64-debug
./build/x64-debug/slint-boilerplate.exe
```

The task chain has its own test suite (`tests/unit/TestTasks.cpp`): seed /
insert / update / remove, validation rules, duplicate rejection, sort
invariants, pagination clamping, and post-delete ordering.

To build **every branch** (main + simple/intermediate/advanced) into one
comparable output tree — `build/bin/<Debug|Release>/<x64|x86>/
slint-boilerplate-<branch>.exe` — run:

```bash
build/harness/build-matrix.cmd
```

See [docs/BRANCHES.md](docs/BRANCHES.md) for the collected-binary layout.

## Prerequisites

- CMake 3.21+ and a C++20 compiler (MSVC 2022, GCC 12+, or Clang 15+)
- Rust 1.92+ (Slint's compiler is built from source via Corrosion)
- First configure downloads dependencies (Slint 1.18.1, Catch2 3.7.1) —
  cached per architecture under `.deps-cache/`

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — layering and data-flow rules
- [docs/COMPONENTS.md](docs/COMPONENTS.md) — the component library API
- [docs/BRANCHES.md](docs/BRANCHES.md) — the progressive example tour

## License

MIT — see [LICENSE](LICENSE).
