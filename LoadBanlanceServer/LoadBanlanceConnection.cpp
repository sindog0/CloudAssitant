#include "LoadBanlanceConnection.h"
#include <chrono>

#define TIMEOUT 60

LoadBanlanceConnection::LoadBanlanceConnection(std::shared_ptr<LoadBanlanceServer> loadBanlanceServer, TaskScheduler *task_scheduler, int sockfd)
    : TcpConnection(task_scheduler, sockfd), loadBanlanceServer_(loadBanlanceServer), socket_(sockfd)

{
    this->SetReadCallback([this](std::shared_ptr<TcpConnection> conn, BufferReader &buffer)
                          { return this->OnRead(buffer); });
    this->SetDisConnectCallback([this](std::shared_ptr<TcpConnection> conn)
                                { this->DisConnection(); });
}

LoadBanlanceConnection::~LoadBanlanceConnection()
{
}

bool LoadBanlanceConnection::OnRead(BufferReader &buffer)
{
    if (buffer.ReadableBytes() > 0)
    {
        HandleMessage(buffer);
    }
    return true;
}

bool LoadBanlanceConnection::IsTimeOut(uint64_t timestamp)
{
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    // 计算差值
    uint64_t time = nowTimestamp - timestamp; // 系统时间不对，应该是创建linux系统系统时时间没有设好
    return abs(time) > TIMEOUT;
}

void LoadBanlanceConnection::HandleMessage(BufferReader &buffer)
{
    if (buffer.ReadableBytes() < sizeof(packet_head))
    {
        return;
    }

    packet_head *head = (packet_head *)buffer.Peek();
    if (buffer.ReadableBytes() < head->len)
    {
        return;
    }

    switch (head->cmd)
    {
    case Login:
        HnadleLogin(buffer);
        break;
    case Minotor:
        HandleMinoterInfo(buffer);
        break;
    default:
        printf("cmd error\n");
        break;
    }
    buffer.Retrieve(head->len);
}

void LoadBanlanceConnection::HnadleLogin(BufferReader &buffer)
{
    LoginReply reply;
    LoginInfo *info = (LoginInfo *)buffer.Peek();
    if (IsTimeOut(info->timestamp))
    {
        printf("IsTimeout\n");
        reply.cmd = ERROR;
    }
    else
    {

        // 获取ip和端口
        auto server = loadBanlanceServer_.lock();
        if (server)
        {
            Monitor_body *minotor = server->GetMonitorInfo(); // 我们需要在GetMonitorInfo里面做一个资源算法来分配
            reply.ip = minotor->ip;
            reply.port = minotor->port;
        }
        else
        {
            printf("!server\n");
            reply.cmd = ERROR;
        }
    }
    Send((const char *)&reply, reply.len);
}

void LoadBanlanceConnection::HandleMinoterInfo(BufferReader &buffer)
{
    //处理心跳
    //获取info
    Monitor_body* body = (Monitor_body*)buffer.Peek(); 
    auto server = loadBanlanceServer_.lock();
    if(server)
    {
        server->UpdateMonitor(socket_,body);
    }
}

void LoadBanlanceConnection::DisConnection()
{

}