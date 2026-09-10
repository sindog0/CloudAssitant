#pragma once
#include <map>
#include <vector>
#include <cstdint>
#include "Define.h"
#include "Net/TcpConnection.h"

class SigConnection : public TcpConnection
{
public:
    SigConnection(TaskScheduler *task_schduler, int sockfd);
    ~SigConnection();

public:
    bool IsAlive() { return state_ != CLOSE; }
    bool IsNoJoin() { return state_ == NONE; }
    bool IsIdle() { return state_ == IDLE; }
    bool IsBusy() { return state_ == PULLER || state_ == PUSHER; }

    void DisConnect();
    void AddCustom(const std::string &code);
    void RemoveCustom(const std::string &code);

    RoleState GetRoleState() const { return state_; }
    std::string GetCode() const { return code_; }
    std::string GetStreamAddress() const { return stream_address_; }

protected:
    bool OnRead(BufferReader &buffer);
    void HandleMessage(BufferReader &buffer);
    void Clear();

private:
    void HandleJoin(const packet_head *data);
    void HandleObtainStream(const packet_head *data);
    void HandleCreateStream(const packet_head *data);
    void HandleDeleteStream(const packet_head *data);
    void HandleOtherMessage(const packet_head *data);

private:
    void DoObtainStream(const packet_head* data);
    void DoCreateStream(const packet_head* data);
private:
    RoleState state_ = NONE;
    std::string code_;
    std::string stream_address_;
    TcpConnection::Ptr conn_ = nullptr;
    std::vector<std::string> objects_;
};
