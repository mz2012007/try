#include <jni.h>
#include <vector>
#include <string>
#include "luks/luks2_parser.h"
#include "crypto/keyslot_engine.h"
#include "crypto/disk_decryptor.h"
#include "io/container_reader.h"
#include "fs/luks_blockdev.h"
#include "fs/fs_utils.h"
#include "ext4.h"
#include <android/log.h>

#define TAG "LUKS2FS"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

static DiskDecryptor* g_dec = nullptr;
static bool g_mounted = false;

extern "C" {

JNIEXPORT jobjectArray JNICALL
Java_com_app_NativeBridge_openContainer(JNIEnv* env, jclass, jstring path_, jstring pass_) {
    const char* path = env->GetStringUTFChars(path_, nullptr);
    const char* pass = env->GetStringUTFChars(pass_, nullptr);
    if (!path || !pass) return nullptr;

    Luks2Parser parser(path);
    if (!parser.parseHeader()) {
        LOGE("parse failed");
        env->ReleaseStringUTFChars(path_, path);
        env->ReleaseStringUTFChars(pass_, pass);
        return nullptr;
    }

    auto& slots = parser.getKeyslots();
    auto& segs = parser.getSegments();
    if (slots.empty() || segs.empty()) {
        env->ReleaseStringUTFChars(path_, path);
        env->ReleaseStringUTFChars(pass_, pass);
        return nullptr;
    }
    auto& slot = slots[0];
    auto& seg = segs[0];

    ContainerReader reader(path);
    if (!reader.open()) {
        env->ReleaseStringUTFChars(path_, path);
        env->ReleaseStringUTFChars(pass_, pass);
        return nullptr;
    }
    auto enc = reader.readBytes(slot.area_offset, slot.area_size);
    reader.close();

    std::vector<uint8_t> password(pass, pass + strlen(pass));
    auto volumeKey = KeyslotEngine::deriveVolumeKey(slot, password, enc, seg.sector_size);
    if (volumeKey.empty()) {
        env->ReleaseStringUTFChars(path_, path);
        env->ReleaseStringUTFChars(pass_, pass);
        return nullptr;
    }

    uint64_t dataOffset = seg.offset * seg.sector_size;
    g_dec = new DiskDecryptor(path, volumeKey, dataOffset, seg.sector_size, seg.cipher);

    setup_luks_blockdev(g_dec, seg.sector_size);
    ext4_blockdev* bd = get_luks_blockdev();

    int rc = ext4_mount(bd, "/", false);
    if (rc != EOK) {
        LOGE("mount fail %d", rc);
        delete g_dec;
        g_dec = nullptr;
        env->ReleaseStringUTFChars(path_, path);
        env->ReleaseStringUTFChars(pass_, pass);
        return nullptr;
    }
    g_mounted = true;

    std::vector<std::string> files;
    if (!listRootFiles(files)) {
        ext4_umount();
        delete g_dec;
        g_dec = nullptr;
        g_mounted = false;
        env->ReleaseStringUTFChars(path_, path);
        env->ReleaseStringUTFChars(pass_, pass);
        return nullptr;
    }

    jclass strClass = env->FindClass("java/lang/String");
    jobjectArray arr = env->NewObjectArray(files.size(), strClass, nullptr);
    for (size_t i = 0; i < files.size(); i++) {
        env->SetObjectArrayElement(arr, i, env->NewStringUTF(files[i].c_str()));
    }

    env->ReleaseStringUTFChars(path_, path);
    env->ReleaseStringUTFChars(pass_, pass);
    return arr;
}

JNIEXPORT jboolean JNICALL
Java_com_app_NativeBridge_extractFile(JNIEnv* env, jclass, jstring src_, jstring dst_) {
    if (!g_mounted) return JNI_FALSE;
    const char* src = env->GetStringUTFChars(src_, nullptr);
    const char* dst = env->GetStringUTFChars(dst_, nullptr);
    if (!src || !dst) return JNI_FALSE;
    bool ok = copyFileOut(src, dst);
    env->ReleaseStringUTFChars(src_, src);
    env->ReleaseStringUTFChars(dst_, dst);
    return ok ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_app_NativeBridge_deleteFile(JNIEnv* env, jclass, jstring path_) {
    if (!g_mounted) return JNI_FALSE;
    const char* path = env->GetStringUTFChars(path_, nullptr);
    if (!path) return JNI_FALSE;
    bool ok = deleteFile(path);
    env->ReleaseStringUTFChars(path_, path);
    return ok ? JNI_TRUE : JNI_FALSE;
}

} // extern "C"