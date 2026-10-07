// DashboardModels.cpp — POCO → generated-struct adapters.

#include "models/DashboardModels.h"

#include <cctype>

namespace cpp_slint_boilerplate::models {

std::string initialsOf(const std::string &name)
{
    std::string initials;
    bool takeNext = true;
    int taken = 0;
    for (char ch : name) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!initials.empty()) {
                takeNext = true; // a second word begins
            }
            continue;
        }
        if (takeNext && std::isalpha(static_cast<unsigned char>(ch))) {
            initials += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            ++taken;
            takeNext = false;
            if (taken >= 2) {
                break;
            }
        }
    }
    return initials;
}

::app::StatCard toSlint(const StatCard &card)
{
    ::app::StatCard result;
    result.title = slint::SharedString(card.title);
    result.value = slint::SharedString(card.value);
    result.delta = slint::SharedString(card.delta);
    result.trend_positive = card.trendPositive;
    result.icon_name = slint::SharedString(card.iconName);
    return result;
}

std::shared_ptr<slint::Model<::app::StatCard>> toSlintModel(const std::vector<StatCard> &cards)
{
    std::vector<::app::StatCard> converted;
    converted.reserve(cards.size());
    for (const auto &card : cards) {
        converted.push_back(toSlint(card));
    }
    return std::make_shared<slint::VectorModel<::app::StatCard>>(std::move(converted));
}

} // namespace cpp_slint_boilerplate::models
