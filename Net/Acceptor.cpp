#include "Acceptor.h"

#include <cstdio>

#include <unistd.h>

Acceptor::Acceptor(
    TaskScheduler* scheduler)
    : scheduler_(scheduler)
{
}

Acceptor::~Acceptor()
{
    Stop();
}

bool Acceptor::Listen(
    const std::string& ip,
    uint16_t port,
    int backlog)
{
    if (listening_.load()) {
        return true;
    }

    if (!scheduler_) {
        std::fprintf(
            stderr,
            "Acceptor scheduler is null\n");
        return false;
    }

    if (!server_socket_.IsValid()) {
        std::fprintf(
            stderr,
            "Acceptor server socket is invalid\n");
        return false;
    }

    if (!server_socket_.Bind(ip, port)) {
        return false;
    }

    if (!server_socket_.Listen(backlog)) {
        return false;
    }

    channel_ = std::make_shared<Channel>(
        server_socket_.GetSocket());

    channel_->SetReadCallback([this]() {
        HandleRead();
    });

    channel_->SetErrorCallback([]() {
        std::fprintf(
            stderr,
            "Acceptor socket error\n");
    });

    channel_->SetCloseCallback([this]() {
        Stop();
    });

    channel_->EnableReading();

    listening_.store(true);

    scheduler_->UpdateChannel(
        channel_.get());

    return true;
}

void Acceptor::Stop()
{
    if (!listening_.exchange(false)) {
        return;
    }

    if (channel_ && scheduler_) {
        channel_->DisableAll();

        scheduler_->RemoveChannel(
            channel_.get());
    }

    server_socket_.Close();
    channel_.reset();
}

void Acceptor::HandleRead()
{
    if (!listening_.load()) {
        return;
    }

    // 一次 EPOLLIN 事件可能对应多个连接。
    // 必须循环 Accept，直到返回 EAGAIN。
    while (listening_.load()) {
        int client_socket =
            server_socket_.Accept();

        if (client_socket < 0) {
            break;
        }

        if (new_connection_callback_) {
            new_connection_callback_(
                client_socket);
        } else {
            // 没有处理回调时，避免泄漏。
            ::close(client_socket);
        }
    }
}