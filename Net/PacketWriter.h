#pragma once
#include "Buffer.h"
#include <cstdint>
class PacketWriter
{
public:
    explicit PacketWriter(Buffer& buffer) : buffer_(buffer) {}

    void WriteUint8(uint8_t value);
    void WriteUint16BE(uint16_t value);
    void WriteUint24BE(uint32_t value);
    void WriteUint32BE(uint32_t value);

    void WriteUint16LE(uint16_t value);
    void WriteUint24LE(uint32_t value);
    void WriteUint32LE(uint32_t value);
    void WriteBytes(const char* data, size_t len);// 向缓冲区写入指定长度的字节数据
private:
    Buffer& buffer_;
};