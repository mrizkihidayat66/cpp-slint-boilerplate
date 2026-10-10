// TaskValidationService.h — field-level validation rules (advanced branch).

#pragma once

#include "models/TaskModels.h"
#include "services/IService.h"

namespace cpp_slint_boilerplate::services {

/// Applies the business rules a task draft must satisfy before it can be
/// stored. Stateless — one instance can be shared.
class TaskValidationService : public IService {
public:
    /// Minimum title length.
    static constexpr int kMinTitleLength = 3;
    /// Maximum title length.
    static constexpr int kMaxTitleLength = 64;
    /// Maximum description length.
    static constexpr int kMaxDescriptionLength = 280;

    void initialize() override;
    void shutdown() override;

    /// Validate a draft. Returns per-field error messages; an empty result
    /// means the draft is valid.
    [[nodiscard]] models::ValidationResult validate(const models::Task &draft) const;

    /// True when `title` is a duplicate of another task's title (repository
    /// contents are provided by the caller to keep this layer stateless).
    [[nodiscard]] static bool isDuplicateTitle(const std::string &title,
                                               const std::vector<models::Task> &tasks,
                                               std::optional<int> ignoreId);
};

} // namespace cpp_slint_boilerplate::services
