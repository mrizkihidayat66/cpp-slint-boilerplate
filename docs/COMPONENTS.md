# Component library

A Bootstrap-like component set for Slint, in
[`src/ui/components/`](../src/ui/components/). Import everything from the
index — never from individual files:

```slint
import { AppButton, AppCard, AppInput } from "../components/library.slint";
```

## API contract

Every component follows the same rules (see also
[CODING_STYLE.md](CODING_STYLE.md)):

- **Variants and sizes are enums** — `variant: AppButtonVariant.danger`,
  `size: AppButtonSize.lg` — never bool matrices
- **States**: `disabled` (muted, not interactive) and `loading` (spinner,
  swallows clicks) wherever they make sense
- **Events**: DOM-conventional callback names — `clicked`, `accepted`,
  `edited`, `toggled`, `selected`
- **Zero-config works**: every property has a sensible default
- **Theming**: components read `Palette` / `Typography` / `Metrics` globals
  only — restyling the app means editing `src/ui/theme/`, nothing else

## Theme tokens

### Palette (`theme/colors.slint`)

| Token group  | Members                                                        |
| ------------ | -------------------------------------------------------------- |
| Brand        | `primary`, `primary-hover`, `primary-active`, `primary-disabled` |
| Feedback     | `success(-surface)`, `warning(-surface)`, `danger(-hover, -surface)`, `info(-surface)` |
| Neutrals     | `background`, `surface`, `surface-alt`, `border`, `border-strong` |
| Text         | `text`, `text-muted`, `text-disabled`, `text-on-primary`       |
| Effects      | `shadow`, `overlay`                                            |

### Typography (`theme/typography.slint`)

`font-family` (Segoe UI Variable stack), `font-size-xs … 2xl`
(11→28 px), `font-weight-regular/medium/bold`, `line-height-tight/normal`.

### Metrics (`theme/metrics.slint`)

`space-1…6` (4→32 px), `radius-sm/md/lg/pill`, `control-height-sm/md/lg`
(28/36/44 px), layout constants (`sidebar-width`, `navbar-height`,
`window-min-*`), `elevation-1/2/3`.

## Catalog

### Actions

#### AppButton

| Property   | Type               | Default                      | Description                        |
| ---------- | ------------------ | ---------------------------- | ---------------------------------- |
| `text`     | `string`           | `""`                         | Label (empty → icon-only)          |
| `variant`  | `AppButtonVariant` | `primary`                    | `primary` `secondary` `outline` `ghost` `danger` |
| `size`     | `AppButtonSize`    | `md`                         | `sm` `md` `lg`                     |
| `icon`     | `image`            | —                            | Leading icon                       |
| `loading`  | `bool`             | `false`                      | Spinner + swallow clicks           |
| `disabled` | `bool`             | `false`                      | Muted, non-interactive             |
| `block`    | `bool`             | `false`                      | Full width                         |

Callback: `clicked()`.

#### AppSpinner

`size: length` (16px), `color: color` (Palette.primary). Indeterminate arc.

### Forms

#### AppInput

| Property       | Type     | Default  | Description                         |
| -------------- | -------- | -------- | ----------------------------------- |
| `text`         | in-out   | `""`     | Value                               |
| `label`        | `string` | `""`     | Leading label (hidden when empty)   |
| `placeholder`  | `string` | `""`     |                                     |
| `icon-name`    | `string` | `""`     | Leading AppIcon name                |
| `error-text`   | `string` | `""`     | Error styling + message             |
| `helper-text`  | `string` | `""`     | Muted hint (hidden when error)      |
| `clearable`    | `bool`   | `true`   | Clear button                        |
| `password`     | `bool`   | `false`  | Masked input                        |
| `disabled`, `read-only` | `bool` | `false` |                             |

Callbacks: `accepted()`, `edited()`, `cleared()`.

#### AppTextarea

`text` (in-out), `label`, `placeholder`, `maxlength` (0 = no counter),
`error-text`, `disabled`, `read-only`. Callbacks `edited()`, `accepted()`.

#### AppCheckbox

`checked` (in-out), `indeterminate`, `label`, `disabled`.
Callback `toggled()`.

#### AppRadio

`value`, `group-value` (in-out, shared by all radios of the group),
`label`, `disabled`. Callback `selected(value)`.
**Grouping**: bind every option's `group-value` to the same parent property —

```slint
property <string> color;
HorizontalLayout {
    AppRadio { value: "red";  group-value <=> root.color; label: "Red"; }
    AppRadio { value: "blue"; group-value <=> root.color; label: "Blue"; }
}
```

