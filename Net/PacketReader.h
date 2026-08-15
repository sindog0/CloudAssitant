#pragma once
#include "Buffer.h"
#include <cstdint>
#include <vector>
class PacketReader
{
public:
    explicit PacketReader(Buffer& buffer) : buffer_(buffer) {}

    size_t ReadableBytes() const;
    uint16_t PeekUint16BE(size_t offset = 0) const;
    uint32_t PeekUint32BE(size_t offset = 0) const;

    uint8_t ReadUint8();
    uint16_t ReadUint16BE();//BE是大端的缩写，这里表示读取大端字节序的16位无符号整数
    uint32_t ReadUint24BE();
    uint32_t ReadUint32BE();

    uint16_t ReadUint16LE();//LE是小端的缩写，这里表示读取小端字节序的16位无符号整数
    uint32_t ReadUint24LE();
    uint32_t ReadUint32LE();
    std::vector<char> ReadBytes(size_t len);
private:
    Buffer& buffer_;
};
