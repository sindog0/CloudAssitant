#include "TcpConnection.h"
#include <unistd.h>
TcpConnection::TcpConnection(int fd, TaskScheduler *scheduler)
    : scheduler_(scheduler),
      channel_(std::make_shared<Channel>(fd)),
      read_buffer_(std::make_unique<Buffer>()),
      write_buffer_(std::make_unique<Buffer>())
{
    channel_->SetReadCallback([this]() { HandleRead(); });
    channel_->SetWriteCallback([this]() { HandleWrite(); });
    channel_->SetErrorCallback([this]() { HandleError(); });
    channel_->SetCloseCallback([this]() { HandleClose(); });
}

TcpConnection::~TcpConnection()
{
    closed_ = true;
    if(channel_ && scheduler_){
        channel_->DisableReading();
        channel_->DisableWriting();
        scheduler_->RemoveChannel(channel_.get());
    }
}

void TcpConnection::Send(const char* data, size_t len)
{
    
    if(!closed_){
        mutex_.lock();
        write_buffer_->Append(data, len);
        mutex_.unlock();
        this->HandleWrite();// 直接调用HandleWrite()，尝试立即发送数据
    }

}

void TcpConnection::Send(std::shared_ptr<char> data, size_t len)
{
    if(!closed_){
        mutex_.lock();
        write_buffer_->Append(data.get(), len);
        mutex_.unlock();
        this->HandleWrite();// 直接调用HandleWrite()，尝试立即发送数据
    }
}

void TcpConnection::Disconnect()
{
    if(!closed_){
        closed_ = true;
        Close();
    }
}

void TcpConnection::Close()
{
    if(channel_){
        channel_->DisableReading();
        channel_->DisableWriting();
        scheduler_->RemoveChannel(channel_.get());
        channel_ = nullptr;
    }
    if(disconnect_callback_){
        disconnect_callback_(shared_from_this());
    }
}

void TcpConnection::HandleRead()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(closed_){
            return;
        }
        int bytes_read = read_buffer_->ReadFd(channel_->GetSocket());
        if(bytes_read < 0){
            Close();
            return;
        }
    }
    if(read_callback_){
        PacketReader reader(*read_buffer_);
        bool continue_reading = read_callback_(shared_from_this(), reader);
        if(!continue_reading){
            std::lock_guard<std::mutex> lock(mutex_);
            Close();
        }
    }
}

void TcpConnection::HandleWrite()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(closed_){
        return;
    }
    if(write_buffer_->ReadableBytes() > 0){
        int bytes_written = write(channel_->GetSocket(), write_buffer_->Peek(), write_buffer_->ReadableBytes());
        if(bytes_written < 0){
            Close();
            return;
        }
        write_buffer_->Retrieve(bytes_written);
    }
    if(write_buffer_->ReadableBytes() == 0){
        channel_->DisableWriting();
    }
}

void TcpConnection::HandleClose()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(closed_){
        return;
    }
    closed_ = true;
    Close();
    if(close_callback_){
        close_callback_(shared_from_this());
    }
}

void TcpConnection::HandleError()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(closed_){
        return;
    }
    Close();
}
