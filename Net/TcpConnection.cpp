#include "TcpConnection.h"

#include <cerrno>
#include <cstdio>

#include <sys/socket.h>
#include <unistd.h>

TcpConnection::TcpConnection(
    int socket_fd,
    TaskScheduler* scheduler)
    : scheduler_(scheduler),
      channel_(
          std::make_shared<Channel>(
              socket_fd)),
      read_buffer_(
          std::make_unique<Buffer>()),
      write_buffer_(
          std::make_unique<Buffer>())
{
    channel_->SetReadCallback([this]() {
        HandleRead();
    });

    channel_->SetWriteCallback([this]() {
        HandleWrite();
    });

    channel_->SetCloseCallback([this]() {
        HandleClose();
    });

    channel_->SetErrorCallback([this]() {
        HandleError();
    });
}

TcpConnection::~TcpConnection()
{
    if (!closed_.exchange(true)) {
        if (channel_ && scheduler_) {
            channel_->DisableAll();
            scheduler_->RemoveChannel(
                channel_.get());
        }

        int socket_fd = GetSocket();
        if (socket_fd >= 0) {
            ::close(socket_fd);
        }
    }
}

void TcpConnection::Start()
{
    if (closed_.load() ||
        !channel_ ||
        !scheduler_) {
        return;
    }

    bool expected = false;

    if (!started_.compare_exchange_strong(
            expected,
            true)) {
        return;
    }

    channel_->EnableReading();
    scheduler_->UpdateChannel(
        channel_.get());
}

void TcpConnection::Send(
    const char* data,
    std::size_t length)
{
    if (!data || length == 0) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(
            write_mutex_);

        // 必须在获取锁后再次检查，防止与关闭操作竞争。
        if (closed_.load() ||
            !channel_ ||
            !scheduler_) {
            return;
        }

        write_buffer_->Append(
            data,
            length);

        channel_->EnableWriting();
    }

    scheduler_->UpdateChannel(
        channel_.get());
}

void TcpConnection::Send(
    const std::shared_ptr<char>& data,
    std::size_t length)
{
    if (!data) {
        return;
    }

    Send(data.get(), length);
}

void TcpConnection::Disconnect()
{
    // 保证回调执行期间对象不会销毁。
    TcpConnectionPtr self;

    try {
        self = shared_from_this();
    } catch (...) {
        CloseConnection();
        return;
    }

    if (!CloseConnection()) {
        return;
    }

    if (disconnect_callback_) {
        disconnect_callback_(self);
    }
}

void TcpConnection::HandleRead()
{
    TcpConnectionPtr self;

    try {
        self = shared_from_this();
    } catch (...) {
        return;
    }

    if (closed_.load() ||
        !channel_) {
        return;
    }

    ssize_t result =
        read_buffer_->ReadFd(
            channel_->GetSocket());

    if (result == 0) {
        HandleClose();
        return;
    }

    if (result < 0) {
        if (errno == EINTR ||
            errno == EAGAIN ||
            errno == EWOULDBLOCK) {
            return;
        }

        HandleError();
        return;
    }

    if (!read_callback_) {
        return;
    }

    PacketReader reader(*read_buffer_);

    bool keep_connection =
        read_callback_(self, reader);

    if (!keep_connection) {
        HandleClose();
    }
}

void TcpConnection::HandleWrite()
{
    bool fatal_error = false;

    {
        std::lock_guard<std::mutex> lock(
            write_mutex_);

        if (closed_.load() ||
            !channel_ ||
            !scheduler_) {
            return;
        }

        while (
            write_buffer_->ReadableBytes() > 0) {
            ssize_t written = ::send(
                channel_->GetSocket(),
                write_buffer_->Peek(),
                write_buffer_->ReadableBytes(),
                MSG_NOSIGNAL);

            if (written > 0) {
                write_buffer_->Retrieve(
                    static_cast<std::size_t>(
                        written));
                continue;
            }

            if (written < 0 &&
                errno == EINTR) {
                continue;
            }

            if (written < 0 &&
                (errno == EAGAIN ||
                 errno == EWOULDBLOCK)) {
                break;
            }

            fatal_error = true;
            break;
        }

        if (!fatal_error) {
            if (write_buffer_
                    ->ReadableBytes() == 0) {
                channel_->DisableWriting();
            } else {
                channel_->EnableWriting();
            }
        }
    }

    if (fatal_error) {
        HandleError();
        return;
    }

    scheduler_->UpdateChannel(
        channel_.get());
}

void TcpConnection::HandleClose()
{
    TcpConnectionPtr self;

    try {
        self = shared_from_this();
    } catch (...) {
        CloseConnection();
        return;
    }

    if (!CloseConnection()) {
        return;
    }

    if (close_callback_) {
        close_callback_(self);
    }

    if (disconnect_callback_) {
        disconnect_callback_(self);
    }
}

void TcpConnection::HandleError()
{
    TcpConnectionPtr self;

    try {
        self = shared_from_this();
    } catch (...) {
        CloseConnection();
        return;
    }

    if (!CloseConnection()) {
        return;
    }

    if (close_callback_) {
        close_callback_(self);
    }

    if (disconnect_callback_) {
        disconnect_callback_(self);
    }
}

bool TcpConnection::CloseConnection()
{
    bool expected = false;

    if (!closed_.compare_exchange_strong(
            expected,
            true)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(
        write_mutex_);

    if (!channel_) {
        return true;
    }

    int socket_fd =
        channel_->GetSocket();

    channel_->DisableAll();

    if (scheduler_) {
        scheduler_->RemoveChannel(
            channel_.get());
    }

    if (socket_fd >= 0) {
        ::shutdown(
            socket_fd,
            SHUT_RDWR);

        ::close(socket_fd);
    }

    return true;
}