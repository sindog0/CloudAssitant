#pragma once

#include "Acceptor.h"
#include "EventLoop.h"
#include "PacketReader.h"
#include "TcpConnection.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class TcpServer
{
public:
    using TcpConnectionPtr =
        std::shared_ptr<TcpConnection>;

    using ConnectionCallback =
        std::function<void(
            const TcpConnectionPtr&)>;

    using MessageCallback =
        std::function<bool(
            const TcpConnectionPtr&,
            PacketReader&)>;

    using CloseCallback =
        std::function<void(
            const TcpConnectionPtr&)>;

    explicit TcpServer(EventLoop* event_loop);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) =
        delete;

    bool Start(
        const std::string& ip,
        uint16_t port,
        int backlog = 128);

    void Stop();

    bool IsRunning() const
    {
        return running_.load();
    }

    std::size_t GetConnectionCount() const;

    void SetConnectionCallback(
        ConnectionCallback callback)
    {
        connection_callback_ =
            std::move(callback);
    }

    void SetMessageCallback(
        MessageCallback callback)
    {
        message_callback_ =
            std::move(callback);
    }

    void SetCloseCallback(
        CloseCallback callback)
    {
        close_callback_ =
            std::move(callback);
    }

private:
    void HandleNewConnection(
        int socket_fd);

    void HandleConnectionClosed(
        const TcpConnectionPtr& connection);

private:
    EventLoop* event_loop_ = nullptr;

    // 保证 Acceptor 使用的调度器生命周期有效。
    std::shared_ptr<TaskScheduler>
        accept_scheduler_;

    std::unique_ptr<Acceptor> acceptor_;

    mutable std::mutex connections_mutex_;

    std::unordered_map<
        int,
        TcpConnectionPtr> connections_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;

    std::atomic_bool running_{false};
};