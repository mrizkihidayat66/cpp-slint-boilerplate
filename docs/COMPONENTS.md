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
| Brand        | `primary`, `primary-hover`, `primary-active`, `primary-disabled`, `primary-tint` |
| Feedback     | `success(-hover, -surface)`, `warning(-surface)`, `danger(-hover, -surface)`, `info(-surface)` |
| Neutrals     | `background`, `surface`, `surface-alt`, `surface-sunken`, `border`, `border-strong`, `hairline` |
| Text         | `text`, `text-muted`, `text-faint`, `text-disabled`, `text-on-primary` |
| Sidebar      | `sidebar`, `sidebar-hover`, `sidebar-active`, `sidebar-text(-active)`, `sidebar-brand`, `sidebar-border` |
| Effects      | `shadow`, `shadow-strong`, `overlay`                           |

### Typography (`theme/typography.slint`)

`font-family` (Segoe UI Variable stack), `font-family-mono`, `font-size-xs … 2xl`
(11→26 px), `font-weight-regular/medium/semibold/bold`, `line-height-tight/normal`.

### Metrics (`theme/metrics.slint`)

`space-1…6` (4→32 px), `radius-sm/md/lg/pill`, `control-height-sm/md/lg`
(28/34/42 px), layout constants (`sidebar-width`, `sidebar-width-collapsed`,
`navbar-height`, `page-max-width`, `window-min-*`), `elevation-1/2/3`.

## Catalog

### Actions

#### AppButton

| Property       | Type                 | Default | Description                              |
| -------------- | -------------------- | ------- | ---------------------------------------- |
| `text`         | `string`             | `""`    | Label (empty + `square` → icon button)   |
| `variant`      | `AppButtonVariant`   | `primary` | `primary` `secondary` `outline` `ghost` `danger` `success` |
| `size`         | `AppButtonSize`      | `md`    | `sm` `md` `lg`                           |
| `icon-name`    | `string`             | `""`    | Leading icon (AppIcon name)              |
| `trailing-icon-name` | `string`       | `""`    | Trailing icon (chevrons, next actions)   |
| `text-align`   | `AppButtonTextAlign` | `center`| `left` `center` `right` (block buttons)  |
| `square`       | `bool`               | `false` | Square icon-only button (width = height) |
| `loading`      | `bool`               | `false` | Spinner + swallow clicks                 |
| `disabled`     | `bool`               | `false` | Muted, non-interactive                   |
| `block`        | `bool`               | `false` | Full width                               |

Callback: `clicked()`.

#### AppSpinner

`size: length` (16px), `accent-color: color` (Palette.primary). Indeterminate arc.

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
`error-text`, `disabled`, `read-only`. Callback `edited()`.

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

`title`, `subtitle`, `header-icon-name`, `elevation` (`flat`/`raised`/`floating`),
`body-padding`, `bordered`. Body via `@children`.

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

`initials` (pre-built fallback text, e.g. `"AL"`), `name` (fallback when
`initials` is empty), `source: image` (overrides initials), `size`
(`sm`/`md`/`lg`), `show-status`, `status-color`. Slint strings cannot be split
or indexed, so initials are computed **C++-side** and passed in via `initials`
— the component does not derive them from `name`.

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

`name`, `size`, `accent-color`, `stroke-weight`. The glyph is selected by an
`if/else if` chain over `name` inside `app-icon.slint`, each branch returning
one 24-unit viewBox stroke path drawn by a single `Path` element — no font or
image dependency, so icons stay crisp at any size (the earlier glyph-font
approach rendered as tofu under femtovg). Unknown names render a neutral
**circle + dot placeholder** (not a warning glyph), so a typo stays visible.

`AppIcons`, exported alongside `AppIcon` from `library.slint`, is the catalog:
`AppIcons.names` is a `[string]` array of every built-in name, for pickers and
galleries. The component gallery enumerates it into a reflowing grid, so a new
name shows up there automatically.

The mechanism is deliberate. Measured against Slint 1.18:

- a `for` over an array with per-element `visible:` instantiates one element per
  entry, and `visible:` lowers to an injected `Clip`, so ~2N elements — wasteful
  for a 1-of-N lookup;
- `array.index-of` / `find-index` / `any` / `all` are experimental features
  (they need `SLINT_FEATURE_EXPERIMENTAL`);
- `@image-url()` is resolved at compile time, so a runtime `name` cannot select
  an SVG file — bundling `.svg` assets would need a C++ load path plus a chain
  anyway.

The chain keeps each icon at exactly one element and adds no dependency.

To add an icon, append an `else if (name == "…")` branch in `app-icon.slint`
**and** add the name to `AppIcons.names`.

Built-in names (95):

