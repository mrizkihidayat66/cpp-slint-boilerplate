// DashboardService.cpp — static demo data on the main branch.

#include "services/DashboardService.h"

namespace cpp_slint_boilerplate::services {

void DashboardService::initialize()
{
    // Nothing to warm up for static data.
}

void DashboardService::shutdown()
{
    // Nothing to flush.
}

std::vector<models::StatCard> DashboardService::statCards() const
{
    return {
        { "Active users", "1,248", "+12% this week", true, "users" },
        { "Revenue", "$48.2k", "+8.4% this month", true, "chart" },
        { "Open issues", "17", "-3 since yesterday", true, "warning" },
        { "Uptime", "99.98%", "30-day average", true, "check" },
    };
}

std::vector<models::ActivityItem> DashboardService::recentActivity() const
{
    return {
        { "Ada Lovelace", "created project “Analytical Engine”", "2 min ago", "folder" },
        { "Grace Hopper", "closed issue “Compiler crash on COBOL”", "18 min ago", "check" },
        { "Alan Turing", "commented on “Decryption milestone”", "1 h ago", "document" },
        { "Margaret Hamilton", "deployed release v0.4.1", "3 h ago", "refresh" },
        { "Katherine Johnson", "updated trajectory calculations", "yesterday", "chart" },
    };
}

} // namespace cpp_slint_boilerplate::services
