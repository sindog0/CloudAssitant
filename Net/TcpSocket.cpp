#include "TcpSocket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
TcpSocket::TcpSocket(){
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd_ < 0)
    {
        std::perror("socket create error");
    }
}

TcpSocket::TcpSocket(int fd):sock_fd_(fd)
{
    if(sock_fd_ < 0)
    {
        std::perror("socket create error");
    }
}

TcpSocket::~TcpSocket()
{
    Close();
}

bool TcpSocket::Bind(std::string ip, uint16_t port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(bind(sock_fd_, (struct sockaddr*)&addr, sizeof(addr) <0)){
        
        std::perror("bind error");
        return false;
    }
    return true;
}

bool TcpSocket::Listen(int backlog)
{
    if(sock_fd_ < 0){
        std::perror("socket fd error");
        return false;
    }
    if(listen(sock_fd_, backlog) < 0 ){
        std::perror("listen error");
        return false;
    }
    return true;
}

bool TcpSocket::Connect(std::string ip, uint16_t port, int timeout)
{
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if(connect(sock_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        std::perror("connect error");
        return false;
    }
    return true;
}

void TcpSocket::ShutdownWrite()
{
    if(shutdown(sock_fd_, SHUT_WR) < 0){
        std::perror("shutdown write error");
        
    }
    
}

int TcpSocket::Accept()
{
    struct sockaddr_in addr{};
    socklen_t addrlen = sizeof(addr);
    return accept(sock_fd_, (struct sockaddr*)&addr, &addrlen);
}

void TcpSocket::Close()
{
    if(sock_fd_ >=0 ){
        close(sock_fd_);
        sock_fd_ = -1;
    }
}
