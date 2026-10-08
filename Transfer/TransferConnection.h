#pragma once
#include "Net/TcpConnection.h"
#include "Net/BufferReader.h"
#include "TransferMessage.h"
#include "ClipboardService.h"
#include "FileReceiver.h"
#include <filesystem>
#include <utility>
class TransferConnection : public TcpConnection
{
public:
    TransferConnection(TaskScheduler *scheduler, int sockfd,
                       std::filesystem::path output_directory = "received_files");
    virtual ~TransferConnection();
    bool OnRead(BufferReader &buffer);
    void OnClose();
    bool SendMessage(const TransferMessage &msg);
    void SetClipboardCallback(ClipboardService::Callback callback)
    {
        clipboard_service_.SetCallback(std::move(callback));
    }

private:
    bool HandleMessage(const TransferMessage &message);
    void SendAck(uint32_t request_id, uint64_t offset, bool ok);
    void SendResumeResponse(uint32_t request_id, bool accepted, uint64_t offset);

    FileReceiver file_receiver_;
    ClipboardService clipboard_service_;

};
