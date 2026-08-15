#include "EpollTaskScheduler.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#include <sys/eventfd.h>
#include <unistd.h>

EpollTaskScheduler::EpollTaskScheduler(int id)
    : TaskScheduler(id),
      events_(64)
{
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("epoll_create1 failed");
    }

    wakeup_fd_ = eventfd(
        0,
        EFD_NONBLOCK | EFD_CLOEXEC);

    if (wakeup_fd_ < 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
        throw std::runtime_error("eventfd failed");
    }

    epoll_event event{};
    event.data.fd = wakeup_fd_;
    event.events = EPOLLIN;

    if (epoll_ctl(
            epoll_fd_,
            EPOLL_CTL_ADD,
            wakeup_fd_,
            &event) < 0) {
        close(wakeup_fd_);
        close(epoll_fd_);
        wakeup_fd_ = -1;
        epoll_fd_ = -1;
        throw std::runtime_error("add wakeup fd failed");
    }
}

EpollTaskScheduler::~EpollTaskScheduler()
{
    if (wakeup_fd_ >= 0) {
        close(wakeup_fd_);
        wakeup_fd_ = -1;
    }

    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

uint32_t EpollTaskScheduler::ToEpollEvents(uint32_t events)
{
    uint32_t result = EPOLLRDHUP;

    if (events & Channel::ReadEvent) {
        result |= EPOLLIN | EPOLLPRI;
    }

    if (events & Channel::WriteEvent) {
        result |= EPOLLOUT;
    }

    return result;
}

uint32_t EpollTaskScheduler::FromEpollEvents(uint32_t events)
{
    uint32_t result = Channel::NoneEvent;

    if (events & (EPOLLIN | EPOLLPRI)) {
        result |= Channel::ReadEvent;
    }

    if (events & EPOLLOUT) {
        result |= Channel::WriteEvent;
    }

    if (events & EPOLLERR) {
        result |= Channel::ErrorEvent;
    }

    if (events & (EPOLLHUP | EPOLLRDHUP)) {
        result |= Channel::CloseEvent;
    }

    return result;
}

bool EpollTaskScheduler::UpdateEpoll(
    int operation,
    Channel* channel)
{
    epoll_event event{};
    event.data.fd = channel->GetSocket();
    event.events = ToEpollEvents(channel->GetEvents());

    if (epoll_ctl(
            epoll_fd_,
            operation,
            channel->GetSocket(),
            &event) < 0) {
        std::perror("epoll_ctl");
        return false;
    }

    return true;
}

void EpollTaskScheduler::UpdateChannel(Channel* channel)
{
    if (!channel) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    int socket = channel->GetSocket();
    auto iterator = channels_.find(socket);

    if (channel->IsNoneEvent()) {
        if (iterator != channels_.end()) {
            UpdateEpoll(EPOLL_CTL_DEL, channel);
            channels_.erase(iterator);
        }
        return;
    }

    if (iterator == channels_.end()) {
        if (UpdateEpoll(EPOLL_CTL_ADD, channel)) {
            channels_[socket] = channel;
        }
    } else {
        UpdateEpoll(EPOLL_CTL_MOD, channel);
    }

    Wakeup();
}

void EpollTaskScheduler::RemoveChannel(Channel* channel)
{
    if (!channel) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto iterator = channels_.find(channel->GetSocket());
    if (iterator == channels_.end()) {
        return;
    }

    UpdateEpoll(EPOLL_CTL_DEL, channel);
    channels_.erase(iterator);

    Wakeup();
}

bool EpollTaskScheduler::HandleEvent()
{
    int count = epoll_wait(
        epoll_fd_,
        events_.data(),
        static_cast<int>(events_.size()),
        1000);

    if (count < 0) {
        if (errno == EINTR) {
            return true;
        }

        std::perror("epoll_wait");
        return false;
    }

    for (int index = 0; index < count; ++index) {
        int socket = events_[index].data.fd;

        if (socket == wakeup_fd_) {
            HandleWakeup();
            continue;
        }

        Channel* channel = nullptr;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto iterator = channels_.find(socket);
            if (iterator != channels_.end()) {
                channel = iterator->second;
            }
        }

        if (channel) {
            channel->HandleEvent(
                FromEpollEvents(events_[index].events));
        }
    }

    if (count == static_cast<int>(events_.size())) {
        events_.resize(events_.size() * 2);
    }

    return true;
}

void EpollTaskScheduler::Wakeup()
{
    if (wakeup_fd_ < 0) {
        return;
    }

    uint64_t value = 1;
    ssize_t result = write(
        wakeup_fd_,
        &value,
        sizeof(value));

    if (result < 0 && errno != EAGAIN) {
        std::perror("eventfd write");
    }
}

void EpollTaskScheduler::HandleWakeup()
{
    uint64_t value = 0;

    while (read(
               wakeup_fd_,
               &value,
               sizeof(value)) > 0) {
    }

    if (errno != EAGAIN) {
        // eventfd 已经被读空时一般返回 EAGAIN。
    }
}