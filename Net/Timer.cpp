#include "Timer.h"

TimerId TimerQueue::AddTimer(const TimerCallback& callback, uint32_t msec)
{
    uint64_t expiration = GetTimeNow() + msec;
    auto timer = std::make_shared<Timer>(callback, msec);
    timer->SetExpiration(expiration);
    timers_[++last_timer_id_] = timer;
    events_[{expiration, last_timer_id_}] = timer;
    return last_timer_id_;// 返回新添加的定时器ID
}

void TimerQueue::RemoveTimer(TimerId timer_id)
{
    auto it = timers_.find(timer_id);
    if (it != timers_.end())
    {
        auto timer = it->second;
        events_.erase({timer->GetExpiration(), timer_id});
        timers_.erase(it);
    }
}

void TimerQueue::HandleTimerEvents()
{
    uint64_t now = GetTimeNow();
    while(!events_.empty())
    {
        auto it = events_.begin();
        auto timer = it->second;
        if(timer->GetExpiration() > now)
        {
            break;
        }
        TimerId id = it->first.second;
        events_.erase(it);

        bool repeat = timer->callback_();
        if(repeat)
        {
            timer->SetExpiration(
                now + timer->interval_
            );

            events_.emplace(
                std::make_pair(
                    timer->GetExpiration(),
                    id),
                timer);
        }
        else
        {
            timers_.erase(id);
        }
    }
}

uint64_t TimerQueue::GetTimeNow() const
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());// 获取自纪元以来的毫秒数
    return duration.count();// 返回当前时间的毫秒数
}