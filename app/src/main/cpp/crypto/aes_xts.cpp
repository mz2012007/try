#include "aes_xts.h"
#include <openssl/evp.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"XTS",__VA_ARGS__)

std::vector<uint8_t> AESXTS::decrypt(const std::vector<uint8_t>& ct,
                                     const std::vector<uint8_t>& key, uint64_t tweak) {
    std::vector<uint8_t> pt(ct.size());
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};
    // assume key length 32 bytes (AES-256)
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_xts(), nullptr, nullptr, nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    // set tweak
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_XTS_SET_TWEAK, 8, &tweak) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    // set key
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nullptr) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    int len = 0;
    if (EVP_DecryptUpdate(ctx, pt.data(), &len, ct.data(), ct.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    pt.resize(len);
    EVP_CIPHER_CTX_free(ctx);
    return pt;
}