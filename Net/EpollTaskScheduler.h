#pragma once
#include "TaskScheduler.h"
#include <sys/epoll.h>
#include <unordered_map>
#include <mutex>
#include <vector>
class EpollTaskScheduler : public TaskScheduler
{
public:
    EpollTaskScheduler(int id = 0) : TaskScheduler(id) {epoll_fd_ = epoll_create1(0);}
    ~EpollTaskScheduler() override;

private:
    void UpdateChannel(Channel *channel) override;
    void RemoveChannel(Channel *channel) override;
    bool HandleEvent() override;
private:
    void UpdateEpoll(int operation, Channel *channel);// 更新epoll事件
private:
    int epoll_fd_ = -1;
    std::unordered_map<int, Channel *> channels_; // 维护一个fd到Channel的映射
    std::vector<struct epoll_event> events_; // 用于存储epoll_wait返回的事件
    std::mutex mutex_; // 保护channels_和events_的线程安全
};
