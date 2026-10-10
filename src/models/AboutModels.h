// AboutModels.h — application metadata.

#pragma once

#include <string>
#include <vector>

namespace cpp_slint_boilerplate::models {

/// Information shown on the About page.
struct AboutInfo {
    std::string applicationName;
    std::string version;
    std::string description;
    std::string slintVersion; // runtime Slint version string
    std::string compiler;     // build compiler description
    std::string buildType;    // Debug / Release
};

/// A link entry rendered at the bottom of the About page.
struct AboutLink {
    std::string label;
    std::string url;
};

} // namespace cpp_slint_boilerplate::models
