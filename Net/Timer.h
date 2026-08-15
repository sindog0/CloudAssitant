#pragma once
#include <functional>
#include <cstdint>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <map>

using TimerCallback = std::function<bool(void)>;
using TimerId = uint32_t;

class Timer
{
public:
    Timer(const TimerCallback &callback, uint32_t msec) : callback_(callback), interval_(msec) {}
    ~Timer() = default;

    static void Sleep(uint32_t msec)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(msec)); // 让当前线程休眠指定的毫秒数
    }

private:
    void SetExpiration(uint64_t expiration)
    {
        expiration_ = expiration;
    }

    uint64_t GetExpiration() const
    {
        return expiration_;
    }

private:
    TimerCallback callback_;
    uint32_t interval_ = 0;   // 定时器的间隔时间，单位为毫秒
    uint64_t expiration_ = 0; // 定时器的到期时间，单位为毫秒

    friend class TimerQueue;
};

class TimerQueue
{
public:
    TimerQueue() = default;
    ~TimerQueue() = default;

    TimerId AddTimer(const TimerCallback &callback, uint32_t msec);
    void RemoveTimer(TimerId timer_id);

    void HandleTimerEvents(); // 处理定时器事件，检查是否有定时器到期，并执行相应的回调函数

protected:
    uint64_t GetTimeNow() const;

private:
    uint32_t last_timer_id_ = 0; // 用于生成唯一的定时器ID
    // 作用是为了存储所有的定时器对象，键为定时器ID，值为定时器对象的智能指针
    std::unordered_map<TimerId, std::shared_ptr<Timer>> timers_; 
    //作用是为了按照定时器的到期时间进行排序，方便快速找到最先到期的定时器。键为一个包含定时器到期时间和定时器ID的pair，值为定时器对象的智能指针
    std::map<std::pair<int64_t,TimerId>,std::shared_ptr<Timer>> events_;
};
