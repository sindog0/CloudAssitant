#include "TransferServer.h"
#include "Net/EventLoop.h"
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv)
{
    const std::string ip = argc > 1 ? argv[1] : "0.0.0.0";
    const uint16_t port = argc > 2 ? static_cast<uint16_t>(std::strtoul(argv[2], nullptr, 10)) : 6540;
    const std::string directory = argc > 3 ? argv[3] : "received_files";
    EventLoop loop(1);
    TransferServer server(&loop, directory);
    if (!server.Start(ip, port)) {
        std::cerr << "TransferServer start failed\n";
        return 1;
    }
    std::cout << "TransferServer listening on " << ip << ':' << port
              << ", output directory: " << directory << '\n';
    std::cout << "Press Enter to stop.\n";
    std::cin.get();
    return 0;
}
