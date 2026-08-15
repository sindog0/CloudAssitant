#include "EpollTaskScheduler.h"
#include <cstdio>
#include <unistd.h>

EpollTaskScheduler::~EpollTaskScheduler()
{
    if(epoll_fd_ >= 0){
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

void EpollTaskScheduler::UpdateChannel(Channel *channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int fd = channel->GetSocket();
    auto it = channels_.find(fd);    
    if(it != channels_.end())
    {
        UpdateEpoll(EPOLL_CTL_MOD, channel);
    }
    else
    {
        UpdateEpoll(EPOLL_CTL_ADD, channel);
        channels_.emplace(fd, channel);
    }
}

void EpollTaskScheduler::RemoveChannel(Channel *channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int fd = channel->GetSocket();
    auto it = channels_.find(fd);
    if(it != channels_.end())
    {
        UpdateEpoll(EPOLL_CTL_DEL, channel);
        channels_.erase(it);
    }
}

void EpollTaskScheduler::UpdateEpoll(int operation, Channel *channel)
{
    struct epoll_event event;
    event.data.fd = channel->GetSocket();
    event.events = channel->GetEvents();
    if(epoll_ctl(epoll_fd_, operation, channel->GetSocket(), &event) < 0)
    {
        std::perror("epoll_ctl error");
    }
}

bool EpollTaskScheduler::HandleEvent()
{
    int num_events = epoll_wait(epoll_fd_, events_.data(), events_.size(),1000);
    if(num_events < 0)
    {
        std::perror("epoll_wait error");
        return false;
    }
    for(int i = 0;i < num_events;++i)
    {
        int fd = events_[i].data.fd;
        auto it = channels_.find(fd);
        if(it != channels_.end())
        {
            Channel *channel = it->second;
            channel->HandleEvent(events_[i].events);
        }
    }
    return true;
}
