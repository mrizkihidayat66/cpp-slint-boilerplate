// TestServices.cpp — service logic (headless, no UI backend needed).

#include <catch2/catch_test_macros.hpp>

#include "services/AboutService.h"
#include "services/DashboardService.h"

using cpp_slint_boilerplate::services::AboutService;
using cpp_slint_boilerplate::services::DashboardService;

TEST_CASE("DashboardService returns non-empty demo data", "[services]")
{
    DashboardService service;
    service.initialize();

    const auto cards = service.statCards();
    REQUIRE(!cards.empty());
    for (const auto &card : cards) {
        // KPI tiles are useless without a title and a value.
        CHECK(!card.title.empty());
        CHECK(!card.value.empty());
    }

    const auto activity = service.recentActivity();
    REQUIRE(!activity.empty());
    for (const auto &item : activity) {
        CHECK(!item.actor.empty());
        CHECK(!item.action.empty());
    }
}

TEST_CASE("AboutService reports build metadata after initialize", "[services]")
{
    AboutService service;
    service.initialize();

    const auto &info = service.aboutInfo();
    CHECK(info.applicationName == "cpp-slint-boilerplate");
    CHECK(!info.version.empty());
    CHECK(!info.slintVersion.empty());
    CHECK((info.buildType == "Debug" || info.buildType == "Release"));
}

TEST_CASE("Service lifecycle hooks are safe no-ops for demo services", "[services]")
{
    DashboardService dashboard;
    dashboard.initialize();
    dashboard.shutdown();

    AboutService about;
    about.initialize();
    about.shutdown();

    SUCCEED("no crashes, no exceptions");
}
