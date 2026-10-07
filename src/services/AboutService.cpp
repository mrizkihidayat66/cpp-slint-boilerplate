// AboutService.cpp — build/runtime metadata.

#include "services/AboutService.h"

#include <slint/config.h>

namespace cpp_slint_boilerplate::services {

void AboutService::initialize()
{
    m_info = {
        .applicationName = "cpp-slint-boilerplate",
        .version = "0.1.0",
        .description = "A professional C++20 + Slint desktop application boilerplate.",
        .slintVersion = slint::version_string(),
        .compiler =
#if defined(_MSC_VER)
            "MSVC " + std::to_string(_MSC_VER),
#elif defined(__clang__)
            "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__),
#elif defined(__GNUC__)
            "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__),
#else
            "Unknown compiler",
#endif
        .buildType =
#if defined(NDEBUG)
            "Release",
#else
            "Debug",
#endif
    };
}

void AboutService::shutdown()
{
    // Nothing to flush.
}

const models::AboutInfo &AboutService::aboutInfo() const
{
    return m_info;
}

} // namespace cpp_slint_boilerplate::services
