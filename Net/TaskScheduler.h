#pragma once

#include "Channel.h"
#include "Timer.h"

#include <atomic>
#include <cstdint>

class TaskScheduler
{
public:
    explicit TaskScheduler(int id = 0)
        : id_(id)
    {
    }

    virtual ~TaskScheduler() = default;

    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    void Start();
    void Stop();

    TimerId AddTimer(const TimerCallback& callback, uint32_t msec);
    void RemoveTimer(TimerId timer_id);

    virtual void UpdateChannel(Channel* channel) = 0;
    virtual void RemoveChannel(Channel* channel) = 0;

    int GetId() const
    {
        return id_;
    }

protected:
    virtual bool HandleEvent() = 0;
    virtual void Wakeup() = 0;

private:
    int id_ = 0;
    std::atomic_bool running_{false};
    TimerQueue timer_queue_;
};