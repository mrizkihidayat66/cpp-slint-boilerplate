// TaskService.h — orchestrates repository + validation (advanced branch).

#pragma once

#include <memory>

#include "models/TaskModels.h"
#include "services/IService.h"
#include "services/TaskRepository.h"
#include "services/TaskValidationService.h"

namespace cpp_slint_boilerplate::app {
class ServiceLocator;
}

namespace cpp_slint_boilerplate::services {

/// Outcome of a create/update attempt. `errors` carries per-field messages
/// the dialog shows next to the inputs; a failed attempt keeps the dialog
/// open, a successful one closes it.
struct TaskMutationResult {
    bool success = false;
    models::ValidationResult errors;
    std::string errorMessage; // non-field summary (e.g. duplicate title)
    models::Task task;        // stored task on success
};

/// Reads and mutates tasks through the repository, applying validation
/// first. Sorts and paginates on read. The chain is wired by the
/// ServiceLocator: TaskService is registered AFTER its dependencies.
class TaskService : public IService {
public:
    /// Rows per page in the tasks table.
    static constexpr int kPageSize = 10;

    /// Owns the dependencies (dependency injection by constructor).
    explicit TaskService(std::shared_ptr<TaskRepository> repository,
                         std::shared_ptr<TaskValidationService> validation);

    void initialize() override;
    void shutdown() override;

    /// One sorted, paginated page of tasks.
    [[nodiscard]] models::TaskPage page(models::TaskSortColumn column,
                                        models::TaskSortDirection direction, int pageNumber) const;

    /// All tasks in the current sort order (export, tests).
    [[nodiscard]] std::vector<models::Task> sorted(models::TaskSortColumn column,
                                                   models::TaskSortDirection direction) const;

    /// Task by id, or nullopt.
    [[nodiscard]] std::optional<models::Task> find(int id) const;

    /// Validate + insert. On success returns the stored task.
    TaskMutationResult create(const models::Task &draft);

    /// Validate + replace. `id` must exist.
    TaskMutationResult update(const models::Task &draft);

    /// Remove by id; false when it does not exist.
    bool remove(int id);

    /// Total number of tasks.
    [[nodiscard]] int count() const;

private:
    /// Shared create/update pipeline: validate, reject duplicate titles.
    TaskMutationResult persist(models::Task draft);

    std::shared_ptr<TaskRepository> m_repository;
    std::shared_ptr<TaskValidationService> m_validation;
};

} // namespace cpp_slint_boilerplate::services
