#pragma once
#include <vector>
#include <cstdint>

class AESXTS {
public:
    static std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext,
                                        const std::vector<uint8_t>& key,
                                        uint64_t tweak);
};