#pragma once
#include <string>
#include <cstdint>
class TcpSocket
{
public:
    TcpSocket();
    explicit TcpSocket(int fd);
    ~TcpSocket();

    bool Bind(std::string ip, uint16_t port);
    bool Listen(int backlog);
    bool Connect(std::string ip, uint16_t port, int timeout = 0);
    int Accept();
    void Close();

    void ShutdownWrite(); // 为了实现TCP的半关闭
    int GetSocket() const { return sock_fd_; }

private:
    int sock_fd_;
};