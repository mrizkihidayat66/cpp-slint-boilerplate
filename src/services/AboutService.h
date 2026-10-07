// AboutService.h — application metadata provider.

#pragma once

#include "models/AboutModels.h"
#include "services/IService.h"

namespace cpp_slint_boilerplate::services {

/// Supplies the About page with build/runtime metadata.
class AboutService : public IService
{
public:
    void initialize() override;
    void shutdown() override;

    /// Gathered at initialize() time.
    [[nodiscard]] const models::AboutInfo &aboutInfo() const;

private:
    models::AboutInfo m_info;
};

} // namespace cpp_slint_boilerplate::services
