#pragma once
#include <cstddef>
#include <vector>

class Buffer
{
public:
    explicit Buffer(size_t initial_size = 4096);
    size_t ReadableBytes() const;
    size_t WriteableBytes() const;

    const char *Peek() const; // 返回可读数据的起始位置
    const char *BeginWrite() const; // 返回可写数据的起始位置

    void Retrieve(size_t len); // 消费len字节的可读数据
    void RetrieveAll();        // 消费所有可读数据

    void Append(const char *data, size_t len); // 向缓冲区添加数据
    void EnsureWriteableBytes(size_t len);     // 确保缓冲区有足够的可写空间
    void HasWritten(size_t len);               // 更新写入数据后的缓冲区状态

    void MakeSpace(size_t len); // 扩展缓冲区空间

    int ReadFd(int fd); // 从文件描述符中读取数据到缓冲区

private:
    std::vector<char> buffer_;
    size_t read_index_ = 0;
    size_t write_index_ = 0;
};