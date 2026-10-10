// SettingsModels.h — persisted application settings.

#pragma once

#include <string>

namespace cpp_slint_boilerplate::models {

/// Everything the settings page edits, persisted to %APPDATA% as JSON.
struct AppSettings {
    std::string displayName = "Admin"; // shown in the sidebar footer
    std::string theme = "light";       // "light" | "dark"
    std::string language = "en";       // "en" | "id"
    bool notifications = true;
    bool autoStart = false;
};

} // namespace cpp_slint_boilerplate::models
