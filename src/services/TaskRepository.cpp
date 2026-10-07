// TaskRepository.cpp — see TaskRepository.h.

#include "services/TaskRepository.h"

#include <algorithm>

namespace cpp_slint_boilerplate::services {

void TaskRepository::initialize()
{
    if (!m_tasks.empty()) {
        return; // already seeded (or test pre-populated)
    }
    seed();
}

void TaskRepository::shutdown()
{
}

void TaskRepository::seed()
{
    // Demo set: enough rows to make sorting and pagination observable
    // (3 pages at 10 rows per page) with varied titles, assignees, statuses.
    struct Seed
    {
        const char *title;
        const char *assignee;
        models::TaskStatus status;
    };

    static const Seed seeds[] = {
        { "Set up CI pipeline", "M Rizki", models::TaskStatus::Done },
        { "Write onboarding docs", "Ada Lovelace", models::TaskStatus::Done },
        { "Implement login form", "Grace Hopper", models::TaskStatus::Done },
        { "Migrate settings to JSON", "M Rizki", models::TaskStatus::Done },
        { "Design settings page", "Grace Hopper", models::TaskStatus::Done },
        { "Add component gallery", "Ada Lovelace", models::TaskStatus::Done },
        { "Dark theme tokens", "M Rizki", models::TaskStatus::InProgress },
        { "Sortable task table", "Grace Hopper", models::TaskStatus::InProgress },
        { "Pagination controls", "Ada Lovelace", models::TaskStatus::InProgress },
        { "Task dialog validation", "M Rizki", models::TaskStatus::InProgress },
        { "Delete confirmations", "Grace Hopper", models::TaskStatus::Todo },
        { "Loading placeholders", "Ada Lovelace", models::TaskStatus::Todo },
        { "Keyboard shortcuts", "M Rizki", models::TaskStatus::Todo },
        { "Export tasks to CSV", "Grace Hopper", models::TaskStatus::Todo },
        { "Filter by assignee", "Ada Lovelace", models::TaskStatus::Todo },
        { "Bulk status updates", "M Rizki", models::TaskStatus::Todo },
        { "Search across tasks", "Grace Hopper", models::TaskStatus::Todo },
        { "Archive old tasks", "Ada Lovelace", models::TaskStatus::Todo },
        { "Localize to Indonesian", "M Rizki", models::TaskStatus::Todo },
        { "Polish empty states", "Grace Hopper", models::TaskStatus::Todo },
        { "Write example branches", "Ada Lovelace", models::TaskStatus::Todo },
        { "Record demo screencast", "M Rizki", models::TaskStatus::Todo },
        { "Ship v1 release", "M Rizki", models::TaskStatus::Todo },
    };

    m_tasks.clear();
    m_tasks.reserve(std::size(seeds));
    for (const auto &s : seeds) {
        models::Task task;
        task.id = m_nextId++;
        task.title = s.title;
        task.description = "Seeded demo task";
        task.assignee = s.assignee;
        task.status = s.status;
        m_tasks.push_back(std::move(task));
    }
}

const std::vector<models::Task> &TaskRepository::all() const
{
    return m_tasks;
}

std::optional<models::Task> TaskRepository::find(int id) const
{
    for (const auto &task : m_tasks) {
        if (task.id == id) {
            return task;
        }
    }
    return std::nullopt;
}

models::Task TaskRepository::insert(models::Task task)
{
    task.id = m_nextId++;
    m_tasks.push_back(std::move(task));
    return m_tasks.back();
}

bool TaskRepository::update(const models::Task &task)
{
    for (auto &existing : m_tasks) {
        if (existing.id == task.id) {
            existing = task;
            return true;
        }
    }
    return false;
}

bool TaskRepository::remove(int id)
{
    const auto it = std::find_if(m_tasks.begin(), m_tasks.end(),
                                 [id](const models::Task &t) { return t.id == id; });
    if (it == m_tasks.end()) {
        return false;
    }
    m_tasks.erase(it);
    return true;
}

std::size_t TaskRepository::count() const
{
    return m_tasks.size();
}

void TaskRepository::clear()
{
    m_tasks.clear();
    m_nextId = 1;
}

} // namespace cpp_slint_boilerplate::services
