#ifndef SERVER_TASK_GROUP_H
#define SERVER_TASK_GROUP_H 1

#include <functional>
#include <future>
#include <vector>

class TaskGroup {
public:
    void emplace(std::future<void>&& future);
    void launch(std::function<void()> task);
    void get();

private:
    std::vector<std::future<void>> _tasks;
};

#endif