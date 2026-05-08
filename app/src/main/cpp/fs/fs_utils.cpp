#include "fs_utils.h"
#include "ext4.h"
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"FS",__VA_ARGS__)

bool listRootFiles(std::vector<std::string>& out) {
    ext4_dir dir;
    if (ext4_dir_open(&dir, "/") != EOK) return false;
    const ext4_direntry* e;
    while (ext4_dir_read(&dir, &e) == EOK) {
        if (e->name_length > 0) {
            std::string name(e->name, e->name_length);
            if (name != "." && name != "..") out.push_back(name);
        }
    }
    ext4_dir_close(&dir);
    return true;
}

bool copyFileOut(const char* srcPath, const char* dstAndroidPath) {
    ext4_file file;
    if (ext4_fopen(&file, srcPath, "r") != EOK) return false;
    int fd = open(dstAndroidPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { ext4_fclose(&file); return false; }
    uint8_t buf[4096];
    size_t read;
    while (ext4_fread(&file, buf, sizeof(buf), &read) == EOK && read > 0) {
        write(fd, buf, read);
    }
    close(fd);
    ext4_fclose(&file);
    return true;
}

bool deleteFile(const char* path) {
    return ext4_fremove(path) == EOK;
}