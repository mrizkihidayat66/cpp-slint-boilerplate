# Notice

This boilerplate repository does not carry an explicit license for its own
scaffolding code. You are free to use it as a starting point for your own
projects; add the license of your choice when you fork it.

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
<https://github.com/slint-ui/slint> (the `release/1` branch by default). Its
license and notices apply to that fetched copy, not to anything in this
repository.
