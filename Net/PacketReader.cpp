#include "PacketReader.h"


size_t PacketReader::ReadableBytes() const
{
    return buffer_.ReadableBytes();
}

uint16_t PacketReader::PeekUint16BE(size_t offset) const
{
    if(buffer_.ReadableBytes() < offset + sizeof(uint16_t)){
        return 0;
    }

    const char* data = buffer_.Peek() + offset;
    return (static_cast<uint16_t>(static_cast<uint8_t>(data[0])) << 8) |
           static_cast<uint16_t>(static_cast<uint8_t>(data[1]));
}

uint32_t PacketReader::PeekUint32BE(size_t offset) const
{
    if(buffer_.ReadableBytes() < offset + sizeof(uint32_t)){
        return 0;
    }

    const char* data = buffer_.Peek() + offset;
    return (static_cast<uint32_t>(static_cast<uint8_t>(data[0])) << 24) |
           (static_cast<uint32_t>(static_cast<uint8_t>(data[1])) << 16) |
           (static_cast<uint32_t>(static_cast<uint8_t>(data[2])) << 8) |
           static_cast<uint32_t>(static_cast<uint8_t>(data[3]));
}

uint8_t PacketReader::ReadUint8(){
    uint8_t value = 0;
    if(buffer_.ReadableBytes() >= sizeof(value)){
        value = *reinterpret_cast<const uint8_t*>(buffer_.Peek());
        buffer_.Retrieve(sizeof(value));
    }
    return value;
}

uint16_t PacketReader::ReadUint16BE(){
    uint16_t value = 0;
    if(buffer_.ReadableBytes() >= sizeof(value)){
        value = (static_cast<uint16_t>(static_cast<uint8_t>(buffer_.Peek()[0])) << 8) |
                static_cast<uint16_t>(static_cast<uint8_t>(buffer_.Peek()[1]));
        buffer_.Retrieve(sizeof(value));
    }
    return value;
}

uint32_t PacketReader::ReadUint24BE(){
    uint32_t value = 0;
    if(buffer_.ReadableBytes() >= 3){
        value = (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[0])) << 16) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[1])) << 8) |
                static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[2]));
        buffer_.Retrieve(3);
    }
    return value;
}

uint32_t PacketReader::ReadUint32BE(){
    uint32_t value = 0;
    if(buffer_.ReadableBytes() >= sizeof(value)){
        value = (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[0])) << 24) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[1])) << 16) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[2])) << 8) |
                static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[3]));
        buffer_.Retrieve(sizeof(value));
    }
    return value;
}

uint16_t PacketReader::ReadUint16LE(){
    uint16_t value = 0;
    if(buffer_.ReadableBytes() >= sizeof(value)){
        value = static_cast<uint16_t>(static_cast<uint8_t>(buffer_.Peek()[0])) |
                (static_cast<uint16_t>(static_cast<uint8_t>(buffer_.Peek()[1])) << 8);
        buffer_.Retrieve(sizeof(value));
    }
    return value;
}

uint32_t PacketReader::ReadUint24LE(){
    uint32_t value = 0;
    if(buffer_.ReadableBytes() >= 3){
        value = static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[0])) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[1])) << 8) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[2])) << 16);
        buffer_.Retrieve(3);
    }
    return value;
}

uint32_t PacketReader::ReadUint32LE(){
    uint32_t value = 0;
    if(buffer_.ReadableBytes() >= sizeof(value)){
        value = static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[0])) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[1])) << 8) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[2])) << 16) |
                (static_cast<uint32_t>(static_cast<uint8_t>(buffer_.Peek()[3])) << 24);
        buffer_.Retrieve(sizeof(value));
    }
    return value;
}

std::vector<char> PacketReader::ReadBytes(size_t len)
{
    std::vector<char> data;
    if(buffer_.ReadableBytes() < len){
        return data;
    }

    data.assign(buffer_.Peek(), buffer_.Peek() + len);
    buffer_.Retrieve(len);
    return data;
}
