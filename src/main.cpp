// main.cpp — phase-1 skeleton entry point.
//
// Replaced by the Application composition root in phase 3; kept trivial so the
// first buildable milestone stays minimal.

#include <app-window.h>

int main()
{
    auto window = app::AppWindow::create();
    window->run();
    return 0;
}
