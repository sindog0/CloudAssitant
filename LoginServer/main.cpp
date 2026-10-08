#include <stdio.h>
#include "Net/EventLoop.h"
#include "LoginServer.h"

int main()
{

    int count = std::thread::hardware_concurrency();
    EventLoop loop(1);
    auto login_server = LoginServer::Create(&loop);

    if (!login_server->Start("10.211.55.7", 9867))
    {
        printf("LoginServer server failed\n");
    }
    printf("LoginServer server success\n");
    getchar();
    return 0;
}