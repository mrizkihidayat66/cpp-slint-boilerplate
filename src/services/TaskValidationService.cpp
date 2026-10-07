// TaskValidationService.cpp — see TaskValidationService.h.

#include "services/TaskValidationService.h"

#include <cctype>

namespace cpp_slint_boilerplate::services {

void TaskValidationService::initialize()
{
}

void TaskValidationService::shutdown()
{
}

namespace {

std::string trimmed(const std::string &text)
{
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end
           && std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    while (end > begin
           && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }
    return text.substr(begin, end - begin);
}

} // namespace

models::ValidationResult TaskValidationService::validate(const models::Task &draft) const
{
    models::ValidationResult result;

    const std::string title = trimmed(draft.title);
    if (title.empty()) {
        result.errors.emplace_back("title", "Title is required");
    } else if (title.size() < kMinTitleLength) {
        result.errors.emplace_back(
                "title", "Title must be at least " + std::to_string(kMinTitleLength)
                             + " characters");
    } else if (title.size() > kMaxTitleLength) {
        result.errors.emplace_back(
                "title", "Title must be at most " + std::to_string(kMaxTitleLength)
                             + " characters");
    }

    if (draft.description.size() > kMaxDescriptionLength) {
        result.errors.emplace_back(
                "description", "Description must be at most "
                                   + std::to_string(kMaxDescriptionLength)
                                   + " characters");
    }

    const std::string assignee = trimmed(draft.assignee);
    if (assignee.empty()) {
        result.errors.emplace_back("assignee", "Assignee is required");
    } else if (assignee.size() > 32) {
        result.errors.emplace_back("assignee", "Assignee must be at most 32 characters");
    }

    return result;
}

bool TaskValidationService::isDuplicateTitle(const std::string &title,
                                             const std::vector<models::Task> &tasks,
                                             std::optional<int> ignoreId)
{
    for (const auto &task : tasks) {
        if (ignoreId && task.id == *ignoreId) {
            continue;
        }
        if (task.title == title) {
            return true;
        }
    }
    return false;
}

} // namespace cpp_slint_boilerplate::services
