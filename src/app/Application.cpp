// Application.cpp — see Application.h.

#include "app/Application.h"

#include <cstdlib>

#include <slint.h>

#include "app/AppMetadata.h"
#include "models/AboutModels.h"
#include "models/DashboardModels.h"
#include "models/SettingsModels.h"
#include "services/AboutService.h"
#include "services/DashboardService.h"
#include "services/SettingsService.h"

namespace cpp_slint_boilerplate::app {

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
}

void Application::bindUiCallbacks()
{
    auto &window = **m_window;
    auto &state = window.global<::app::AppState>();

    // BOILERPLATE_START_PAGE selects the initial page (screenshot harness,
    // CI smoke runs). Page indices mirror the sidebar entries.
    if (const char *startPage = std::getenv("BOILERPLATE_START_PAGE")) {
        const int page = std::atoi(startPage);
        if (page >= 0 && page < 4) {
            state.set_current_page(page);
        }
    }

    state.on_request_navigation([&state](int page) {
        // Page indices mirror the sidebar entries (see app-window.slint).
        if (page >= 0 && page < 4) {
            state.set_current_page(page);
        }
    });

    state.on_request_quit([] { slint::quit_event_loop(); });

    // Settings: persist the values the settings page committed to AppState.
    // `this` is captured for pushSettings (it touches m_window for the
    // Palette theme switch).
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

    // Settings: push the loaded values (triggers the theme via changed
    // callbacks on the settings page; Palette.dark is also set directly so
    // the theme applies even before the settings page is instantiated).
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
