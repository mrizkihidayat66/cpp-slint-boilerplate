# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Opt-in x64 Skia renderer presets (`x64-debug-skia`, `x64-release-skia`).
- Linux and macOS CI legs (`native-*` presets) that build, test, and smoke-run
  the software renderer headlessly.
- Release workflow: on a `v*` tag, build the x64 release and publish a portable
  ZIP (executable + Slint runtime DLL) to a GitHub Release.
- Richer icon set: 95 built-in vector icons (up from 31), each an inline 24-unit
  stroke path, plus an `AppIcons` catalog global that lists every name for
  pickers and galleries.

### Changed

- The component gallery's "Avatars & icons" card now enumerates the full
  `AppIcons` set in a grid that reflows with the page width instead of showing a
  fixed 16 icons.

## [0.1.0] - 2026-10-07

### Added

- Initial public release.
- Layered C++20 scaffold (`app/` composition root + service locator, `services/`
  behind `IService`, `models/` POCOs with `toSlint()` adapters, `ui/` Slint
  markup).
- Component library of 25 `App*` components with variants, loading/disabled
  states, and design-token theming (`Palette`, `Typography`, `Metrics`).
- x64 and x86 CMake presets; Slint built from source via `FetchContent`, pinned
  to a release tag.
- Progressive example branches (`examples/simple`, `examples/intermediate`,
  `examples/advanced`).
- GitHub Actions CI: formatting check, unit tests, and a GPU-less smoke run.

[Unreleased]: https://github.com/mrizkihidayat66/cpp-slint-boilerplate/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/mrizkihidayat66/cpp-slint-boilerplate/releases/tag/v0.1.0
