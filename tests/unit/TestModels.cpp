// TestModels.cpp — POCO → Slint adapter behavior.

#include <catch2/catch_test_macros.hpp>

#include "models/DashboardModels.h"

using cpp_slint_boilerplate::models::StatCard;

TEST_CASE("toSlint copies all StatCard fields", "[models]")
{
    const StatCard input{
        .title = "Active users",
        .value = "1,248",
        .delta = "+12%",
        .trendPositive = true,
        .iconName = "users",
    };

    const auto converted = cpp_slint_boilerplate::models::toSlint(input);

    CHECK(converted.title == input.title);
    CHECK(converted.value == input.value);
    CHECK(converted.delta == input.delta);
    CHECK(converted.trend_positive == input.trendPositive);
    CHECK(converted.icon_name == input.iconName);
}

TEST_CASE("toSlint keeps trend direction distinct", "[models]")
{
    const StatCard up{ .title = "a", .value = "1", .delta = "+1", .trendPositive = true,
                       .iconName = "chart" };
    const StatCard down{ .title = "a", .value = "1", .delta = "-1", .trendPositive = false,
                         .iconName = "chart" };

    CHECK(cpp_slint_boilerplate::models::toSlint(up).trend_positive);
    CHECK_FALSE(cpp_slint_boilerplate::models::toSlint(down).trend_positive);
}

TEST_CASE("toSlintModel preserves order and count", "[models]")
{
    const std::vector<StatCard> cards{
        { "one", "1", "+", true, "check" },
        { "two", "2", "-", false, "warning" },
        { "three", "3", "+", true, "chart" },
    };

    const auto model = cpp_slint_boilerplate::models::toSlintModel(cards);

    REQUIRE(model->row_count() == cards.size());
    for (std::size_t i = 0; i < cards.size(); ++i) {
        CHECK(model->row_data(int(i))->title == cards[i].title);
    }
}
