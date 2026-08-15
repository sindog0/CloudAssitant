#include "Buffer.h"
#include "TcpSocket.h"
#include "Channel.h"

int main()
{
    TcpSocket server_socket;
    server_socket.Bind("127.0.0.1", 9999);
    return 0;
}