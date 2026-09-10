#include "Net/EventLoop.h"
#include "SigServer.h"

int main()
{
    int count = std::thread::hardware_concurrency();//获取当前系统的CPU核心数 只是一个参考值 不一定线程要用这么多
    EventLoop loop(1);
    std::shared_ptr<SigServer> server = SigServer::Create(&loop);
    if(server->Start("10.211.55.7", 9999))
    {
        printf("SigServer start success\n");
    }
    else
    {
        printf("SigServer start failed\n");
    }
    getchar();//等待用户输入
    return 0;
}