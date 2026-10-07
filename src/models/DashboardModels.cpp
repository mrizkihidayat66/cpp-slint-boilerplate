// DashboardModels.cpp — POCO → generated-struct adapters.

#include "models/DashboardModels.h"

#include <slint/vec_model.h>

namespace cpp_slint_boilerplate::models {

app::StatCard toSlint(const StatCard &card)
{
    app::StatCard result;
    result.title = slint::SharedString(card.title);
    result.value = slint::SharedString(card.value);
    result.delta = slint::SharedString(card.delta);
    result.trend_positive = card.trendPositive;
    result.icon_name = slint::SharedString(card.iconName);
    return result;
}

std::shared_ptr<slint::Model<app::StatCard>> toSlintModel(const std::vector<StatCard> &cards)
{
    std::vector<app::StatCard> converted;
    converted.reserve(cards.size());
    for (const auto &card : cards) {
        converted.push_back(toSlint(card));
    }
    return std::make_shared<slint::VecModel<app::StatCard>>(std::move(converted));
}

} // namespace cpp_slint_boilerplate::models