#### AppSwitch

`checked` (in-out), `label`, `label-first`, `disabled`.
Callback `toggled()`.

#### AppSelect

`model: [string]`, `selected-index` (in-out, -1 = placeholder),
`placeholder`, `disabled`. Callback `selected(index, value)`.

#### AppSlider

`value` (in-out), `minimum`, `maximum`, `step`, `snap`, `disabled`.
Callback `changed(value)`.

### Feedback

#### AppBadge

`text`, `variant` (`primary` `success` `warning` `danger` `info` `neutral`),
`pill`, `dot`.

#### AppAlert

`text`, `title`, `variant` (`info` `success` `warning` `danger`),
`dismissible`. Callback `dismissed()`.

#### AppProgress

`value` (0–1; **negative = indeterminate**), `thickness`, `show-percentage`.

#### AppTooltip

Wraps any content (`@children`); shows `text` on hover. `placement`
(`top`/`bottom`/`left`/`right`), `delay`.

### Containers

#### AppCard

`title`, `subtitle`, `header-icon`, `elevation` (`flat`/`raised`/`floating`),
`padding`, `bordered`. Body via `@children`.

#### AppDialog

`open` (in-out), `title`, `dismiss-on-overlay`. Body via `@children`.
Callbacks `accepted()`, `rejected()`.
Instantiate at the window root so the overlay covers everything. Slint allows
one `@children` per component, so host the *footer* (buttons) as part of the
body and call `accepted()`/`rejected()` from the buttons themselves.

### Data display

#### AppTable

`columns: [AppTableColumn{title, weight}]`, `rows: [AppTableRow{cells}]`,
`sortable`, `sort-column`, `sort-ascending`, `striped`, `bordered`,
`row-height`. Callbacks `row-activated(index, row)`,
`sort-requested(column, ascending)` — the **owner** reorders the model
(no sorting logic in the view). Built on `ListView` → virtualized.

#### AppTabs

`model: [string]`, `current-index` (in-out), `variant`
(`underline`/`pills`), `disabled`. Callback `current-index-changed(int)`.

#### AppAvatar

`name` (first character becomes the fallback glyph), `initials-override`,
`second-initial`, `source: image` (overrides initials), `size`
(`sm`/`md`/`lg`), `show-status`, `status-color`.

#### AppAccordion

`items: [AppAccordionItem{title, icon-name}]`, `expanded-index` (in-out,
-1 = all collapsed), `multiple`, `open-flags: [bool]` (for `multiple`).
Callbacks `expanded(int)`, `collapsed(int)`. When `multiple` is set, the
owner mutates `open-flags` from the callbacks (models are C++-owned).

### Navigation

#### AppSidebar

`items: [AppSidebarItem{icon-name, label}]`, `current-index` (in-out),
`collapsed` (icon-only rail). Callback `navigate(int)`.

#### AppNavbar

`title`, `subtitle`, `icon-name`; leading slot via `@children`.

#### AppBreadcrumb

`model: [string]`, `current-index` (-1 = last is current).
Callback `navigate(int)`.

#### AppMenu

`items: [AppMenuItem{icon-name, label, is-separator, disabled}]`, `open`
(in-out), `menu-width`. Callback `selected(index, item)`. Trigger content via
`@children` (e.g. an AppButton with `clicked => { menu.open = true; }`).

#### AppPagination

`page` (1-based, in-out), `page-count`, `window-size`, `disabled`.
Callback `page-changed(int)`.

#### AppIcon

`name` (see the `AppIcons` global for the built-in set), `size`, `color`.
Unknown names render the warning glyph — extend `AppIcons` in
`app-icon.slint` when you need more.

## Responsive patterns

Slint has no CSS media queries; the idiomatic equivalents used here:

1. **Width-threshold conditionals** — e.g. the app shell:
   `collapsed: root.width < 600px` on the sidebar
2. **`block` properties** — stretch controls full-width inside narrow
   containers
3. **`horizontal-stretch` / `vertical-stretch`** — let layouts redistribute
   space instead of hardcoding sizes
4. **Token-derived sizes** — preferred sizes come from `Metrics`, so a theme
   change reflows everything

## Adding a component

1. Create `src/ui/components/app-<name>.slint` following the API contract
2. Export it from `components/library.slint`
3. Add a catalog entry (this file)
4. (Example branches) add it to the component-gallery page
