// DashboardService.h — provides the dashboard's data.

#pragma once

#include <vector>

#include "models/DashboardModels.h"
#include "services/IService.h"

namespace cpp_slint_boilerplate::services {

/// Serves KPI cards and the recent-activity feed. On the main branch this
/// returns static demo data; the example branches swap in real sources.
class DashboardService : public IService
{
public:
    void initialize() override;
    void shutdown() override;

    /// KPI tiles, in display order.
    [[nodiscard]] std::vector<models::StatCard> statCards() const;

    /// Recent activity, newest first.
    [[nodiscard]] std::vector<models::ActivityItem> recentActivity() const;
};

} // namespace cpp_slint_boilerplate::services
