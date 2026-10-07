// main.cpp — the entire C++ side of the simple example.
//
// Create the window, run the event loop. That's it. Everything else —
// layout, data, navigation — lives in src/ui/*.slint. Compare this with
// the intermediate/advanced branches to see when the Application /
// ServiceLocator architecture starts paying for itself.

#include <slint.h>

#include "app-window.h"

int main()
{
    auto window = ::app::AppWindow::create();
    window->run();
    return 0;
}
