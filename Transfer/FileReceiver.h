#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

class FileReceiver
{
public:
    struct Result { bool ok = false; uint64_t offset = 0; std::string error; };
    explicit FileReceiver(std::filesystem::path output_directory = "received_files");
    Result Begin(uint32_t, uint64_t, uint32_t, uint32_t, const std::string &);
    Result Write(uint32_t, uint64_t, const uint8_t *, size_t);
    Result Finish(uint32_t, uint32_t);
    Result Resume(const std::string &, uint64_t, uint32_t) const;
    void Cancel(uint32_t);
    static std::string MakeFileId(uint64_t, uint32_t);

private:
    struct Session {
        uint64_t file_size = 0, received = 0;
        uint32_t chunk_size = 0, checksum = 0;
        std::filesystem::path part_path, final_path;
        std::ofstream stream;
    };
    static std::string SafeFileName(const std::string &);
    static uint32_t ChecksumFile(const std::filesystem::path &);
    std::filesystem::path output_directory_;
    std::unordered_map<uint32_t, Session> sessions_;
};
