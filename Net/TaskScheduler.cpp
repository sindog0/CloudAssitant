#include "TaskScheduler.h"

void TaskScheduler::Start()
{
    bool expected = false;

    if (!running_.compare_exchange_strong(expected, true)) {
        return;
    }

    while (running_.load()) {
        HandleEvent();
        timer_queue_.HandleTimerEvents();
    }
}

void TaskScheduler::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }

    // 唤醒 epoll_wait，让线程立即退出。
    Wakeup();
}

TimerId TaskScheduler::AddTimer(
    const TimerCallback& callback,
    uint32_t msec)
{
    TimerId id = timer_queue_.AddTimer(callback, msec);
    Wakeup();
    return id;
}

void TaskScheduler::RemoveTimer(TimerId timer_id)
{
    timer_queue_.RemoveTimer(timer_id);
    Wakeup();
}