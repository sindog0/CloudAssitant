#pragma once
#include "TcpSocket.h"
#include "Channel.h"
#include "PacketReader.h"
#include "PacketWriter.h"
#include <memory>
#include "TaskScheduler.h"

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using DisconnectCallback = std::function<void(TcpConnectionPtr)>;
    using CloseCallback = std::function<void(TcpConnectionPtr)>;
    using ReadCallback = std::function<bool(TcpConnectionPtr, PacketReader &)>;

    TcpConnection(int fd, TaskScheduler *scheduler);
    virtual ~TcpConnection();

    TaskScheduler *GetTaskScheduler() const { return scheduler_; }
    void SetReadCallback(ReadCallback cb) { read_callback_ = std::move(cb); }
    void SetCloseCallback(CloseCallback cb) { close_callback_ = std::move(cb); }
    bool IsClosed() const { return closed_; }
    int GetSocket() const { return channel_->GetSocket(); }
    void Send(const char *data, size_t len);
    void Send(std::shared_ptr<char> data, size_t len);
    void Disconnect();

protected:
    
    void SetDisconnectCallback(DisconnectCallback cb) { disconnect_callback_ = std::move(cb); }

protected:
    friend class TcpServer;

private:
    TaskScheduler *scheduler_;
    ReadCallback read_callback_;
    CloseCallback close_callback_;
    DisconnectCallback disconnect_callback_;
    bool closed_ = false;
    std::shared_ptr<Channel> channel_ = nullptr;
    std::unique_ptr<Buffer> read_buffer_ = nullptr;
    std::unique_ptr<Buffer> write_buffer_ = nullptr;

    std::mutex mutex_;
    void Close();
    void HandleRead();
    void HandleWrite();
    void HandleClose();
    void HandleError();
};