#include "PacketWriter.h"
#include <arpa/inet.h> // for htons, htonl
void PacketWriter::WriteUint8(uint8_t value){
    buffer_.Append(reinterpret_cast<const char*>(&value), sizeof(value));
}

void PacketWriter::WriteUint16BE(uint16_t value){
    uint16_t be_value = htons(value);//网络字节序是大端字节序
    buffer_.Append(reinterpret_cast<const char*>(&be_value), sizeof(be_value));
}

void PacketWriter::WriteUint24BE(uint32_t value){
    char data[3];
    //位运算不会修改原变量
    data[0] = static_cast<char>((value >> 16) & 0xFF);
    data[1] = static_cast<char>((value >> 8) & 0xFF);
    data[2] = static_cast<char>(value & 0xFF);
    buffer_.Append(data, sizeof(data));
}

void PacketWriter::WriteUint32BE(uint32_t value){
    uint32_t be_value = htonl(value);
    buffer_.Append(reinterpret_cast<const char*>(&be_value), sizeof(be_value));
}   

void PacketWriter::WriteUint16LE(uint16_t value){
    char data[2];
    data[0] = static_cast<char>(value & 0xFF);//取低8位
    data[1] = static_cast<char>((value >> 8) & 0xFF);//取高8位
    buffer_.Append(data, sizeof(data));
}

void PacketWriter::WriteUint24LE(uint32_t value){
    char data[3];
    data[0] = static_cast<char>(value & 0xFF);//取低8位
    data[1] = static_cast<char>((value >> 8) & 0xFF);//取中8位
    data[2] = static_cast<char>((value >> 16) & 0xFF);//取高8位
    buffer_.Append(data, sizeof(data));
}

void PacketWriter::WriteUint32LE(uint32_t value){
    char data[4];
    data[0] = static_cast<char>(value & 0xFF);//取低8位
    data[1] = static_cast<char>((value >> 8) & 0xFF);//取次低8位
    data[2] = static_cast<char>((value >> 16) & 0xFF);//取次高8位
    data[3] = static_cast<char>((value >> 24) & 0xFF);//取高8位
    buffer_.Append(data, sizeof(data));
}

void PacketWriter::WriteBytes(const char* data, size_t len){
    buffer_.Append(data, len);
}