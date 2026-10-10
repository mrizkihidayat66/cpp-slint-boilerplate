// Application.cpp — see Application.h.

#include "app/Application.h"

#include <cstdlib>

#include <slint.h>

#include "app/AppMetadata.h"
#include "models/AboutModels.h"
#include "models/DashboardModels.h"
#include "models/SettingsModels.h"
#include "models/TaskModels.h"
#include "services/AboutService.h"
#include "services/DashboardService.h"
#include "services/SettingsService.h"
#include "services/TaskRepository.h"
#include "services/TaskService.h"
#include "services/TaskValidationService.h"

namespace cpp_slint_boilerplate::app {

namespace {

/// Sidebar entries: 0 Dashboard, 1 Tasks, 2 Components, 3 Settings, 4 About.
constexpr int kPageCount = 5;

models::TaskSortColumn sortColumnFromIndex(int index)
{
    switch (index) {
    case 1:
        return models::TaskSortColumn::Title;
    case 2:
        return models::TaskSortColumn::Assignee;
    case 3:
        return models::TaskSortColumn::Status;
    case 0:
    default:
        return models::TaskSortColumn::Id;
    }
}

models::TaskStatus statusFromIndex(int index)
{
    switch (index) {
    case 1:
        return models::TaskStatus::InProgress;
    case 2:
        return models::TaskStatus::Done;
    case 0:
    default:
        return models::TaskStatus::Todo;
    }
}

int statusToIndex(models::TaskStatus status)
{
    switch (status) {
    case models::TaskStatus::InProgress:
        return 1;
    case models::TaskStatus::Done:
        return 2;
    case models::TaskStatus::Todo:
    default:
        return 0;
    }
}

} // namespace

Application::Application()
{
    m_window = ::app::AppWindow::create();
    registerServices();
    m_services.initializeAll();
    bindUiCallbacks();
    loadInitialData();
}

Application::~Application()
{
    m_services.shutdownAll();
}

void Application::registerServices()
{
    m_services.registerConstructed<services::DashboardService>();
    m_services.registerConstructed<services::AboutService>();
    m_services.registerConstructed<services::SettingsService>();

    // Task chain — registration order is initialization order:
    // repository and validation first, the orchestrating service last.
    const auto repository = m_services.registerConstructed<services::TaskRepository>();
    const auto validation = m_services.registerConstructed<services::TaskValidationService>();
    m_services.registerService<services::TaskService>(
        std::make_shared<services::TaskService>(repository, validation));
}

void Application::bindUiCallbacks()
{
    auto &window = **m_window;
    auto &state = window.global<::app::AppState>();

    // BOILERPLATE_START_PAGE selects the initial page (screenshot harness,
    // CI smoke runs). Page indices mirror the sidebar entries.
    if (const char *startPage = std::getenv("BOILERPLATE_START_PAGE")) {
        const int page = std::atoi(startPage);
        if (page >= 0 && page < kPageCount) {
            state.set_current_page(page);
        }
    }

    // BOILERPLATE_GALLERY_SCROLL scrolls the gallery to a 0..1 fraction of
    // its scrollable height (screenshot harness; wheel automation is flaky).
    if (const char *scroll = std::getenv("BOILERPLATE_GALLERY_SCROLL")) {
        const float fraction = static_cast<float>(std::atof(scroll));
        if (fraction > 0.0f && fraction <= 1.0f) {
            state.set_gallery_scroll(fraction);
        }
    }

    state.on_request_navigation([&state](int page) {
        // Page indices mirror the sidebar entries (see app-window.slint).
        if (page >= 0 && page < kPageCount) {
            state.set_current_page(page);
        }
    });

    state.on_request_quit([] { slint::quit_event_loop(); });

    // Settings: persist the values the settings page committed to AppState.
    // `this` is captured for pushSettings (it flips the Palette theme flag).
    state.on_request_save_settings([this, &state]() {
        const auto settings = m_services.get<services::SettingsService>();

        models::AppSettings updated;
        updated.displayName = std::string(state.get_display_name());
        updated.theme = state.get_dark() ? "dark" : "light";
        updated.language = state.get_language_index() == 1 ? "id" : "en";
        updated.notifications = state.get_notifications();
        updated.autoStart = state.get_auto_start();
        settings->save(updated);

        // Confirm with the normalized values the service now owns.
        pushSettings(state, settings->settings());
    });

    // Settings: discard unsaved edits — re-push what the service owns.
    state.on_request_reload_settings([this, &state]() {
        const auto settings = m_services.get<services::SettingsService>();
        pushSettings(state, settings->settings());
    });

    // Screenshot harness / smoke runs: open the Tasks page's form or delete
    // confirmation without user interaction. "form" shows a create dialog
    // armed with a failing draft (per-field error states visible); "confirm"
    // shows the delete confirmation for the first seeded task.
    if (const char *dialog = std::getenv("BOILERPLATE_TASKS_DIALOG")) {
        state.set_current_page(1);
        const std::string spec(dialog);
        if (spec == "confirm") {
            const auto task = m_services.get<services::TaskService>()->find(1);
            if (task) {
                state.set_tasks_delete_id(task->id);
                state.set_tasks_delete_title(slint::SharedString(task->title));
                state.set_tasks_confirm_visible(true);
            }
        } else if (spec == "form") {
            state.set_task_form_title(slint::SharedString("ab"));
            state.set_task_form_assignee(slint::SharedString(""));
            state.set_task_form_title_error(
                slint::SharedString("Title must be at least 3 characters"));
            state.set_task_form_assignee_error(slint::SharedString("Assignee is required"));
            state.set_tasks_form_visible(true);
        }
    }

    // --- Tasks: reads (sort / page) ------------------------------------------
    state.on_request_task_sort([this, &state](int column, bool ascending) {
        m_sortColumn = sortColumnFromIndex(column);
        m_sortAscending = ascending;
        state.set_tasks_sort_column(column);
        state.set_tasks_sort_ascending(ascending);
        pushTasks(state);
    });

    state.on_request_task_page([this, &state](int page) {
        m_taskPage = page;
        pushTasks(state);
    });

    // --- Tasks: dialog intents -------------------------------------------------
    state.on_request_task_create([this, &state]() {
        state.set_task_form_id(0);
        state.set_task_form_title(slint::SharedString(""));
        state.set_task_form_description(slint::SharedString(""));
        state.set_task_form_assignee(slint::SharedString(""));
        state.set_task_form_status_index(0);
        clearFormErrors(state);
        state.set_status_message(slint::SharedString(""));
        state.set_tasks_form_visible(true);
    });

    state.on_request_task_edit_by_cell([this, &state](slint::SharedString idCell) {
        const auto task =
            m_services.get<services::TaskService>()->find(std::atoi(std::string(idCell).c_str()));
        if (!task) {
            state.set_status_message(slint::SharedString("That task no longer exists"));
            return;
        }
        state.set_task_form_id(task->id);
        state.set_task_form_title(slint::SharedString(task->title));
        state.set_task_form_description(slint::SharedString(task->description));
        state.set_task_form_assignee(slint::SharedString(task->assignee));
        state.set_task_form_status_index(statusToIndex(task->status));
        clearFormErrors(state);
        state.set_status_message(slint::SharedString(""));
        state.set_tasks_form_visible(true);
    });

    state.on_request_task_submit([this, &state]() {
        const auto tasks = m_services.get<services::TaskService>();

        models::Task draft;
        draft.id = state.get_task_form_id();
        draft.title = std::string(state.get_task_form_title());
        draft.description = std::string(state.get_task_form_description());
        draft.assignee = std::string(state.get_task_form_assignee());
        draft.status = statusFromIndex(state.get_task_form_status_index());

        const auto result = draft.id == 0 ? tasks->create(draft) : tasks->update(draft);
        if (!result.success) {
            // Per-field messages land under the inputs; the summary (e.g.
            // duplicate title) goes to the top of the dialog.
            state.set_task_form_title_error(slint::SharedString(result.errors.messageFor("title")));
            state.set_task_form_assignee_error(
                slint::SharedString(result.errors.messageFor("assignee")));
            state.set_task_form_error(slint::SharedString(result.errorMessage));
            return;
        }

        state.set_status_message(slint::SharedString(draft.id == 0
                                                         ? "Created “" + result.task.title + "”"
                                                         : "Updated “" + result.task.title + "”"));
        clearFormErrors(state);
        state.set_task_form_id(0);
        state.set_tasks_form_visible(false);
        pushTasks(state);
    });

    state.on_request_task_cancel([this, &state]() {
        state.set_task_form_id(0);
        clearFormErrors(state);
    });

    // Edit dialog's Delete button: stage the task for deletion and show the
    // confirmation dialog (its Delete button fires request-task-delete).
    state.on_request_task_confirm_delete([this, &state]() {
        const auto existing =
            m_services.get<services::TaskService>()->find(state.get_task_form_id());
        if (!existing) {
            state.set_status_message(slint::SharedString("Task no longer exists"));
            state.set_tasks_form_visible(false);
            return;
        }
        state.set_tasks_delete_id(existing->id);
        state.set_tasks_delete_title(slint::SharedString(existing->title));
        state.set_tasks_confirm_visible(true);
    });

    state.on_request_task_delete([this, &state](int id) {
        const auto tasks = m_services.get<services::TaskService>();
        const auto existing = tasks->find(id);
        const bool removed = existing && tasks->remove(id);
        state.set_status_message(slint::SharedString(removed ? "Deleted “" + existing->title + "”"
                                                             : "Task no longer exists"));
        if (removed) {
            // A deletion from the form also closes it.
            state.set_tasks_form_visible(false);
        }
        pushTasks(state);
    });
}

void Application::loadInitialData()
{
    auto &window = **m_window;

    // Application identity — the single CMakeLists.txt definition, pushed to
    // the UI so the window title, sidebar brand and About card all agree.
    auto &info = window.global<::app::AppInfo>();
    info.set_display_name(slint::SharedString(app::kDisplayName));
    info.set_technical_name(slint::SharedString(app::kTechnicalName));
    info.set_version(slint::SharedString(app::kVersion));

    const auto &dashboard = m_services.get<services::DashboardService>();
    window.set_stat_cards(models::toSlintModel(dashboard->statCards()));

    std::vector<::app::ActivityItem> activity;
    for (const auto &item : dashboard->recentActivity()) {
        ::app::ActivityItem converted;
        converted.actor = slint::SharedString(item.actor);
        converted.initials = slint::SharedString(models::initialsOf(item.actor));
        converted.action = slint::SharedString(item.action);
        converted.timestamp = slint::SharedString(item.timestamp);
        converted.icon_name = slint::SharedString(item.iconName);
        activity.push_back(std::move(converted));
    }
    window.set_recent_activity(
        std::make_shared<slint::VectorModel<::app::ActivityItem>>(std::move(activity)));

    const auto &about = m_services.get<services::AboutService>()->aboutInfo();
    auto &state = window.global<::app::AppState>();
    // Structured fields — the About page lays them out as key/value rows.
    state.set_about_description(slint::SharedString(about.description));
    state.set_about_version(slint::SharedString(about.version));
    state.set_about_slint_version(slint::SharedString(about.slintVersion));
    state.set_about_compiler(slint::SharedString(about.compiler));
    state.set_about_build_type(slint::SharedString(about.buildType));

    // Tasks: first page, default sort (id = created order).
    pushTasks(state);

    // Settings: push the loaded values (Palette.dark is set directly so the
    // theme applies even before the settings page is instantiated).
    pushSettings(state, m_services.get<services::SettingsService>()->settings());
}

void Application::pushSettings(const ::app::AppState &state, const models::AppSettings &settings)
{
    state.set_display_name(slint::SharedString(settings.displayName));
    state.set_dark(settings.theme == "dark");
    state.set_language_index(settings.language == "id" ? 1 : 0);
    state.set_notifications(settings.notifications);
    state.set_auto_start(settings.autoStart);

    // Live theme switch: every Palette token reacts immediately.
    (**m_window).global<::app::Palette>().set_dark(settings.theme == "dark");
}

void Application::pushTasks(const ::app::AppState &state)
{
    const auto tasks = m_services.get<services::TaskService>();
    const auto direction = m_sortAscending ? models::TaskSortDirection::Ascending
                                           : models::TaskSortDirection::Descending;
    const auto page = tasks->page(m_sortColumn, direction, m_taskPage);

    state.set_tasks_rows(models::taskRowsToModel(page.tasks));
    state.set_tasks_total(page.totalCount);
    state.set_tasks_page(page.pageNumber);
    state.set_tasks_page_count(page.pageCount);
    m_taskPage = page.pageNumber; // clamped by the service when out of range
}

void Application::clearFormErrors(const ::app::AppState &state)
{
    state.set_task_form_title_error(slint::SharedString(""));
    state.set_task_form_assignee_error(slint::SharedString(""));
    state.set_task_form_error(slint::SharedString(""));
}

int Application::run()
{
    (**m_window).run();
    return 0;
}

void Application::quit()
{
    slint::quit_event_loop();
}

} // namespace cpp_slint_boilerplate::app
