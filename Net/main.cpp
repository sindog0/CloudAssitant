#include "EventLoop.h"
#include "TcpServer.h"

#include <iostream>
#include <string>

int main()
{
    // 注意声明顺序：
    // EventLoop 必须比 TcpServer 更早创建，
    // 从而比 TcpServer 更晚销毁。
    EventLoop event_loop(2);
    TcpServer server(&event_loop);

    server.SetConnectionCallback(
        [](const std::shared_ptr<
           TcpConnection>& connection) {
            std::cout
                << "client connected, fd = "
                << connection->GetSocket()
                << std::endl;
        });

    server.SetMessageCallback(
        [](
            const std::shared_ptr<
                TcpConnection>& connection,
            PacketReader& reader) {
            std::size_t readable =
                reader.ReadableBytes();

            if (readable == 0) {
                return true;
            }

            std::vector<char> data =
                reader.ReadBytes(readable);

            std::string message(
                data.begin(),
                data.end());

            std::cout
                << "received from fd "
                << connection->GetSocket()
                << ": "
                << message
                << std::endl;

            // Echo：把收到的数据原样发送回客户端。
            connection->Send(
                data.data(),
                data.size());

            return true;
        });

    server.SetCloseCallback(
        [](const std::shared_ptr<
           TcpConnection>& connection) {
            std::cout
                << "client closed, fd = "
                << connection->GetSocket()
                << std::endl;
        });

    if (!server.Start(
            "0.0.0.0",
            9999)) {
        std::cerr
            << "start server failed"
            << std::endl;
        return 1;
    }

    std::cout
        << "Echo server listening on "
        << "0.0.0.0:9999"
        << std::endl;

    std::cout
        << "Press Enter to stop..."
        << std::endl;

    std::cin.get();

    server.Stop();

    std::cout
        << "server stopped"
        << std::endl;

    return 0;
}