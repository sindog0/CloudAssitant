#pragma once

#include <cstdint>
#include <array>
#include "LoginServer/Define.h"

#pragma pack(push, 1)
struct LoginInfo:public  packet_head
{
    LoginInfo() : packet_head()
    {
        cmd = Login;
        len = sizeof(LoginInfo);
        timestamp = -1;
    }
    uint64_t timestamp;
};

struct LoginReply: public packet_head
{
    LoginReply():packet_head()
    {
        cmd = Login;  //如果请求超时，将这个cmd置为ERROR
        len = sizeof(LoginReply);
        port = -1;
        ip.fill('\0');
    }
    uint16_t port;
    std::array<char,16> ip;
};

using MinotorPair = std::pair<int, Monitor_body*>;
struct CmpByValue
{
    bool operator()(const MinotorPair& l,const MinotorPair& r)
    {
        return l.second->mem < r.second->mem;//排序，从小到大排序
    }
};
#pragma pack(pop)