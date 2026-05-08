#include "container_reader.h"
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"IO",__VA_ARGS__)

ContainerReader::ContainerReader(const std::string& p) : path(p) {}
ContainerReader::~ContainerReader() { close(); }
bool ContainerReader::open() {
    fd = ::open(path.c_str(), O_RDONLY);
    return fd != -1;
}
void ContainerReader::close() { if (fd != -1) { ::close(fd); fd = -1; } }
std::vector<uint8_t> ContainerReader::readBytes(uint64_t offset, size_t size) {
    std::vector<uint8_t> buf(size);
    ssize_t ret = pread(fd, buf.data(), size, static_cast<off_t>(offset));
    if (ret != static_cast<ssize_t>(size)) {
        LOGE("pread failed");
        return {};
    }
    return buf;
}