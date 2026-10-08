#include "FileReceiver.h"
#include <array>
#include <iomanip>
#include <sstream>

namespace {
uint32_t UpdateCrc32(uint32_t crc, const uint8_t *data, size_t size)
{
    crc = ~crc;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}
}

FileReceiver::FileReceiver(std::filesystem::path output_directory)
    : output_directory_(std::move(output_directory))
{
    std::error_code error;
    std::filesystem::create_directories(output_directory_, error);
}

std::string FileReceiver::MakeFileId(uint64_t file_size, uint32_t checksum)
{
    std::ostringstream value;
    value << file_size << '-' << std::hex << std::setw(8) << std::setfill('0') << checksum;
    return value.str();
}

std::string FileReceiver::SafeFileName(const std::string &name)
{
    const std::string result = std::filesystem::path(name).filename().string();
    return result.empty() || result == "." || result == ".." ? "unnamed.bin" : result;
}

FileReceiver::Result FileReceiver::Begin(uint32_t request_id, uint64_t file_size,
                                         uint32_t chunk_size, uint32_t checksum,
                                         const std::string &file_name)
{
    if (chunk_size == 0 || file_name.empty()) return {false, 0, "invalid file metadata"};
    Cancel(request_id);
    Session session;
    session.file_size = file_size;
    session.chunk_size = chunk_size;
    session.checksum = checksum;
    session.final_path = output_directory_ / SafeFileName(file_name);
    session.part_path = output_directory_ / ("." + MakeFileId(file_size, checksum) + ".part");
    std::error_code error;
    session.received = std::filesystem::exists(session.part_path, error)
                           ? std::filesystem::file_size(session.part_path, error) : 0;
    if (error || session.received > file_size) {
        session.received = 0;
        std::ofstream truncate(session.part_path, std::ios::binary | std::ios::trunc);
        if (!truncate) return {false, 0, "cannot reset temporary file"};
    }
    session.stream.open(session.part_path, std::ios::binary | std::ios::app);
    if (!session.stream) return {false, 0, "cannot open temporary file"};
    const uint64_t offset = session.received;
    sessions_.emplace(request_id, std::move(session));
    return {true, offset, {}};
}

FileReceiver::Result FileReceiver::Write(uint32_t request_id, uint64_t offset,
                                         const uint8_t *data, size_t size)
{
    auto it = sessions_.find(request_id);
    if (it == sessions_.end()) return {false, 0, "unknown request"};
    Session &session = it->second;
    if (offset != session.received) return {false, session.received, "unexpected offset"};
    if (size > session.chunk_size || size > session.file_size - session.received)
        return {false, session.received, "invalid chunk size"};
    session.stream.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
    if (!session.stream) return {false, session.received, "file write failed"};
    session.received += size;
    return {true, session.received, {}};
}

uint32_t FileReceiver::ChecksumFile(const std::filesystem::path &path)
{
    std::ifstream input(path, std::ios::binary);
    std::array<uint8_t, 64 * 1024> buffer{};
    uint32_t crc = 0;
    while (input) {
        input.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        crc = UpdateCrc32(crc, buffer.data(), static_cast<size_t>(input.gcount()));
    }
    return crc;
}

FileReceiver::Result FileReceiver::Finish(uint32_t request_id, uint32_t checksum)
{
    auto it = sessions_.find(request_id);
    if (it == sessions_.end()) return {false, 0, "unknown request"};
    Session &session = it->second;
    session.stream.close();
    if (session.received != session.file_size) return {false, session.received, "file is incomplete"};
    if (checksum != session.checksum || ChecksumFile(session.part_path) != checksum)
        return {false, session.received, "checksum mismatch"};
    std::error_code error;
    std::filesystem::rename(session.part_path, session.final_path, error);
    if (error) {
        std::filesystem::remove(session.final_path, error);
        error.clear();
        std::filesystem::rename(session.part_path, session.final_path, error);
    }
    if (error) return {false, session.received, "cannot finalize file"};
    const uint64_t offset = session.received;
    sessions_.erase(it);
    return {true, offset, {}};
}

FileReceiver::Result FileReceiver::Resume(const std::string &file_id, uint64_t file_size,
                                          uint32_t checksum) const
{
    if (file_id != MakeFileId(file_size, checksum)) return {false, 0, "invalid file id"};
    const auto path = output_directory_ / ("." + file_id + ".part");
    std::error_code error;
    const uint64_t size = std::filesystem::exists(path, error)
                              ? std::filesystem::file_size(path, error) : 0;
    if (error || size > file_size) return {false, 0, "invalid temporary file"};
    return {true, size, {}};
}

void FileReceiver::Cancel(uint32_t request_id)
{
    auto it = sessions_.find(request_id);
    if (it == sessions_.end()) return;
    it->second.stream.close();
    std::error_code error;
    std::filesystem::remove(it->second.part_path, error);
    sessions_.erase(it);
}
