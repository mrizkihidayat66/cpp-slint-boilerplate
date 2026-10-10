// SettingsService.cpp — %APPDATA% JSON persistence, hand-rolled.

#include "services/SettingsService.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>

namespace cpp_slint_boilerplate::services {
namespace {

/// %APPDATA% (Roaming) or empty when the variable is not set.
std::string appDataDirectory()
{
    if (const char *appData = std::getenv("APPDATA")) {
        return appData;
    }
    return {};
}

} // namespace

SettingsService::SettingsService(std::string directoryOverride)
    : m_directoryOverride(std::move(directoryOverride))
{
}

void SettingsService::initialize()
{
    // Resolve the settings directory: real %APPDATA%, or the test override.
    const std::string base =
        !m_directoryOverride.empty() ? m_directoryOverride : appDataDirectory();
    if (base.empty()) {
        // No APPDATA (service context?): fall back to defaults in memory.
        m_filePath.clear();
        m_settings = {};
        return;
    }

    m_filePath = base + "\\cpp-slint-boilerplate\\settings.json";

    // Ensure the directory exists before any read/write — on a first run
    // %APPDATA%\cpp-slint-boilerplate\ does not yet exist, and a silent
    // ofstream failure would make every save() a no-op.
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(m_filePath).parent_path(), ec);

    std::ifstream file(m_filePath);
    if (!file) {
        // First run: write the defaults so the file exists from now on.
        m_settings = {};
        save(m_settings);
        return;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (auto parsed = parseJson(buffer.str())) {
        m_settings = *parsed;
    } else {
        // Corrupt file: keep defaults, rewrite on next save.
        m_settings = {};
    }
}

void SettingsService::shutdown()
{
    // Nothing to flush — save() is eager.
}

const models::AppSettings &SettingsService::settings() const
{
    return m_settings;
}

void SettingsService::save(const models::AppSettings &settings)
{
    m_settings = settings;
    if (m_filePath.empty()) {
        return;
    }

    // Atomic write: .tmp then replace, so a crash mid-write cannot corrupt
    // the previous settings.
    const std::string tmpPath = m_filePath + ".tmp";
    {
        std::ofstream tmp(tmpPath, std::ios::trunc);
        if (!tmp) {
            return; // Directory missing or locked — settings stay in memory.
        }
        tmp << toJson(m_settings);
    }
    std::remove(m_filePath.c_str());
    std::rename(tmpPath.c_str(), m_filePath.c_str());
}

const std::string &SettingsService::filePath() const
{
    return m_filePath;
}

std::string SettingsService::toJson(const models::AppSettings &settings)
{
    std::ostringstream json;
    json << "{\n"
         << "  \"displayName\": \"" << settings.displayName << "\",\n"
         << "  \"theme\": \"" << settings.theme << "\",\n"
         << "  \"language\": \"" << settings.language << "\",\n"
         << "  \"notifications\": " << (settings.notifications ? "true" : "false") << ",\n"
         << "  \"autoStart\": " << (settings.autoStart ? "true" : "false") << "\n"
         << "}\n";
    return json.str();
}

std::optional<models::AppSettings> SettingsService::parseJson(const std::string &json)
{
    const auto displayName = jsonString(json, "displayName");
    const auto theme = jsonString(json, "theme");
    const auto language = jsonString(json, "language");
    const auto notifications = jsonBool(json, "notifications");
    const auto autoStart = jsonBool(json, "autoStart");

    // Theme and language drive UI logic — treat a missing value as invalid
    // so we fall back to defaults instead of an unknown enum state.
    if (!theme || !language) {
        return std::nullopt;
    }

    models::AppSettings settings;
    if (displayName) {
        settings.displayName = *displayName;
    }
    settings.theme = *theme;
    settings.language = *language;
    if (notifications) {
        settings.notifications = *notifications;
    }
    if (autoStart) {
        settings.autoStart = *autoStart;
    }
    return settings;
}

std::optional<std::string> SettingsService::jsonString(const std::string &json,
                                                       const std::string &key)
{
    const std::string needle = "\"" + key + "\"";
    std::size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    const std::size_t start = pos + 1;
    const std::size_t end = json.find('"', start);
    if (end == std::string::npos) {
        return std::nullopt;
    }
    return json.substr(start, end - start);
}

std::optional<bool> SettingsService::jsonBool(const std::string &json, const std::string &key)
{
    const std::string needle = "\"" + key + "\"";
    std::size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos += 1;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
        ++pos;
    }
    if (json.compare(pos, 4, "true") == 0) {
        return true;
    }
    if (json.compare(pos, 5, "false") == 0) {
        return false;
    }
    return std::nullopt;
}

} // namespace cpp_slint_boilerplate::services