| Group                 | Names                                                                                                                                                                                                                                             |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Navigation & arrows   | `home`, `menu`, `chevron-up`, `chevron-down`, `chevron-left`, `chevron-right`, `chevrons-left`, `chevrons-right`, `arrow-up`, `arrow-down`, `arrow-left`, `arrow-right`, `external-link`                                                          |
| Actions               | `plus`, `minus`, `close`, `check`, `edit`, `trash`, `copy`, `save`, `download`, `upload`, `refresh`, `search`, `filter`, `sort`, `link`, `share`, `more-horizontal`, `more-vertical`, `logout`, `log-in`                                            |
| Status & feedback     | `info`, `warning`, `error`, `check-circle`, `x-circle`, `help-circle`, `activity`                                                                                                                                                                 |
| Files & data          | `document`, `folder`, `clipboard`, `chart`, `pie-chart`, `database`, `image`, `trending-up`, `trending-down`                                                                                                                                       |
| Media & communication | `play`, `pause`, `stop`, `skip-back`, `skip-forward`, `volume`, `mute`, `camera`, `video`, `mail`, `bell`, `send`                                                                                                                                  |
| People                | `user`, `users`, `user-plus`                                                                                                                                                                                                                      |
| Objects & misc        | `settings`, `grid`, `calendar`, `clock`, `lock`, `unlock`, `eye`, `eye-off`, `star`, `bookmark`, `tag`, `globe`, `map-pin`, `sun`, `moon`, `cloud`, `wifi`, `battery`, `power`, `zap`, `shield`, `award`, `shopping-cart`, `credit-card`, `gift`, `package`, `layers`, `monitor`, `smartphone`, `printer`, `coffee` |

### Forms (extended)

#### AppCheckList

`items: [CheckItem{label, checked, disabled}]`, `visible-rows` (scroll cap),
`label`, `row-height`, `disabled`. Callback `item-toggled(index, checked)` —
the owner mutates the model (and any derived state) from it; per-row
`checked` may bind to individual properties so toggles fan out (see the
gallery's chain-reaction demo).

#### AppDatePicker

`date: Date` (in-out; `year == 0` = none → placeholder), `placeholder`,
`label`, `disabled`. Callbacks `accepted()`, `canceled()`. Built on the
std-widgets `DatePickerPopup`; `Date` is the std-widgets struct
(`{year, month, day}`), which maps to a plain C++ struct in the generated
API — no string parsing anywhere.

### Containers (grouping)

#### AppPanel

`title`, `icon-name`, `tone: AppPanelTone` (`surface` `tinted` `sunken`
`transparent`), `content-padding`, `bordered`, `fill` (stretch weights).
Content via `@children`. Lighter than AppCard: no shadow, no header slot.

#### AppGroupBox

`title`, `icon-name`, `content-padding`, `fill`. Content via `@children`.
Classic fieldset look: the caption floats on the top border.

### Media

#### AppImage

`source: image` (empty → themed placeholder + optional caption), `fit:
AppImageFit` (`contain` `cover` `fill`), `corner-radius`, `checkered`,
`fill`, `fallback-icon`. Set the source from C++ with
`slint::Image::load_from_path(...)` (or `load_from_embedded_data`).

### Layout

#### AppGrid / AppGridRow / AppGridItem

Bootstrap-aligned responsive grid vocabulary. `AppGrid` exposes breakpoint
flags (`at-sm/md/lg/xl`, min-width 576/768/992/1200 px) resolved against a
caller-supplied `viewport-width` — bind it to a width that never derives
from the grid's own content (the page's width), or the binding loops.
`AppGridRow` is a HorizontalLayout; `AppGridItem.span` (1..12) is the
relative width weight within its row.

```slint
grid := AppGrid {
    viewport-width: root.width;
    AppGridRow {
        AppGridItem { span: grid.at-lg ? 3 : 12; AppCard { ... } }
    }
}
```

## Responsive patterns

Slint has no CSS media queries; the idiomatic equivalents used here:

1. **Width-threshold conditionals** — e.g. the app shell:
   `collapsed: root.width < 600px` on the sidebar
2. **`AppGrid` breakpoint flags** — Bootstrap-style `at-sm/md/lg/xl` for
   span-based column layouts (see Layout above)
3. **`block` properties** — stretch controls full-width inside narrow
   containers
4. **`horizontal-stretch` / `vertical-stretch`** — let layouts redistribute
   space instead of hardcoding sizes
5. **Token-derived sizes** — preferred sizes come from `Metrics`, so a theme
   change reflows everything

## Adding a component

1. Create `src/ui/components/app-<name>.slint` following the API contract
2. Export it from `components/library.slint`
3. Add a catalog entry (this file)
4. Add it to the component-gallery page (the branches that ship one)
