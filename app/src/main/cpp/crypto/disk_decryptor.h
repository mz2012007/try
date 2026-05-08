#pragma once
#include <string>
#include <vector>
#include <cstdint>

class DiskDecryptor {
public:
    DiskDecryptor(const std::string& path, const std::vector<uint8_t>& volumeKey,
                  uint64_t dataOffset, uint32_t sectorSize, const std::string& cipher);
    ~DiskDecryptor();
    std::vector<uint8_t> readSector(uint64_t sector);
private:
    std::string path;
    std::vector<uint8_t> volumeKey;
    uint64_t dataOffset;
    uint32_t sectorSize;
    std::string cipher;
    int fd = -1;
    bool open();
};