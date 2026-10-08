#pragma once
#include "LoadBanlanceServer.h"

class LoadBanlanceConnection : public TcpConnection
{
public:
    LoadBanlanceConnection(std::shared_ptr<LoadBanlanceServer> loadBanlanceServer, TaskScheduler *task_scheduler, int sockfd);
    ~LoadBanlanceConnection();

protected:
    void DisConnection();
    bool OnRead(BufferReader &buffer);
    bool IsTimeOut(uint64_t timestamp);
    void HandleMessage(BufferReader &buffer);
    void HnadleLogin(BufferReader &buffer);
    void HandleMinoterInfo(BufferReader &buffer);

private:
    int socket_;
    std::weak_ptr<LoadBanlanceServer> loadBanlanceServer_;
};