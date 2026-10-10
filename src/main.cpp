// main.cpp — the entire C++ side of the simple example.
//
// Create the window, push the app identity, run the event loop. Everything
// else — layout, data, navigation — lives in src/ui/*.slint. Compare this with
// the intermediate/advanced branches to see when the Application /
// ServiceLocator architecture starts paying for itself.

#include <slint.h>

#include "app-window.h"
#include "app/AppMetadata.h"

int main()
{
    auto window = ::app::AppWindow::create();

    // Application identity — the single CMakeLists.txt definition, pushed to
    // the UI so the window title, sidebar brand and About card all agree.
    auto &info = window->global<::app::AppInfo>();
    info.set_display_name(slint::SharedString(cpp_slint_boilerplate::app::kDisplayName));
    info.set_technical_name(slint::SharedString(cpp_slint_boilerplate::app::kTechnicalName));
    info.set_version(slint::SharedString(cpp_slint_boilerplate::app::kVersion));

    window->run();
    return 0;
}
