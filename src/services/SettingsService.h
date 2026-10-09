// SettingsService.h — loads and persists settings to %APPDATA% JSON.

#pragma once

#include <optional>
#include <string>

#include "models/SettingsModels.h"
#include "services/IService.h"

namespace cpp_slint_boilerplate::services {

/// Owns the AppSettings POCO. initialize() loads (or creates) the JSON file;
/// every save() call writes the current state back atomically (write to a
/// .tmp, then replace), so a crash never corrupts the file.
///
/// JSON shape (hand-rolled — no dependency needed for six keys):
///   { "displayName": "...", "theme": "light", "language": "en",
///     "notifications": true, "autoStart": false }
class SettingsService : public IService
{
public:
    void initialize() override;
    void shutdown() override;

    /// Current settings (loaded in initialize(), defaults if the file was
    /// missing or unreadable).
    [[nodiscard]] const models::AppSettings &settings() const;

    /// Persist the given settings and make them current.
    void save(const models::AppSettings &settings);

    /// The JSON file path used by this instance. Public for tests: the
    /// constructor takes a directory override so tests can point it at a
    /// temp dir instead of the real %APPDATA%.
    [[nodiscard]] const std::string &filePath() const;

    /// Parse a settings JSON document. Exposed for unit tests.
    [[nodiscard]] static std::optional<models::AppSettings> parseJson(const std::string &json);

    /// Serialize settings to a JSON document. Exposed for unit tests.
    [[nodiscard]] static std::string toJson(const models::AppSettings &settings);

    /// Extract a JSON string value for a key ("key": "value").
    [[nodiscard]] static std::optional<std::string> jsonString(const std::string &json,
                                                               const std::string &key);

    /// Extract a JSON bool value for a key ("key": true|false).
    [[nodiscard]] static std::optional<bool> jsonBool(const std::string &json,
                                                      const std::string &key);

    explicit SettingsService(std::string directoryOverride = {});

private:
    models::AppSettings m_settings;
    std::string m_directoryOverride;
    std::string m_filePath;
};

} // namespace cpp_slint_boilerplate::services
