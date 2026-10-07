// TestTasks.cpp — the task chain (repository → validation → service).

#include <catch2/catch_test_macros.hpp>

#include "services/TaskRepository.h"
#include "services/TaskService.h"
#include "services/TaskValidationService.h"

using namespace cpp_slint_boilerplate;
using models::Task;
using models::TaskSortColumn;
using models::TaskSortDirection;
using models::TaskStatus;

namespace {

/// Fresh chain wired the same way Application wires it.
struct TaskChain
{
    std::shared_ptr<services::TaskRepository> repository =
            std::make_shared<services::TaskRepository>();
    std::shared_ptr<services::TaskValidationService> validation =
            std::make_shared<services::TaskValidationService>();
    std::shared_ptr<services::TaskService> service;

    TaskChain()
        : service(std::make_shared<services::TaskService>(repository, validation))
    {
        repository->initialize();
        validation->initialize();
        service->initialize();
    }
};

Task draft(std::string title, std::string assignee)
{
    Task task;
    task.title = std::move(title);
    task.assignee = std::move(assignee);
    return task;
}

} // namespace

TEST_CASE("TaskRepository seeds, inserts, updates, removes", "[tasks]")
{
    services::TaskRepository repo;
    repo.initialize();

    CHECK(repo.count() >= 20);

    const int before = static_cast<int>(repo.count());
    const Task inserted = repo.insert(draft("Write tests", "M Rizki"));
    CHECK(inserted.id > 0);
    CHECK(repo.count() == before + 1);
    CHECK(repo.find(inserted.id).has_value());

    Task edited = inserted;
    edited.title = "Write more tests";
    CHECK(repo.update(edited));
    CHECK(repo.find(edited.id)->title == "Write more tests");

    CHECK(repo.remove(edited.id));
    CHECK_FALSE(repo.find(edited.id).has_value());
    CHECK_FALSE(repo.remove(edited.id)); // double delete
}

TEST_CASE("TaskValidationService rejects empty and short fields", "[tasks]")
{
    services::TaskValidationService validation;
    validation.initialize();

    Task empty;
    const auto result = validation.validate(empty);
    REQUIRE_FALSE(result.ok());
    CHECK(result.messageFor("title") == "Title is required");
    CHECK(result.messageFor("assignee") == "Assignee is required");

    Task tooShort = draft("ab", "M Rizki");
    CHECK(validation.validate(tooShort).messageFor("title")
          == "Title must be at least 3 characters");

    Task trimmed = draft("   ok title   ", "M Rizki");
    CHECK(validation.validate(trimmed).ok()); // leading/trailing spaces trimmed
}

TEST_CASE("TaskService create rejects invalid drafts and duplicates", "[tasks]")
{
    TaskChain chain;

    // Invalid: no assignee.
    const auto invalid = chain.service->create(draft("Valid title", ""));
    CHECK_FALSE(invalid.success);
    CHECK(invalid.errors.messageFor("assignee") == "Assignee is required");

    // Duplicate of a seeded title.
    const auto duplicate = chain.service->create(draft("Set up CI pipeline", "M Rizki"));
    CHECK_FALSE(duplicate.success);
    CHECK(duplicate.errorMessage == "A task with this title already exists");

    // Valid create.
    const auto created = chain.service->create(draft("Brand new task", "M Rizki"));
    REQUIRE(created.success);
    CHECK(created.task.id > 0);
    CHECK(chain.service->find(created.task.id).has_value());
}

TEST_CASE("TaskService update validates and keeps ids stable", "[tasks]")
{
    TaskChain chain;

    const auto created = chain.service->create(draft("Original title", "Ada Lovelace"));
    REQUIRE(created.success);

    Task edited = created.task;
    edited.title = "x"; // too short
    const auto invalid = chain.service->update(edited);
    CHECK_FALSE(invalid.success);
    CHECK_FALSE(invalid.errors.messageFor("title").empty());

    edited.title = "Edited title";
    const auto updated = chain.service->update(edited);
    REQUIRE(updated.success);
    CHECK(updated.task.id == created.task.id);
    CHECK(chain.service->find(created.task.id)->title == "Edited title");
}

TEST_CASE("TaskService sorts by title, assignee, and status", "[tasks]")
{
    TaskChain chain;

    const auto byTitle = chain.service->sorted(TaskSortColumn::Title,
                                               TaskSortDirection::Ascending);
    REQUIRE(byTitle.size() >= 2);
    for (std::size_t i = 1; i < byTitle.size(); ++i) {
        CHECK(byTitle[i - 1].title <= byTitle[i].title);
    }

    const auto byAssignee = chain.service->sorted(TaskSortColumn::Assignee,
                                                  TaskSortDirection::Descending);
    for (std::size_t i = 1; i < byAssignee.size(); ++i) {
        CHECK(byAssignee[i - 1].assignee >= byAssignee[i].assignee);
    }

    const auto byStatus = chain.service->sorted(TaskSortColumn::Status,
                                                TaskSortDirection::Ascending);
    for (std::size_t i = 1; i < byStatus.size(); ++i) {
        CHECK(static_cast<int>(byStatus[i - 1].status) <= static_cast<int>(byStatus[i].status));
    }
}

TEST_CASE("TaskService paginates and clamps out-of-range pages", "[tasks]")
{
    TaskChain chain;

    const int total = chain.service->count();
    REQUIRE(total > services::TaskService::kPageSize); // pagination is observable

    const auto first = chain.service->page(TaskSortColumn::Id,
                                           TaskSortDirection::Ascending, 1);
    CHECK(first.pageNumber == 1);
    CHECK(static_cast<int>(first.tasks.size()) == services::TaskService::kPageSize);
    CHECK(first.totalCount == total);
    CHECK(first.pageCount == (total + services::TaskService::kPageSize - 1)
                              / services::TaskService::kPageSize);

    const auto beyond = chain.service->page(TaskSortColumn::Id,
                                            TaskSortDirection::Ascending, 999);
    CHECK(beyond.pageNumber == beyond.pageCount); // clamped to last page
    CHECK(static_cast<int>(beyond.tasks.size()) <= services::TaskService::kPageSize);
}

TEST_CASE("Deleting tasks shrinks pages and keeps sort order", "[tasks]")
{
    TaskChain chain;

    const auto page1 = chain.service->page(TaskSortColumn::Title,
                                           TaskSortDirection::Ascending, 1);
    REQUIRE_FALSE(page1.tasks.empty());

    // Delete every task on the first page.
    for (const auto &task : page1.tasks) {
        REQUIRE(chain.service->remove(task.id));
    }

    const auto after = chain.service->page(TaskSortColumn::Title,
                                           TaskSortDirection::Ascending, 1);
    CHECK(after.totalCount == page1.totalCount - static_cast<int>(page1.tasks.size()));
    // Still sorted after deletion.
    for (std::size_t i = 1; i < after.tasks.size(); ++i) {
        CHECK(after.tasks[i - 1].title <= after.tasks[i].title);
    }
}

TEST_CASE("Status labels round-trip", "[tasks]")
{
    CHECK(models::taskStatusLabel(models::TaskStatus::Todo) == "To do");
    CHECK(models::taskStatusFromLabel("In progress") == models::TaskStatus::InProgress);
    CHECK(models::taskStatusFromLabel("Nope") == std::nullopt);
}
