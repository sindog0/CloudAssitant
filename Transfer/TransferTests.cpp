#include "FileReceiver.h"
#include "TransferCodec.h"
#include <cassert>
#include <filesystem>
#include <string>

int main()
{
    TransferMessage message;
    message.header.version = 1;
    message.header.type = 3;
    message.header.flags = 7;
    message.header.request_id = 42;
    message.payload = {1, 2, 3, 4};
    const auto encoded = TransferCodec::Encode(message);
    TransferMessage decoded;
    assert(TransferCodec::Decode(reinterpret_cast<const char *>(encoded.data()),
                                 static_cast<uint32_t>(encoded.size()), decoded));
    assert(decoded.header.version == 1 && decoded.header.type == 3);
    assert(decoded.header.flags == 7 && decoded.header.request_id == 42);
    assert(decoded.payload == message.payload);

    const auto directory = std::filesystem::temp_directory_path() / "cloudassistant_transfer_test";
    std::error_code error;
    std::filesystem::remove_all(directory, error);
    FileReceiver receiver(directory);
    const std::string contents = "123456789";
    constexpr uint32_t checksum = 0xCBF43926;
    auto begin = receiver.Begin(9, contents.size(), contents.size(), checksum, "../result.bin");
    assert(begin.ok && begin.offset == 0);
    auto chunk = receiver.Write(9, 0, reinterpret_cast<const uint8_t *>(contents.data()), contents.size());
    assert(chunk.ok && chunk.offset == contents.size());
    auto finish = receiver.Finish(9, checksum);
    assert(finish.ok && std::filesystem::exists(directory / "result.bin"));
    assert(!std::filesystem::exists(directory.parent_path() / "result.bin"));
    std::filesystem::remove_all(directory, error);
    return 0;
}
