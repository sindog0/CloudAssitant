#pragma once

#include "Buffer.h"
#include "Channel.h"
#include "PacketReader.h"
#include "TaskScheduler.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>

class TcpConnection
    : public std::enable_shared_from_this<
          TcpConnection>
{
public:
    using TcpConnectionPtr =
        std::shared_ptr<TcpConnection>;

    using ReadCallback =
        std::function<bool(
            TcpConnectionPtr,
            PacketReader &)>;

    using CloseCallback =
        std::function<void(
            TcpConnectionPtr)>;

    using DisconnectCallback =
        std::function<void(
            TcpConnectionPtr)>;

    TcpConnection(
        int socket_fd,
        TaskScheduler *scheduler);

    virtual ~TcpConnection();

    TcpConnection(
        const TcpConnection &) = delete;

    TcpConnection &operator=(
        const TcpConnection &) = delete;

    // 必须在 shared_ptr 创建完成，并且回调设置完成后调用。
    void Start();

    void Send(
        const char *data,
        std::size_t length);

    void Send(
        const std::shared_ptr<char> &data,
        std::size_t length);

    void Disconnect();

    bool IsClosed() const
    {
        return closed_.load();
    }

    bool IsStarted() const
    {
        return started_.load();
    }

    int GetSocket() const
    {
        return channel_
                   ? channel_->GetSocket()
                   : -1;
    }

    TaskScheduler *GetTaskScheduler() const
    {
        return scheduler_;
    }

    void SetReadCallback(
        ReadCallback callback)
    {
        read_callback_ =
            std::move(callback);
    }

    void SetCloseCallback(
        CloseCallback callback)
    {
        close_callback_ =
            std::move(callback);
    }

    void SetDisconnectCallback(
        DisconnectCallback callback)
    {
        disconnect_callback_ =
            std::move(callback);
    }

private:
    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();

    // 返回 true 表示本次确实执行了关闭。
    bool CloseConnection();

private:
    TaskScheduler *scheduler_ = nullptr;

    std::shared_ptr<Channel> channel_;
    std::unique_ptr<Buffer> read_buffer_;
    std::unique_ptr<Buffer> write_buffer_;

    ReadCallback read_callback_;
    CloseCallback close_callback_;
    DisconnectCallback disconnect_callback_;

    std::atomic_bool started_{false};
    std::atomic_bool closed_{false};

    std::mutex write_mutex_;
};