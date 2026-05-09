#include "fs_utils.h"
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <android/log.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "FS-STUB", __VA_ARGS__)

bool listRootFiles(std::vector<std::string>& out) {
    // قائمة ملفات وهمية للاختبار
    out.clear();
    out.push_back("test_photo.jpg");
    out.push_back("test_document.pdf");
    out.push_back("test_video.mp4");
    LOGD("Stub file list returned (%zu files)", out.size());
    return true;
}

bool copyFileOut(const char* srcPath, const char* dstAndroidPath) {
    // كتابة محتوى وهمي للملف المُستخرج
    int fd = open(dstAndroidPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return false;
    }
    const char* dummyContent = "This is a dummy file extracted from LUKS container.\n";
    write(fd, dummyContent, strlen(dummyContent));
    close(fd);
    return true;
}

bool deleteFile(const char* path) {
    // محاكاة حذف ناجحة
    return true;
}