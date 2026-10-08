#include <stdio.h>
#include "LoadBanlanceServer.h"
#include <Net/EventLoop.h>

int main()
{
    EventLoop loop(1);
    std::shared_ptr<LoadBanlanceServer> loginServer = nullptr;
    auto loadServer = LoadBanlanceServer::Create(&loop);

    if(loadServer->Start("10.211.55.7",8523))
    {
        loginServer = LoadBanlanceServer::Create(&loop);
        if(loginServer->Start("10.211.55.7",9867))
        {
            printf("server start succefful\n");
        }
    }
    getchar();
    return 0;
}
