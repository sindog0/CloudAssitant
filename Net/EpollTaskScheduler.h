#pragma once

#include "TaskScheduler.h"

#include <mutex>
#include <unordered_map>
#include <vector>

#include <sys/epoll.h>

class EpollTaskScheduler : public TaskScheduler
{
public:
    explicit EpollTaskScheduler(int id = 0);
    ~EpollTaskScheduler() override;

    void UpdateChannel(Channel* channel) override;
    void RemoveChannel(Channel* channel) override;

protected:
    bool HandleEvent() override;
    void Wakeup() override;

private:
    static uint32_t ToEpollEvents(uint32_t events);
    static uint32_t FromEpollEvents(uint32_t events);

    bool UpdateEpoll(int operation, Channel* channel);
    void HandleWakeup();

private:
    int epoll_fd_ = -1;
    int wakeup_fd_ = -1;

    std::unordered_map<int, Channel*> channels_;
    std::vector<epoll_event> events_;
    std::mutex mutex_;
};