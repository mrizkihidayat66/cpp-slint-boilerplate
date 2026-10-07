// Application.h — composition root: owns services, wires the UI, runs.

#pragma once

#include <memory>

#include "app/ServiceLocator.h"
#include "app-window.h"

namespace cpp_slint_boilerplate::app {

/// Bootstraps the application:
///   1. registerServices()  — fill the ServiceLocator
///   2. initializeAll()     — run service startup hooks in order
///   3. bindUiCallbacks()   — connect AppState callbacks to services
///   4. loadInitialData()   — push service data into UI models
///   5. run_event_loop()    — hand control to Slint
class Application
{
public:
    Application();
    ~Application();

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    /// Runs until the window is closed. Returns the process exit code.
    int run();

    /// Requests a graceful quit (usable from callbacks).
    void quit();

private:
    void registerServices();
    void bindUiCallbacks();
    void loadInitialData();

    ServiceLocator m_services;
    std::shared_ptr<app::AppWindow> m_window;
};

} // namespace cpp_slint_boilerplate::app
