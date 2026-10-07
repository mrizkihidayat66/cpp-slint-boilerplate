// TaskRepository.h — in-memory task storage (advanced branch).

#pragma once

#include <optional>
#include <vector>

#include "models/TaskModels.h"
#include "services/IService.h"

namespace cpp_slint_boilerplate::services {

/// Owns the task data. The advanced branch keeps it in memory with a seeded
/// demo set; a real app would swap the implementation for a database while
/// keeping the interface.
class TaskRepository : public IService
{
public:
    void initialize() override;
    void shutdown() override;

    /// All tasks, in insertion (id) order.
    [[nodiscard]] const std::vector<models::Task> &all() const;

    /// Task by id, or nullopt.
    [[nodiscard]] std::optional<models::Task> find(int id) const;

    /// Insert with the next free id; returns the stored task.
    models::Task insert(models::Task task);

    /// Replace the task with the same id; false when it does not exist.
    bool update(const models::Task &task);

    /// Remove by id; false when it does not exist.
    bool remove(int id);

    /// Number of stored tasks.
    [[nodiscard]] std::size_t count() const;

    /// Drop everything (tests).
    void clear();

private:
    /// Populate the demo data set (idempotent guard in initialize()).
    void seed();

    std::vector<models::Task> m_tasks;
    int m_nextId = 1;
};

} // namespace cpp_slint_boilerplate::services
