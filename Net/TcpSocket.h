#pragma once

#include <cstdint>
#include <string>

class TcpSocket
{
public:
    TcpSocket();
    explicit TcpSocket(int socket_fd);
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    TcpSocket(TcpSocket&& other) noexcept;
    TcpSocket& operator=(TcpSocket&& other) noexcept;

    bool Bind(const std::string& ip, uint16_t port);
    bool Listen(int backlog = 128);

    // timeout_ms <= 0 表示立即返回。
    bool Connect(
        const std::string& ip,
        uint16_t port,
        int timeout_ms = 3000);

    int Accept();

    void ShutdownWrite();
    void Close();

    bool IsValid() const
    {
        return socket_fd_ >= 0;
    }

    int GetSocket() const
    {
        return socket_fd_;
    }

    // 放弃当前对象对 socket 的所有权。
    int Release();

private:
    static int CreateSocket();
    static bool SetNonBlocking(int socket_fd);

private:
    int socket_fd_ = -1;
};