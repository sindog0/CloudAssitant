#include "Channel.h"
#include "Buffer.h"
#include "Timer.h"
#include<mutex>
#include <atomic>

//TaskScheduler 负责管理一个线程里的所有事件：IO事件 + 定时器事件，然后在事件发生时调用对应回调
class TaskScheduler
{
public:
    TaskScheduler(int id = 1) : id_(id) {}
    virtual ~TaskScheduler();
    void Start();
    void Stop();
    
    TimerId AddTimer(const TimerCallback& callback, uint32_t msec);
    void RemoveTimer(TimerId timer_id);
    virtual void UpdateChannel(Channel* channel) = 0;
    virtual void RemoveChannel(Channel* channel) = 0;
    virtual bool HandleEvent(){return false;}
    int GetId() const { return id_; }
private:
    int id_;
    std::mutex mutex_;
    TimerQueue timer_queue_;
    std::atomic<bool> running_{false};
};
