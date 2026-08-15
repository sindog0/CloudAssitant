#include "Buffer.h"
#include <sys/uio.h> //iovec
Buffer::Buffer(size_t initial_size)
    : buffer_(initial_size), read_index_(0), write_index_(0) {}

size_t Buffer::ReadableBytes() const {
    return write_index_ - read_index_;
}

size_t Buffer::WriteableBytes() const {
    return buffer_.size() - write_index_;
}

const char* Buffer::Peek() const{
    return buffer_.data() + read_index_;
}

const char* Buffer::BeginWrite() const{
    return buffer_.data() + write_index_;
}

void Buffer::Retrieve(size_t len){
    if(len < ReadableBytes()){
        read_index_ += len;
    } else {
        RetrieveAll();
    }
}

void Buffer::RetrieveAll(){
    read_index_ = 0;
    write_index_ = 0;
}

void Buffer::Append(const char* data, size_t len){
    EnsureWriteableBytes(len);
    std::copy(data, data + len, buffer_.data() + write_index_);
    write_index_ += len;
}

void Buffer::EnsureWriteableBytes(size_t len){
    if(WriteableBytes() < len){
        MakeSpace(len);
    }
}

void Buffer::MakeSpace(size_t len){
    if(WriteableBytes() + read_index_ < len){
        buffer_.resize(write_index_ + len);
    }else{
        size_t readable = ReadableBytes();
        std::copy(buffer_.data() + read_index_, buffer_.data() + write_index_, buffer_.data());
        read_index_ = 0;
        write_index_ = readable;
    }
}

void Buffer::HasWritten(size_t len){
    write_index_ += len;
}

int Buffer::ReadFd(int fd){
    char extra_buffer[65536];//栈上额外的缓冲区
    struct iovec vec[2];
    const size_t writeable = WriteableBytes();
    vec[0].iov_base = buffer_.data() + write_index_;//指向缓冲区可写位置
    vec[0].iov_len = writeable;
    vec[1].iov_base = extra_buffer;//指向额外缓冲区
    vec[1].iov_len = sizeof(extra_buffer);
    
    ssize_t n = readv(fd, vec, 2);//readv可以一次性读取多个缓冲区的数据
    if(n < 0){
        return n;
    }
    if(static_cast<size_t>(n) <= writeable){
        write_index_ += n;
    }else{
        write_index_ = buffer_.size();
        Append(extra_buffer, n - writeable);//将额外缓冲区的数据追加到缓冲区中
    }

    return n;//返回读取的字节数
}