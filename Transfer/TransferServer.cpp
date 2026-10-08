#include "TransferServer.h"
#include "TransferConnection.h"
#include "Net/EventLoop.h"

TransferServer::TransferServer(EventLoop *loop, std::filesystem::path output_directory)
    : TcpServer(loop), loop_(loop), output_directory_(std::move(output_directory))
{
}

TcpConnection::Ptr TransferServer::OnConnect(int socket)
{
    return std::make_shared<TransferConnection>(loop_->GetTaskSchduler().get(), socket,
                                                output_directory_);
}
