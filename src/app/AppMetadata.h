// AppMetadata.h — application identity, defined once in CMakeLists.txt.
//
// The macros arrive via target_compile_definitions() on boilerplate-core. The
// #ifndef fallbacks keep the header usable in a translation unit that does not
// link that target.

#pragma once

#include <string_view>

#ifndef APP_TECHNICAL_NAME
#define APP_TECHNICAL_NAME "cpp-slint-boilerplate"
#endif
#ifndef APP_DISPLAY_NAME
#define APP_DISPLAY_NAME "Boilerplate"
#endif
#ifndef APP_VERSION
#define APP_VERSION "0.1.0"
#endif

namespace cpp_slint_boilerplate::app {

/// Machine-facing name: window title, About card, %APPDATA% directory.
inline constexpr std::string_view kTechnicalName = APP_TECHNICAL_NAME;
/// Short brand shown in the sidebar header.
inline constexpr std::string_view kDisplayName = APP_DISPLAY_NAME;
/// Application version (PROJECT_VERSION in CMakeLists.txt).
inline constexpr std::string_view kVersion = APP_VERSION;

} // namespace cpp_slint_boilerplate::app
