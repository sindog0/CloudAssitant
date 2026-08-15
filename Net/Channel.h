#pragma once
#include <functional>
#include <sys/epoll.h>
class Channel
{
public:
    using EventCallback = std::function<void()>;
    Channel(int fd) : sock_fd_(fd) {}
    ~Channel() = default;

    void SetReadCallback(EventCallback cb) { read_callback_ = std::move(cb); }
    void SetWriteCallback(EventCallback cb) { write_callback_ = std::move(cb); }
    void SetErrorCallback(EventCallback cb) { error_callback_ = std::move(cb); }
    void SetCloseCallback(EventCallback cb) { close_callback_ = std::move(cb); }

    int GetSocket() const { return sock_fd_; }
    int GetEvents() const { return events_; }
    void SetEvents(int events) { events_ = events; }

    void EnableReading() { events_ |= (EPOLLIN | EPOLLPRI); } // EPOLLPRI表示高优先级数据可读
    void EnableWriting() { events_ |= EPOLLOUT; }
    void DisableWriting() { events_ &= ~EPOLLOUT; }
    void DisableReading() { events_ &= ~(EPOLLIN | EPOLLPRI); }

    bool IsNoneEvent() const { return events_ == 0; }
    bool IsWriting() const { return events_ & EPOLLOUT; }
    bool IsReading() const { return events_ & (EPOLLIN | EPOLLPRI); }

    void HandleEvent(int events){
        if(events & (EPOLLIN | EPOLLPRI)){
            if(read_callback_){
                read_callback_();
            }
        }
        if(events & EPOLLOUT){
            if(write_callback_){
                write_callback_();
            }
        }
        if(events & EPOLLERR){
            if(error_callback_){
                error_callback_();
            }
        }
        if(events & (EPOLLHUP | EPOLLRDHUP)){
            if(close_callback_){
                close_callback_();
            }
        }
    } 
private:
    int sock_fd_ = 0;
    int events_ = 0;
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
    EventCallback close_callback_;
};
