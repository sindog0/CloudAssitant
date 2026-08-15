#pragma once

#include "Channel.h"
#include "TaskScheduler.h"
#include "TcpSocket.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

class Acceptor
{
public:
    using NewConnectionCallback =
        std::function<void(int socket_fd)>;

    explicit Acceptor(
        TaskScheduler* scheduler);

    ~Acceptor();

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    bool Listen(
        const std::string& ip,
        uint16_t port,
        int backlog = 128);

    void Stop();

    bool IsListening() const
    {
        return listening_.load();
    }

    void SetNewConnectionCallback(
        NewConnectionCallback callback)
    {
        new_connection_callback_ =
            std::move(callback);
    }

private:
    void HandleRead();

private:
    TaskScheduler* scheduler_ = nullptr;

    TcpSocket server_socket_;
    std::shared_ptr<Channel> channel_;

    NewConnectionCallback
        new_connection_callback_;

    std::atomic_bool listening_{false};
};