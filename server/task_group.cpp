#include "task_group.h"

void TaskGroup::emplace(std::future<void>&& future) {
    _tasks.emplace_back(std::move(future));
}

void TaskGroup::launch(std::function<void()> task) {
    emplace(std::async(std::launch::async, task));
};

void TaskGroup::get() {
    while (_tasks.size()) {
        auto it = std::remove_if(_tasks.begin(), _tasks.end(), [](auto& task) {
            auto status = task.wait_for(std::chrono::milliseconds(100));
            if (status != std::future_status::ready) {
                return false;
            };

            task.get();
            
            return true;
        });
        
        _tasks.erase(it, _tasks.end());
    };
}