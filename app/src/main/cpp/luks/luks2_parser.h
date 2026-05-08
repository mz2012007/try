#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct KeyslotInfo {
    std::string kdf;           // "argon2id"
    int time = 0;
    int memory = 0;            // KiB
    int parallel = 0;
    std::vector<uint8_t> salt;
    uint64_t area_offset = 0;
    uint64_t area_size = 0;
    int key_size = 256;
};

struct SegmentInfo {
    uint64_t offset = 0;       // in sectors
    uint32_t sector_size = 512;
    std::string cipher;        // "aes-xts-plain64"
};

class Luks2Parser {
public:
    explicit Luks2Parser(const std::string& path);
    bool parseHeader();
    const std::vector<KeyslotInfo>& getKeyslots() const;
    const std::vector<SegmentInfo>& getSegments() const;
private:
    std::string path;
    std::vector<KeyslotInfo> keyslots;
    std::vector<SegmentInfo> segments;
    bool readAndParse();
};