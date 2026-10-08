#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

namespace Transfer
{
    enum class MessageType : uint8_t
    {
        ClipboardData = 1,
        FileBegin = 2,
        FileChunk = 3,
        FileEnd = 4,
        FileAck = 5,
        FileCancel = 6,
        FileResumeRequest = 7,
        FileResumeResponse = 8
    };

#pragma pack(push, 1)
    struct MessageHeader
    {
        uint8_t version;
        uint8_t type;
        uint16_t flags;
        uint32_t request_id;
        uint32_t payload_size;
    };
#pragma pack(pop)

    struct ClipboardData
    {
        uint8_t format; // text,image等
        std::vector<uint8_t> data;
    };

    struct FileBegin
    {
        uint32_t request_id;
        uint64_t file_size;
        uint32_t chunk_size;
        uint32_t checksum;
        std::string file_name;
    };

    struct FileChunk
    {
        uint32_t request_id;
        uint64_t offset;
        std::vector<uint8_t> data;
    };

    struct FileEnd
    {
        uint32_t request_id;
        uint32_t checksum;
    };
    struct FileAck
    {
        uint32_t request_id;
        uint64_t received_offset;
    };

    struct FileResumeRequest{
        uint32_t request_id;//本次传输 ID
        std::string file_id;//文件唯一标识
        uint64_t file_size;
        uint32_t checksum;
    };

    struct FileResumeResponse
    {
        uint32_t request_id;
        uint8_t acceptd;
        uint64_t resume_offset;
    };
    
};
