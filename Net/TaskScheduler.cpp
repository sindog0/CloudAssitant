#include "TaskScheduler.h"

TaskScheduler::~TaskScheduler() = default;

void TaskScheduler::Start()
{
    running_ = true;
    while(running_)
    {
        // 处理IO事件和定时器事件
        HandleEvent();
        timer_queue_.HandleTimerEvents();
    }
}

void TaskScheduler::Stop()
{
    running_ = false;
}

TimerId TaskScheduler::AddTimer(const TimerCallback& callback, uint32_t msec)
{
    //在一个线程里执行，不需要加锁，因为这个函数只会在一个线程里被调用
    return timer_queue_.AddTimer(callback, msec);
}

void TaskScheduler::RemoveTimer(TimerId timer_id)
{
    //在一个线程里执行，不需要加锁，因为这个函数只会在一个线程里被调用
    timer_queue_.RemoveTimer(timer_id);
}
