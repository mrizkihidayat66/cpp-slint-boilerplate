// AboutService.cpp — build/runtime metadata.

#include "services/AboutService.h"

#include "app/AppMetadata.h"

namespace cpp_slint_boilerplate::services {

void AboutService::initialize()
{
    m_info = {
        .applicationName = std::string(app::kTechnicalName),
        .version = std::string(app::kVersion),
        .description = "A professional C++20 + Slint desktop application boilerplate.",
        // The Slint C++ API exposes no public runtime version accessor; the
        // fetched release is recorded in CMake/FetchSlint.cmake. Keep this
        // string in sync when pinning SLINT_FETCH_TAG.
        .slintVersion = "1.18.1",
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
