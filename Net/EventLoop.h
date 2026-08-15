#pragma once

#include "EpollTaskScheduler.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

class EventLoop
{
public:
    explicit EventLoop(uint32_t num_threads = 1);
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    std::shared_ptr<TaskScheduler> GetTaskScheduler();

    void Loop();
    void Quit();

private:
    uint32_t num_threads_ = 1;
    std::atomic_uint32_t next_scheduler_{0};

    std::vector<std::shared_ptr<TaskScheduler>> schedulers_;
    std::vector<std::thread> threads_;
};