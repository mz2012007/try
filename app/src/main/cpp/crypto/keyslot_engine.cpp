#include "keyslot_engine.h"
#include "aes_xts.h"
#include <argon2.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"KEYSLOT",__VA_ARGS__)

std::vector<uint8_t> KeyslotEngine::deriveVolumeKey(const KeyslotInfo& slot,
                                                     const std::vector<uint8_t>& password,
                                                     const std::vector<uint8_t>& encryptedArea,
                                                     int sectorSize) {
    if (slot.kdf != "argon2id") {
        LOGE("Only argon2id supported");
        return {};
    }
    size_t keyLen = slot.key_size / 8;
    std::vector<uint8_t> kek(keyLen);
    int ret = argon2id_hash_raw(slot.time, slot.memory, slot.parallel,
                                password.data(), password.size(),
                                slot.salt.data(), slot.salt.size(),
                                kek.data(), kek.size());
    if (ret != ARGON2_OK) {
        LOGE("Argon2 failed: %d", ret);
        return {};
    }
    uint64_t tweak = slot.area_offset / sectorSize;
    auto decrypted = AESXTS::decrypt(encryptedArea, kek, tweak);
    if (decrypted.size() < keyLen) return {};
    return std::vector<uint8_t>(decrypted.begin(), decrypted.begin() + keyLen);
}