// TestServices.cpp — service logic (headless, no UI backend needed).

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "services/AboutService.h"
#include "services/DashboardService.h"
#include "services/SettingsService.h"

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

// --- SettingsService (examples/intermediate) ---------------------------------

using cpp_slint_boilerplate::services::SettingsService;

TEST_CASE("SettingsService JSON round-trips every field", "[services][settings]")
{
    cpp_slint_boilerplate::models::AppSettings original;
    original.displayName = "Bjarne Stroustrup";
    original.theme = "dark";
    original.language = "id";
    original.notifications = false;
    original.autoStart = true;

    const auto json = SettingsService::toJson(original);
    const auto parsed = SettingsService::parseJson(json);
    REQUIRE(parsed.has_value());
    CHECK(parsed->displayName == original.displayName);
    CHECK(parsed->theme == original.theme);
    CHECK(parsed->language == original.language);
    CHECK(parsed->notifications == original.notifications);
    CHECK(parsed->autoStart == original.autoStart);
}

TEST_CASE("SettingsService parseJson rejects documents without theme", "[services][settings]")
{
    // theme/language drive UI logic — a missing value must not fall through
    // to defaults silently.
    CHECK_FALSE(SettingsService::parseJson(R"({ "displayName": "x" })").has_value());
    CHECK(SettingsService::parseJson(R"({ "theme": "light", "language": "en" })").has_value());
}

TEST_CASE("SettingsService persist survives re-initialize (reload)", "[services][settings]")
{
    // Point the service at a temp directory (no real %APPDATA% writes).
    const auto tempDir = std::filesystem::temp_directory_path() / "boilerplate-settings-test";
    std::filesystem::create_directories(tempDir);

    cpp_slint_boilerplate::models::AppSettings saved;
    saved.displayName = "Grace Hopper";
    saved.theme = "dark";

    {
        SettingsService writer(tempDir.string());
        writer.initialize();
        writer.save(saved);
    }

    SettingsService reader(tempDir.string());
    reader.initialize();
    CHECK(reader.settings().displayName == "Grace Hopper");
    CHECK(reader.settings().theme == "dark");
    CHECK(reader.filePath().find("settings.json") != std::string::npos);
}
