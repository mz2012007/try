#pragma once
#include <vector>
#include <cstdint>
#include <string>

class ContainerReader {
public:
    explicit ContainerReader(const std::string& path);
    ~ContainerReader();
    bool open();
    std::vector<uint8_t> readBytes(uint64_t offset, size_t size);
    void close();
private:
    std::string path;
    int fd = -1;
};