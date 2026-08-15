#include "TcpSocket.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <utility>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

int TcpSocket::CreateSocket()
{
    int socket_fd = ::socket(
        AF_INET,
        SOCK_STREAM | SOCK_CLOEXEC,
        0);

    if (socket_fd < 0) {
        std::perror("socket");
        return -1;
    }

    if (!SetNonBlocking(socket_fd)) {
        ::close(socket_fd);
        return -1;
    }

    return socket_fd;
}

bool TcpSocket::SetNonBlocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        std::perror("fcntl F_GETFL");
        return false;
    }

    if (fcntl(
            socket_fd,
            F_SETFL,
            flags | O_NONBLOCK) < 0) {
        std::perror("fcntl F_SETFL");
        return false;
    }

    return true;
}

TcpSocket::TcpSocket()
    : socket_fd_(CreateSocket())
{
}

TcpSocket::TcpSocket(int socket_fd)
    : socket_fd_(socket_fd)
{
    if (socket_fd_ >= 0) {
        SetNonBlocking(socket_fd_);
    }
}

TcpSocket::~TcpSocket()
{
    Close();
}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept
    : socket_fd_(other.socket_fd_)
{
    other.socket_fd_ = -1;
}

TcpSocket& TcpSocket::operator=(
    TcpSocket&& other) noexcept
{
    if (this == &other) {
        return *this;
    }

    Close();

    socket_fd_ = other.socket_fd_;
    other.socket_fd_ = -1;

    return *this;
}

bool TcpSocket::Bind(
    const std::string& ip,
    uint16_t port)
{
    if (socket_fd_ < 0) {
        return false;
    }

    int reuse_address = 1;

    if (setsockopt(
            socket_fd_,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse_address,
            sizeof(reuse_address)) < 0) {
        std::perror("setsockopt SO_REUSEADDR");
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (ip.empty() ||
        ip == "0.0.0.0" ||
        ip == "*") {
        address.sin_addr.s_addr =
            htonl(INADDR_ANY);
    } else {
        int result = inet_pton(
            AF_INET,
            ip.c_str(),
            &address.sin_addr);

        if (result != 1) {
            std::fprintf(
                stderr,
                "invalid IPv4 address: %s\n",
                ip.c_str());
            return false;
        }
    }

    int result = ::bind(
        socket_fd_,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address));

    if (result < 0) {
        std::perror("bind");
        return false;
    }

    return true;
}

bool TcpSocket::Listen(int backlog)
{
    if (socket_fd_ < 0) {
        return false;
    }

    if (::listen(socket_fd_, backlog) < 0) {
        std::perror("listen");
        return false;
    }

    return true;
}

bool TcpSocket::Connect(
    const std::string& ip,
    uint16_t port,
    int timeout_ms)
{
    if (socket_fd_ < 0) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            ip.c_str(),
            &address.sin_addr) != 1) {
        std::fprintf(
            stderr,
            "invalid IPv4 address: %s\n",
            ip.c_str());
        return false;
    }

    int result = ::connect(
        socket_fd_,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address));

    if (result == 0) {
        return true;
    }

    if (errno != EINPROGRESS) {
        std::perror("connect");
        return false;
    }

    if (timeout_ms <= 0) {
        return false;
    }

    pollfd descriptor{};
    descriptor.fd = socket_fd_;
    descriptor.events = POLLOUT;

    do {
        result = ::poll(
            &descriptor,
            1,
            timeout_ms);
    } while (result < 0 && errno == EINTR);

    if (result == 0) {
        std::fprintf(stderr, "connect timeout\n");
        return false;
    }

    if (result < 0) {
        std::perror("poll");
        return false;
    }

    int socket_error = 0;
    socklen_t error_length =
        sizeof(socket_error);

    if (getsockopt(
            socket_fd_,
            SOL_SOCKET,
            SO_ERROR,
            &socket_error,
            &error_length) < 0) {
        std::perror("getsockopt SO_ERROR");
        return false;
    }

    if (socket_error != 0) {
        std::fprintf(
            stderr,
            "connect error: %s\n",
            std::strerror(socket_error));
        return false;
    }

    return true;
}

int TcpSocket::Accept()
{
    if (socket_fd_ < 0) {
        return -1;
    }

    sockaddr_in client_address{};
    socklen_t address_length =
        sizeof(client_address);

    int client_socket = accept4(
        socket_fd_,
        reinterpret_cast<sockaddr*>(
            &client_address),
        &address_length,
        SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (client_socket < 0) {
        if (errno != EAGAIN &&
            errno != EWOULDBLOCK &&
            errno != EINTR) {
            std::perror("accept4");
        }

        return -1;
    }

    return client_socket;
}

void TcpSocket::ShutdownWrite()
{
    if (socket_fd_ < 0) {
        return;
    }

    if (::shutdown(socket_fd_, SHUT_WR) < 0) {
        if (errno != ENOTCONN) {
            std::perror("shutdown");
        }
    }
}

void TcpSocket::Close()
{
    if (socket_fd_ < 0) {
        return;
    }

    ::close(socket_fd_);
    socket_fd_ = -1;
}

int TcpSocket::Release()
{
    int socket_fd = socket_fd_;
    socket_fd_ = -1;
    return socket_fd;
}