// TaskModels.h — pure C++ data for the tasks demo (advanced branch).
//
// POCOs only: no Slint types beyond the adapter below, so the repository,
// validation, and service layers stay testable without a UI backend.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <slint.h>

#include "app-window.h"

namespace cpp_slint_boilerplate::models {

/// Task lifecycle. Ordering matches the UI badge mapping.
enum class TaskStatus
{
    Todo,
    InProgress,
    Done,
};

/// One row of the tasks table.
struct Task
{
    int id = 0;
    std::string title;
    std::string description;
    std::string assignee;
    TaskStatus status = TaskStatus::Todo;

    [[nodiscard]] std::string statusLabel() const;
};

/// Sort keys — mirror the table columns (id = created order).
enum class TaskSortColumn
{
    Id,
    Title,
    Assignee,
    Status,
};

/// Direction toggle for the active sort.
enum class TaskSortDirection
{
    Ascending,
    Descending,
};

/// Field errors keyed by name ("title", "assignee"). Empty map = valid.
struct ValidationResult
{
    std::vector<std::pair<std::string, std::string>> errors;

    [[nodiscard]] bool ok() const { return errors.empty(); }

    /// First message stored under `field`, or "".
    [[nodiscard]] std::string messageFor(const std::string &field) const
    {
        for (const auto &[name, message] : errors) {
            if (name == field) {
                return message;
            }
        }
        return {};
    }
};

/// One page of tasks plus the totals the pagination bar needs.
struct TaskPage
{
    std::vector<Task> tasks;
    int totalCount = 0;
    int pageCount = 1;
    int pageNumber = 1;
};

/// Display label for a status ("To do", "In progress", "Done").
std::string taskStatusLabel(TaskStatus status);

/// Parse a status label back to the enum; nullopt when unknown.
std::optional<TaskStatus> taskStatusFromLabel(const std::string &label);

/// Convert a Task POCO into a table row of display strings.
::app::AppTableRow taskToRow(const Task &task);

/// Convert a vector of Tasks into the table row model.
std::shared_ptr<slint::Model<::app::AppTableRow>> taskRowsToModel(const std::vector<Task> &tasks);

} // namespace cpp_slint_boilerplate::models
