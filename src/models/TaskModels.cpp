// TaskModels.cpp — status labels + POCO → table-row adapters.

#include "models/TaskModels.h"

namespace cpp_slint_boilerplate::models {

std::string taskStatusLabel(TaskStatus status)
{
    switch (status) {
    case TaskStatus::Todo:
        return "To do";
    case TaskStatus::InProgress:
        return "In progress";
    case TaskStatus::Done:
        return "Done";
    }
    return "To do";
}

std::optional<TaskStatus> taskStatusFromLabel(const std::string &label)
{
    if (label == "To do") {
        return TaskStatus::Todo;
    }
    if (label == "In progress") {
        return TaskStatus::InProgress;
    }
    if (label == "Done") {
        return TaskStatus::Done;
    }
    return std::nullopt;
}

std::string Task::statusLabel() const
{
    return taskStatusLabel(status);
}

::app::AppTableRow taskToRow(const Task &task)
{
    std::vector<slint::SharedString> cells;
    cells.reserve(4);
    cells.push_back(slint::SharedString(std::to_string(task.id)));
    cells.push_back(slint::SharedString(task.title));
    cells.push_back(slint::SharedString(task.assignee));
    cells.push_back(slint::SharedString(task.statusLabel()));

    ::app::AppTableRow row;
    row.cells = std::make_shared<slint::VectorModel<slint::SharedString>>(std::move(cells));
    return row;
}

std::shared_ptr<slint::Model<::app::AppTableRow>> taskRowsToModel(const std::vector<Task> &tasks)
{
    std::vector<::app::AppTableRow> rows;
    rows.reserve(tasks.size());
    for (const auto &task : tasks) {
        rows.push_back(taskToRow(task));
    }
    return std::make_shared<slint::VectorModel<::app::AppTableRow>>(std::move(rows));
}

} // namespace cpp_slint_boilerplate::models
