# scripts/ — development & verification harness

Windows-only tooling used to build and verify this repository. None of it is
needed to build or run the app itself — the CMake presets in the repo root are
the supported path (see [../docs/BUILDING_SLINT.md](../docs/BUILDING_SLINT.md)).
All output lands under `build/`, which is git-ignored.

## Build

| Script | Purpose |
| ------ | ------- |
| `build-matrix.cmd` | Build every branch × every preset (x64/x86 × Debug/Release) and collect the binaries into `build\bin\<Config>\<arch>\`. Reuses git worktrees under `build\build-win\<arch>\<branch>\` and the shared `.deps-cache\`. |

## Capture & visual verification

| Script | Purpose |
| ------ | ------- |
| `capture-all.ps1` | Full screenshot suite — every page at several window sizes, plus scrolled gallery sections — into `build/shots\`. Each capture is a fresh process so state never leaks. |
| `capture-ui.ps1` | Precision capture of the *client area* only (DPI-aware, via `PrintWindow`, GPU-rendered content included). Sizes the window to an exact client rect and crops the title bar/borders. |
| `live-preview.py` | Boots `slint-lsp` over stdio, live-previews any component from any `.slint` file, and re-captures after an edit with **no rebuild**. |
| `shot-pages.py` | Screenshots the preview-harness components (below) through the MCP live preview. Requires Pillow. |
| `verify-align.py` | Numerical alignment checks — dumps element geometry and asserts icon-vs-text vertical centering is within tolerance. Non-zero exit on failure. |

## Interaction & MCP

| Script | Purpose |
| ------ | ------- |
| `functional-test.py` | Clicks real controls via the Slint MCP preview and verifies observable state changes (checkbox, radio, switch, select, …). |
| `mcp.py` | Reusable helpers for the Slint LSP MCP server (`call`, element-tree walking, click/scroll/set-value). |
| `mcp-query.py` | Ad-hoc element-tree query CLI: `python scripts/mcp-query.py [substring]`. |
| `mcp-check.py` | Sanity-checks the MCP element tree of every tracked window. |
| `preview/kill-lp.ps1` | Kills stray `slint-lsp` instances and frees the live-preview control port. |

## Preview harnesses

`preview/*.slint` are small standalone components that isolate a widget for
capture and interaction tests:

- `form-preview.slint` → `FormPreview`
- `select-preview.slint` → `SelectPreview`
- `tasks-preview.slint` → `TasksPreview`

## Requirements

- Windows with PowerShell
- Python 3 (Pillow for `shot-pages.py`)
- `slint-lsp` on `PATH` for the `live-preview` / MCP-based tools
