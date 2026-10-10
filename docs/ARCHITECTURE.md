# Architecture

## Layers

```
┌──────────────────────────────────────────────┐
│ UI (src/ui/*.slint)                          │
│   app-window · pages · components · theme    │
└──────────────△───────────────┬───────────────┘
      models / props      callbacks (intents)
┌──────────────┴───────────────△───────────────┐
│ Generated types (build dir, namespace app)   │
└──────────────┬───────────────┴───────────────┘
┌──────────────△───────────────┬───────────────┐
│ Application (src/app)                        │
│   composition root: Application + Locator    │
└──────────────┬───────────────┴───────────────┘
┌──────────────△───────────────────────────────┐
│ Services (src/services)                      │
│   business logic, IService lifecycle         │
└──────────────┬───────────────────────────────┘
┌──────────────△───────────────────────────────┐
│ Models (src/models)                          │
│   pure-C++ POCOs + toSlint() adapters        │
└──────────────────────────────────────────────┘
```

Dependency direction is strictly downward. Services know nothing about the
UI; the UI knows nothing about services.

## Startup flow

[`Application`](../src/app/Application.h) is the single composition root:

1. `registerServices()` — construct services into the `ServiceLocator`
2. `m_services.initializeAll()` — run each `IService::initialize()` in
   registration order (reverse order on shutdown)
3. `bindUiCallbacks()` — connect `AppState` callbacks to service calls
4. `loadInitialData()` — pull service data, convert with `toSlint()` /
   `toSlintModel()`, push into window properties as `VecModel`s
5. `run()` — the Slint event loop until window close

## Data flow: UI ⇄ C++

Two channels, each with one direction of authority:

**C++ → UI (state & data):** C++ owns the data. Window-level model properties
(`stat-cards`, `recent-activity`) are set from `Application` with
`slint::VecModel` instances; pages consume them read-only. Global state
(`AppState.current-page`, `AppState.about-description`, `AppState.about-version`, …) is set via property setters.

**UI → C++ (intents):** the UI never mutates business state directly. It
invokes callbacks (`request-navigation(int)`, `request-quit()`); `Application`
validates and applies the change back through the state channel. This
round-trip is the state-management pattern the intermediate example
generalizes.

The golden rules (enforced in review, see [CODING_STYLE.md](CODING_STYLE.md)):

1. No business logic in `.slint` files
2. No UI types in `services/` or `models/` beyond the `toSlint()` adapters

## The model adapter pattern

Structs are declared once in [`app-state.slint`](../src/ui/app-state.slint)
and mirrored by POCOs in [`src/models/`](../src/models):

```slint
export struct StatCard { title: string, value: string, delta: string,
                          trend-positive: bool, icon-name: string }
```

```cpp
struct StatCard {
    std::string title, value, delta;
    bool trendPositive = true;
    std::string iconName;
};

app::StatCard toSlint(const StatCard &);                 // single
std::shared_ptr<slint::Model<app::StatCard>> toSlintModel(
        const std::vector<StatCard> &);                  // collection
```

Consequences:

- Services stay testable with Catch2, no windowing backend
- Updating a list from C++ means mutating the `VecModel` (or replacing it),
  and the UI re-renders automatically — the C++ side is the single owner

## Service chains

Services may depend on each other by constructor injection — the advanced
example demonstrates the chain
`TaskRepository → TaskValidationService → TaskService`, all resolved through
the same `ServiceLocator`. Register the dependency first:

```cpp
m_services.registerConstructed<services::TaskRepository>();
m_services.registerConstructed<services::TaskValidationService>(
        m_services.get<services::TaskRepository>());
m_services.registerConstructed<services::TaskService>(
        m_services.get<services::TaskRepository>(),
        m_services.get<services::TaskValidationService>());
```

Initialization order follows registration order, so dependencies are always
initialized before their dependents.

## Adding a page (worked example)

1. **Structs/state** — if the page needs data, declare the struct in
   `app-state.slint` and mirror the POCO in `src/models/`
2. **Service** — add `XService` (`IService` + pure-C++ accessors) in
   `src/services/`; register it in `Application::registerServices()`
3. **Page** — create `src/ui/pages/x-page.slint`, export it in
   `pages.slint`, add the sidebar entry and `if current-page == N` switch in
   `app-window.slint`
4. **Data** — add a window-level model property, feed it from
   `loadInitialData()` through `toSlintModel()`
5. **Intents** — declare callbacks on `AppState`, bind them in
   `bindUiCallbacks()`
6. **Tests** — Catch2 coverage for the new service's logic

## Testing strategy

| Layer            | Mechanism                                                    |
| ---------------- | ------------------------------------------------------------ |
| `.slint` markup | Compiled at build time by `slint_target_sources` — syntax/type errors break the build |
| Models/services | Catch2 unit tests (`tests/unit/`), headless                  |
| Runtime          | Manual run + CI smoke (software renderer)                    |
