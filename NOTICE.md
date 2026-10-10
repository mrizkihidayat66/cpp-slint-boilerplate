# Notice

This boilerplate's own scaffolding code is released under the MIT License — see
[LICENSE](LICENSE). You are free to use it as a starting point for your own
projects.

That license covers this repository's code only. It does **not** cover Slint,
which you must license separately for any application you ship — see below.

## Slint licensing

This project depends on [Slint](https://slint.dev), which is available under a
choice of three licenses. **You must choose one for any application you build
on this boilerplate:**

| License                        | Use case                                                     |
| ------------------------------ | ------------------------------------------------------------ |
| **Royalty-free License**       | Free for proprietary **desktop** (and mobile/web) applications |
| **GNU GPLv3**                  | Free for open-source applications (embedded, desktop, mobile, web) |
| **Commercial License**         | Proprietary embedded applications, or commercial support      |

Review the full terms at <https://slint.dev/license> before distributing an
application built with Slint.

## Slint source

Slint is fetched at configure time via CMake `FetchContent` from
<https://github.com/slint-ui/slint>, pinned to the `v1.18.1` release tag by
default (`SLINT_FETCH_TAG` in `CMake/FetchSlint.cmake`). Its license and notices
apply to that fetched copy, not to anything in this repository.
