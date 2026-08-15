#include "EpollTaskScheduler.h"
#include <vector>

class EventLoop
{
public:
    EventLoop(uint32_t num_threads = -1);
    ~EventLoop();
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    std::shared_ptr<TaskScheduler> GetTaskScheduler();
    TimerId AddTimer(const TimerCallback &callback, uint32_t msec);
    void RemoveTimer(TimerId timer_id);
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);

    void Loop();
    void Quit();
private:
    uint32_t num_threads_ = 1;
    uint32_t index_ = 0; // 用于轮询选择TaskScheduler
    std::vector<std::shared_ptr<TaskScheduler>> schedulers_;// 线程池中的每个线程都有一个TaskScheduler对象
    std::vector<std::shared_ptr<std::thread>> threads_;// 线程池
};
