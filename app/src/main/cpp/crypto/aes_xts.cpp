#include "aes_xts.h"
#include <algorithm>   // for std::min
#include <cstring>     // for memcpy

// === نسخة مؤقتة للتجربة فقط (XOR) – تُستبدل لاحقًا بـ OpenSSL ===
std::vector<uint8_t> AESXTS::decrypt(const std::vector<uint8_t>& ciphertext,
                                     const std::vector<uint8_t>& key,
                                     uint64_t /*tweak*/) {
    std::vector<uint8_t> plaintext(ciphertext.size());
    for (size_t i = 0; i < plaintext.size(); ++i) {
        plaintext[i] = ciphertext[i] ^ key[i % key.size()];
    }
    return plaintext;
}