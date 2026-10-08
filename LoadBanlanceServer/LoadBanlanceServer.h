#pragma once
#include "Net/TcpServer.h"
#include "LoadDefine.h"

class LoadBanlanceServer : public TcpServer, public std::enable_shared_from_this<LoadBanlanceServer>
{
public:
    static std::shared_ptr<LoadBanlanceServer> Create(EventLoop *eventloop);
    ~LoadBanlanceServer();

private:
    friend class LoadBanlanceConnection;
    LoadBanlanceServer(EventLoop *eventloop);
    virtual TcpConnection::Ptr OnConnect(int socket);
    void UpdateMonitor(const int fd, Monitor_body *info);
    Monitor_body *GetMonitorInfo();

private:
    EventLoop* loop_;
    std::mutex mutex_;
    std::map<int, Monitor_body*> monitorInfos_;
};