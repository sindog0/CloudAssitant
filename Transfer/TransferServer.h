#pragma once

#include "Net/TcpServer.h"
#include <filesystem>

class EventLoop;

class TransferServer : public TcpServer
{
public:
    TransferServer(EventLoop *loop, std::filesystem::path output_directory);

protected:
    TcpConnection::Ptr OnConnect(int socket) override;

private:
    EventLoop *loop_;
    std::filesystem::path output_directory_;
};
