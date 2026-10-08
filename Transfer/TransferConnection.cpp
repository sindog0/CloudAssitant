#include "TransferConnection.h"
#include "TransferCodec.h"

namespace {
constexpr uint32_t kHeaderSize = 12;
constexpr uint32_t kMaxPayloadSize = 16 * 1024 * 1024;
constexpr uint8_t kProtocolVersion = 1;
constexpr uint16_t kFlagError = 1;

uint64_t ReadU64(const uint8_t *p) {
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) value = (value << 8) | p[i];
    return value;
}
uint32_t ReadU32(const uint8_t *p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}
uint16_t ReadU16(const uint8_t *p) { return uint16_t((uint16_t(p[0]) << 8) | p[1]); }
void PutU64(std::vector<uint8_t> &out, uint64_t value) {
    for (int i = 7; i >= 0; --i) out.push_back(uint8_t(value >> (i * 8)));
}
}

TransferConnection::TransferConnection(TaskScheduler *scheduler, int sockfd,
                                       std::filesystem::path output_directory)
    : TcpConnection(scheduler, sockfd), file_receiver_(std::move(output_directory))
{
    SetReadCallback([this](std::shared_ptr<TcpConnection>, BufferReader &buffer) {
        return OnRead(buffer);
    });
}

TransferConnection::~TransferConnection() = default;

bool TransferConnection::SendMessage(const TransferMessage &message)
{
    if (message.payload.size() > kMaxPayloadSize) return false;
    auto data = TransferCodec::Encode(message);
    Send(reinterpret_cast<const char *>(data.data()), static_cast<uint32_t>(data.size()));
    return true;
}

bool TransferConnection::OnRead(BufferReader &buffer)
{
    while (buffer.ReadableBytes() >= kHeaderSize) {
        const char *data = buffer.Peek();
        const uint32_t payload_size = ReadUint32BE(data + 8);
        if (payload_size > kMaxPayloadSize) return false;
        const uint64_t message_size = uint64_t(kHeaderSize) + payload_size;
        if (buffer.ReadableBytes() < message_size) break;
        TransferMessage message;
        if (!TransferCodec::Decode(data, static_cast<uint32_t>(message_size), message)) return false;
        buffer.Retrieve(static_cast<size_t>(message_size));
        if (message.header.version != kProtocolVersion || !HandleMessage(message)) return false;
    }
    return true;
}

void TransferConnection::OnClose() {}

void TransferConnection::SendAck(uint32_t request_id, uint64_t offset, bool ok)
{
    TransferMessage reply;
    reply.header.version = kProtocolVersion;
    reply.header.type = static_cast<uint8_t>(Transfer::MessageType::FileAck);
    reply.header.flags = ok ? 0 : kFlagError;
    reply.header.request_id = request_id;
    PutU64(reply.payload, offset);
    SendMessage(reply);
}

void TransferConnection::SendResumeResponse(uint32_t request_id, bool accepted, uint64_t offset)
{
    TransferMessage reply;
    reply.header.version = kProtocolVersion;
    reply.header.type = static_cast<uint8_t>(Transfer::MessageType::FileResumeResponse);
    reply.header.request_id = request_id;
    reply.payload.push_back(accepted ? 1 : 0);
    PutU64(reply.payload, offset);
    SendMessage(reply);
}

bool TransferConnection::HandleMessage(const TransferMessage &message)
{
    const auto &p = message.payload;
    const uint32_t id = message.header.request_id;
    switch (static_cast<Transfer::MessageType>(message.header.type)) {
    case Transfer::MessageType::ClipboardData:
        if (p.empty()) return false;
        clipboard_service_.Handle(p[0], p.data() + 1, p.size() - 1);
        return true;
    case Transfer::MessageType::FileBegin: {
        if (p.size() < 18) return false;
        const uint16_t name_size = ReadU16(p.data() + 16);
        if (name_size == 0 || p.size() != size_t(18) + name_size) return false;
        const std::string name(reinterpret_cast<const char *>(p.data() + 18), name_size);
        auto result = file_receiver_.Begin(id, ReadU64(p.data()), ReadU32(p.data() + 8),
                                           ReadU32(p.data() + 12), name);
        SendAck(id, result.offset, result.ok);
        return true;
    }
    case Transfer::MessageType::FileChunk: {
        if (p.size() < 8) return false;
        auto result = file_receiver_.Write(id, ReadU64(p.data()), p.data() + 8, p.size() - 8);
        SendAck(id, result.offset, result.ok);
        return true;
    }
    case Transfer::MessageType::FileEnd: {
        if (p.size() != 4) return false;
        auto result = file_receiver_.Finish(id, ReadU32(p.data()));
        SendAck(id, result.offset, result.ok);
        return true;
    }
    case Transfer::MessageType::FileCancel:
        if (!p.empty()) return false;
        file_receiver_.Cancel(id);
        SendAck(id, 0, true);
        return true;
    case Transfer::MessageType::FileResumeRequest: {
        if (p.size() < 14) return false;
        const uint16_t id_size = ReadU16(p.data());
        if (id_size == 0 || p.size() != size_t(14) + id_size) return false;
        const std::string file_id(reinterpret_cast<const char *>(p.data() + 2), id_size);
        auto result = file_receiver_.Resume(file_id, ReadU64(p.data() + 2 + id_size),
                                             ReadU32(p.data() + 10 + id_size));
        SendResumeResponse(id, result.ok, result.offset);
        return true;
    }
    case Transfer::MessageType::FileAck:
        return p.size() == 8;
    case Transfer::MessageType::FileResumeResponse:
        return p.size() == 9;
    default:
        return false;
    }
}
