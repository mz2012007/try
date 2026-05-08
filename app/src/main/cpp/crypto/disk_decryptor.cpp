#include "disk_decryptor.h"
#include "aes_xts.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"DISK",__VA_ARGS__)

DiskDecryptor::DiskDecryptor(const std::string& p, const std::vector<uint8_t>& key,
                             uint64_t off, uint32_t ss, const std::string& c)
: path(p), volumeKey(key), dataOffset(off), sectorSize(ss), cipher(c) {
    open();
}
DiskDecryptor::~DiskDecryptor() { if (fd != -1) close(fd); }

bool DiskDecryptor::open() {
    fd = ::open(path.c_str(), O_RDONLY);
    return fd != -1;
}

std::vector<uint8_t> DiskDecryptor::readSector(uint64_t sector) {
    if (fd == -1) return {};
    uint64_t byteOffset = dataOffset + sector * sectorSize;
    std::vector<uint8_t> encrypted(sectorSize);
    ssize_t ret = pread(fd, encrypted.data(), sectorSize, static_cast<off_t>(byteOffset));
    if (ret != static_cast<ssize_t>(sectorSize)) return {};
    if (cipher == "aes-xts-plain64") {
        return AESXTS::decrypt(encrypted, volumeKey, sector);
    }
    // fallback: return encrypted as-is (should not happen)
    return encrypted;
}#include "disk_decryptor.h"
#include "aes_xts.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"DISK",__VA_ARGS__)

DiskDecryptor::DiskDecryptor(const std::string& p, const std::vector<uint8_t>& key,
                             uint64_t off, uint32_t ss, const std::string& c)
: path(p), volumeKey(key), dataOffset(off), sectorSize(ss), cipher(c) {
    open();
}
DiskDecryptor::~DiskDecryptor() { if (fd != -1) close(fd); }

bool DiskDecryptor::open() {
    fd = ::open(path.c_str(), O_RDONLY);
    return fd != -1;
}

std::vector<uint8_t> DiskDecryptor::readSector(uint64_t sector) {
    if (fd == -1) return {};
    uint64_t byteOffset = dataOffset + sector * sectorSize;
    std::vector<uint8_t> encrypted(sectorSize);
    ssize_t ret = pread(fd, encrypted.data(), sectorSize, static_cast<off_t>(byteOffset));
    if (ret != static_cast<ssize_t>(sectorSize)) return {};
    if (cipher == "aes-xts-plain64") {
        return AESXTS::decrypt(encrypted, volumeKey, sector);
    }
    // fallback: return encrypted as-is (should not happen)
    return encrypted;
}