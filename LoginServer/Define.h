#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <sys/sysinfo.h>
#include <string.h>

#pragma pack(push, 1)
enum Cmd : uint16_t
{
    Minotor,//心跳
    ERROR,
    Login,
    Register,
    Destroy,//注销
};

enum ResultCode
{
	S_OK = 0,
	SERVER_ERROR ,
	REQUEST_TIMEOUT ,
	ALREADY_REDISTERED ,
	USER_DISAPPEAR,
	ALREADY_LOGIN,
	VERFICATE_FAILED
};

struct packet_head
{
    packet_head():len(-1),cmd(-1){}
    uint16_t len;
    uint16_t cmd;
};

struct UserRegister_body : public packet_head
{
    UserRegister_body()
    {
        len = sizeof(UserRegister_body);
        cmd = Register;
    }

    void SetCode(const std::string& str)
    {
        str.copy(code.data(), code.size(), 0);//copy是string的成员函数，参数分别是目标数组，拷贝长度，拷贝起始位置
    }

    std::string GetCode()
    {
        return std::string(code.data());
    }

    void SetName(const std::string& str)
    {
        str.copy(name.data(), name.size(), 0);
    }
    
    std::string GetName()
    {
        return std::string(name.data());
    }

    void SetCount(const std::string& str)
    {
        str.copy(count.data(), count.size(), 0);
    }

    std::string GetCount()
    {
        return std::string(count.data());
    }

    void SetPassword(const std::string& str)
    {
        str.copy(password.data(), password.size(), 0);
    }
    std::string GetPassword()
    {
        return std::string(password.data());
    }

    std::array<char, 20> code;//验证码
    std::array<char, 20> name;
    std::array<char, 20> password;
    std::array<char, 12> count;
    uint64_t timestamp;
};

struct UserLogin : public packet_head
{
    UserLogin():packet_head()
    {
        cmd = Login;
        len = sizeof(UserLogin);
    }

    void SetCode(const std::string& str)
    {
        str.copy(code.data(), code.size(),0);
    }

    std::string GetCode()
    {
        return std::string(code.data());
    }
    void SetCount(const std::string& str)
    {
        str.copy(count.data(),count.size(),0);
    }
    std::string GetCount()
    {
        return std::string(count.data());
    }
    void SetPassword(const std::string& str)
    {
        str.copy(password.data(),password.size(),0);
    }
    std::string GetPassword()
    {
        return std::string(password.data());
    }
    std::array<char,20> code;
    std::array<char,12> count;
    std::array<char,33> password; //Md5
    uint64_t timestamp;
};

struct RegisterResult : public packet_head
{
    RegisterResult() : packet_head()
    {
        cmd = Register;
        len = sizeof(RegisterResult);
    }
	ResultCode resultCode;
};


struct LoginResult : public packet_head
{
    LoginResult():packet_head()
    {
        cmd = Login;
        len = sizeof(LoginResult);
    }

    void SetIp(const std::string& str)
    {
        strncpy(ctrSvrIp.data(), str.c_str(), ctrSvrIp.size() - 1);//str所指向的字符串复制ctrSvrIp，最多复制 ctrSvrIp.size() - 1个字符
        ctrSvrIp.back() = '\0'; // 强制最后一个字符为终止符
    }
    std::string GetIp()
    {
        return std::string(ctrSvrIp.data());
    }
	ResultCode resultCode;
	uint16_t port;
	std::array<char, 16> ctrSvrIp;
};

struct UserDestroy:public packet_head
{
    UserDestroy(): packet_head()
    {
        cmd = Destroy;
        len = sizeof(UserDestroy);
    }

    void SetCode(const std::string& str)
    {
        str.copy(code.data(),code.size(),0);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }

    std::array<char,20> code;
};

struct Monitor_body : public packet_head {
    Monitor_body()
        :packet_head()
    {
        cmd = Minotor;
        len = sizeof(Monitor_body);
        ip.fill('\0');
    }
    void SetIp(const std::string& str)
    {
        str.copy(ip.data(), ip.size(), 0);
    }
    std::string GetIp()
    {
        return std::string(ip.data());
    }
    uint8_t mem;
    std::array<char, 16> ip;
	uint16_t port;
};
#pragma pack(pop)
