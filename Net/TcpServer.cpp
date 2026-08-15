#include "TcpServer.h"

#include <cstdio>

#include <unistd.h>

TcpServer::TcpServer(EventLoop* event_loop)
    : event_loop_(event_loop)
{
}

TcpServer::~TcpServer()
{
    Stop();
}

bool TcpServer::Start(
    const std::string& ip,
    uint16_t port,
    int backlog)
{
    if (running_.load()) {
        return true;
    }

    if (!event_loop_) {
        std::fprintf(
            stderr,
            "TcpServer event loop is null\n");
        return false;
    }

    accept_scheduler_ =
        event_loop_->GetTaskScheduler();

    if (!accept_scheduler_) {
        std::fprintf(
            stderr,
            "TcpServer accept scheduler is null\n");
        return false;
    }

    acceptor_ = std::make_unique<Acceptor>(
        accept_scheduler_.get());

    acceptor_->SetNewConnectionCallback(
        [this](int socket_fd) {
            HandleNewConnection(socket_fd);
        });

    if (!acceptor_->Listen(
            ip,
            port,
            backlog)) {
        acceptor_.reset();
        accept_scheduler_.reset();
        return false;
    }

    running_.store(true);
    return true;
}

void TcpServer::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }

    if (acceptor_) {
        acceptor_->Stop();
    }

    std::vector<TcpConnectionPtr>
        connections;

    {
        std::lock_guard<std::mutex> lock(
            connections_mutex_);

        connections.reserve(
            connections_.size());

        for (const auto& item :
             connections_) {
            connections.push_back(
                item.second);
        }

        // 先从服务器容器删除。
        // connections 局部变量仍然持有对象。
        connections_.clear();
    }

    for (const auto& connection :
         connections) {
        if (connection) {
            connection->Disconnect();
        }
    }

    acceptor_.reset();
    accept_scheduler_.reset();
}

std::size_t
TcpServer::GetConnectionCount() const
{
    std::lock_guard<std::mutex> lock(
        connections_mutex_);

    return connections_.size();
}

void TcpServer::HandleNewConnection(
    int socket_fd)
{
    if (socket_fd < 0) {
        return;
    }

    if (!running_.load() ||
        !event_loop_) {
        ::close(socket_fd);
        return;
    }

    std::shared_ptr<TaskScheduler>
        scheduler =
            event_loop_->GetTaskScheduler();

    if (!scheduler) {
        ::close(socket_fd);
        return;
    }

    auto connection =
        std::make_shared<TcpConnection>(
            socket_fd,
            scheduler.get());

    connection->SetReadCallback(
        [this](
            TcpConnectionPtr connection,
            PacketReader& reader) {
            if (!message_callback_) {
                // 没有消息处理回调时，
                // 不主动断开连接。
                return true;
            }

            return message_callback_(
                connection,
                reader);
        });

    connection->SetCloseCallback(
        [this](
            TcpConnectionPtr connection) {
            if (close_callback_) {
                close_callback_(connection);
            }
        });

    connection->SetDisconnectCallback(
        [this](
            TcpConnectionPtr connection) {
            HandleConnectionClosed(
                connection);
        });

    {
        std::lock_guard<std::mutex> lock(
            connections_mutex_);

        connections_[socket_fd] =
            connection;
    }

    // TcpConnection 必须先放入连接表，
    // 再注册到 EventLoop。
    connection->Start();

    if (connection_callback_) {
        connection_callback_(connection);
    }
}

void TcpServer::HandleConnectionClosed(
    const TcpConnectionPtr& connection)
{
    if (!connection) {
        return;
    }

    int socket_fd =
        connection->GetSocket();

    std::lock_guard<std::mutex> lock(
        connections_mutex_);

    auto iterator =
        connections_.find(socket_fd);

    if (iterator == connections_.end()) {
        return;
    }

    // 防止旧连接误删复用相同 fd 的新连接。
    if (iterator->second == connection) {
        connections_.erase(iterator);
    }
}