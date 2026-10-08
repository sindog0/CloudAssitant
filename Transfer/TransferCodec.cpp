#include "TransferCodec.h"
#include "Net/BufferWriter.h"
#include "Net/BufferReader.h"
#include <cstring>
std::vector<uint8_t> TransferCodec::Encode(const TransferMessage& msg)
{
    const uint32_t payload_size = static_cast<uint32_t>(msg.payload.size());
    std::vector<uint8_t> out(12 + payload_size);
    char *p = reinterpret_cast<char *>(out.data());
    WriteUint8(p, msg.header.version);
    p += 1;
    WriteUint8(p,msg.header.type);
    p += 1;
    WriteUint16BE(p, msg.header.flags);
    p += 2;
    WriteUint32BE(p, msg.header.request_id);
    p += 4;
    WriteUint32BE(p, payload_size);
    p += 4;
    if (payload_size != 0) memcpy(p, msg.payload.data(), payload_size);

    return out;
}

bool TransferCodec::Decode(const char* data, uint32_t size, TransferMessage& msg)
{
    constexpr uint32_t kHeaderSize = 12;
    if(data == nullptr || size < kHeaderSize)
    {
        return false;
    }
    const char* p = data;
    msg.header.version = ReadUint8(p);
    p += 1;
    msg.header.type = ReadUint8(p);
    p += 1;
    msg.header.flags = ReadUint16BE(p);
    p += 2; 
    msg.header.request_id = ReadUint32BE(p);
    p += 4;
    msg.header.payload_size = ReadUint32BE(p);
    p += 4;

    if(msg.header.payload_size != size - kHeaderSize){
        return false;
    }
    msg.payload.assign(reinterpret_cast<const uint8_t *>(p),
                       reinterpret_cast<const uint8_t *>(p) + msg.header.payload_size);
    return true;

}
