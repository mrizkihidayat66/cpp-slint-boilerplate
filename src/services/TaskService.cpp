// TaskService.cpp — see TaskService.h.

#include "services/TaskService.h"

#include <algorithm>

namespace cpp_slint_boilerplate::services {

TaskService::TaskService(std::shared_ptr<TaskRepository> repository,
                         std::shared_ptr<TaskValidationService> validation)
    : m_repository(std::move(repository)), m_validation(std::move(validation))
{
}

void TaskService::initialize()
{
    // Dependencies were initialized at their own registration order; nothing
    // to do here.
}

void TaskService::shutdown() {}

models::TaskPage TaskService::page(models::TaskSortColumn column,
                                   models::TaskSortDirection direction, int pageNumber) const
{
    const std::vector<models::Task> all = sorted(column, direction);

    models::TaskPage result;
    result.totalCount = static_cast<int>(all.size());
    result.pageCount = std::max(1, (result.totalCount + kPageSize - 1) / kPageSize);
    result.pageNumber = std::clamp(pageNumber, 1, result.pageCount);

    const int begin = (result.pageNumber - 1) * kPageSize;
    const int end = std::min<int>(begin + kPageSize, result.totalCount);
    result.tasks.assign(all.begin() + begin, all.begin() + end);
    return result;
}

std::vector<models::Task> TaskService::sorted(models::TaskSortColumn column,
                                              models::TaskSortDirection direction) const
{
    std::vector<models::Task> tasks = m_repository->all();

    const bool ascending = direction == models::TaskSortDirection::Ascending;
    const auto compare = [column, ascending](const models::Task &a, const models::Task &b) {
        int cmp = 0;
        switch (column) {
        case models::TaskSortColumn::Id:
            cmp = a.id - b.id;
            break;
        case models::TaskSortColumn::Title:
            cmp = a.title.compare(b.title);
            break;
        case models::TaskSortColumn::Assignee:
            cmp = a.assignee.compare(b.assignee);
            break;
        case models::TaskSortColumn::Status:
            // Enum order matches the lifecycle order (Todo < InProgress < Done).
            cmp = static_cast<int>(a.status) - static_cast<int>(b.status);
            break;
        }
        return ascending ? cmp < 0 : cmp > 0;
    };
    std::stable_sort(tasks.begin(), tasks.end(), compare);
    return tasks;
}

std::optional<models::Task> TaskService::find(int id) const
{
    return m_repository->find(id);
}

TaskMutationResult TaskService::create(const models::Task &draft)
{
    TaskMutationResult result = persist(draft);
    if (result.success) {
        result.task = m_repository->insert(draft);
    }
    return result;
}

TaskMutationResult TaskService::update(const models::Task &draft)
{
    if (!m_repository->find(draft.id)) {
        TaskMutationResult result;
        result.errorMessage = "Task " + std::to_string(draft.id) + " no longer exists";
        return result;
    }

    TaskMutationResult result = persist(draft);
    if (result.success) {
        m_repository->update(draft);
        result.task = draft;
    }
    return result;
}

bool TaskService::remove(int id)
{
    return m_repository->remove(id);
}

int TaskService::count() const
{
    return static_cast<int>(m_repository->count());
}

TaskMutationResult TaskService::persist(models::Task draft)
{
    TaskMutationResult result;

    result.errors = m_validation->validate(draft);
    if (!result.errors.ok()) {
        return result;
    }

    // Duplicate titles are a cross-record rule — needs the repository's
    // contents, so it runs here rather than in the stateless validator.
    if (TaskValidationService::isDuplicateTitle(draft.title, m_repository->all(),
                                                draft.id > 0 ? std::optional<int>(draft.id)
                                                             : std::nullopt)) {
        result.errorMessage = "A task with this title already exists";
        return result;
    }

    result.success = true;
    return result;
}

} // namespace cpp_slint_boilerplate::services
