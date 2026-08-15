#include "EventLoop.h"

#include <algorithm>

EventLoop::EventLoop(uint32_t num_threads)
    : num_threads_(std::max<uint32_t>(1, num_threads))
{
    Loop();
}

EventLoop::~EventLoop()
{
    Quit();
}

void EventLoop::Loop()
{
    if (!schedulers_.empty()) {
        return;
    }

    schedulers_.reserve(num_threads_);
    threads_.reserve(num_threads_);

    for (uint32_t index = 0;
         index < num_threads_;
         ++index) {
        auto scheduler =
            std::make_shared<EpollTaskScheduler>(
                static_cast<int>(index));

        schedulers_.push_back(scheduler);

        threads_.emplace_back([scheduler]() {
            scheduler->Start();
        });
    }
}

void EventLoop::Quit()
{
    for (const auto& scheduler : schedulers_) {
        scheduler->Stop();
    }

    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    threads_.clear();
    schedulers_.clear();
}

std::shared_ptr<TaskScheduler>
EventLoop::GetTaskScheduler()
{
    if (schedulers_.empty()) {
        return nullptr;
    }

    uint32_t index =
        next_scheduler_.fetch_add(1) %
        static_cast<uint32_t>(schedulers_.size());

    return schedulers_[index];
}