// Application.cpp — see Application.h.

#include "app/Application.h"

#include <slint.h>

#include "models/AboutModels.h"
#include "models/DashboardModels.h"
#include "services/AboutService.h"
#include "services/DashboardService.h"

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
}

void Application::bindUiCallbacks()
{
    auto &window = **m_window;
    auto &state = window.global<::app::AppState>();

    state.on_request_navigation([&state](int page) {
        // Page indices mirror the sidebar entries (see app-window.slint).
        if (page >= 0 && page < 3) {
            state.set_current_page(page);
        }
    });

    state.on_request_quit([] { slint::quit_event_loop(); });
}

void Application::loadInitialData()
{
    auto &window = **m_window;
    const auto &dashboard = m_services.get<services::DashboardService>();
    window.set_stat_cards(models::toSlintModel(dashboard->statCards()));

    std::vector<::app::ActivityItem> activity;
    for (const auto &item : dashboard->recentActivity()) {
        ::app::ActivityItem converted;
        converted.actor = slint::SharedString(item.actor);
        converted.action = slint::SharedString(item.action);
        converted.timestamp = slint::SharedString(item.timestamp);
        converted.icon_name = slint::SharedString(item.iconName);
        activity.push_back(std::move(converted));
    }
    window.set_recent_activity(
            std::make_shared<slint::VectorModel<::app::ActivityItem>>(std::move(activity)));

    const auto &about = m_services.get<services::AboutService>()->aboutInfo();
    auto &state = window.global<::app::AppState>();
    state.set_about_text(slint::SharedString(
            about.applicationName + " v" + about.version + "\n" + about.description + "\n\n"
            + "Slint: " + about.slintVersion + "\nCompiler: " + about.compiler + "\nBuild: "
            + about.buildType));
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
