// DashboardModels.h — pure C++ data for the dashboard page.
//
// POCOs only: no Slint types live here (beyond the adapter below), so the
// services layer stays testable without a UI backend. The `toSlint`
// adapters are declared in this header and defined in DashboardModels.cpp
// because they need the generated `app-window.h` struct definitions.

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <slint/shared_string.h>

#include "app-window.h"

namespace cpp_slint_boilerplate::models {

/// One KPI tile on the dashboard.
struct StatCard
{
    std::string title;
    std::string value;
    std::string delta;
    bool trendPositive = true;
    std::string iconName;
};

/// One entry in the recent-activity list.
struct ActivityItem
{
    std::string actor;
    std::string action;
    std::string timestamp;
    std::string iconName;
};

/// Convert a StatCard POCO into the generated `app::StatCard` struct.
app::StatCard toSlint(const StatCard &card);

/// Convert a vector of StatCards into the generated model type.
std::shared_ptr<slint::Model<app::StatCard>> toSlintModel(const std::vector<StatCard> &cards);

} // namespace cpp_slint_boilerplate::models
