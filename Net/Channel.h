#pragma once

#include <cstdint>
#include <functional>
#include <utility>

class Channel
{
public:
    using EventCallback = std::function<void()>;

    enum Event : uint32_t
    {
        NoneEvent  = 0,
        ReadEvent  = 1 << 0,
        WriteEvent = 1 << 1,
        ErrorEvent = 1 << 2,
        CloseEvent = 1 << 3
    };

    explicit Channel(int socket)
        : socket_(socket)
    {
    }

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    int GetSocket() const
    {
        return socket_;
    }

    uint32_t GetEvents() const
    {
        return events_;
    }

    void SetReadCallback(EventCallback callback)
    {
        read_callback_ = std::move(callback);
    }

    void SetWriteCallback(EventCallback callback)
    {
        write_callback_ = std::move(callback);
    }

    void SetErrorCallback(EventCallback callback)
    {
        error_callback_ = std::move(callback);
    }

    void SetCloseCallback(EventCallback callback)
    {
        close_callback_ = std::move(callback);
    }

    void EnableReading()
    {
        events_ |= ReadEvent;
    }

    void EnableWriting()
    {
        events_ |= WriteEvent;
    }

    void DisableReading()
    {
        events_ &= ~ReadEvent;
    }

    void DisableWriting()
    {
        events_ &= ~WriteEvent;
    }

    void DisableAll()
    {
        events_ = NoneEvent;
    }

    bool IsReading() const
    {
        return (events_ & ReadEvent) != 0;
    }

    bool IsWriting() const
    {
        return (events_ & WriteEvent) != 0;
    }

    bool IsNoneEvent() const
    {
        return events_ == NoneEvent;
    }

    void HandleEvent(uint32_t active_events)
    {
        // 即使收到关闭事件，也可能仍有未读取数据。
        if ((active_events & ReadEvent) && read_callback_) {
            read_callback_();
        }

        if ((active_events & WriteEvent) && write_callback_) {
            write_callback_();
        }

        if ((active_events & ErrorEvent) && error_callback_) {
            error_callback_();
        }

        if ((active_events & CloseEvent) && close_callback_) {
            close_callback_();
        }
    }

private:
    int socket_ = -1;
    uint32_t events_ = NoneEvent;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
    EventCallback close_callback_;
};