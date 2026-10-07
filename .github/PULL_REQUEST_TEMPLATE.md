## Summary

<!-- What does this PR change and why? Link any related issues. -->

## Checklist

- [ ] `clang-format` clean (`clang-format --dry-run --Werror` on changed files)
- [ ] `cmake --build --preset x64-debug` succeeds
- [ ] `ctest --preset x64-debug` passes
- [ ] New/changed components are exported in `components/library.slint`
- [ ] New/changed components are documented in `docs/COMPONENTS.md`
- [ ] No business logic added to `.slint` files (see `docs/ARCHITECTURE.md`)
- [ ] No UI types added to `src/services/` or `src/models/` (beyond `toSlint` adapters)

## Testing

<!-- How was this verified? Attach screenshots for UI changes. -->
