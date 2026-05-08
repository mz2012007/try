#pragma once
#include <vector>
#include "../luks/luks2_parser.h"

class KeyslotEngine {
public:
    static std::vector<uint8_t> deriveVolumeKey(const KeyslotInfo& slot,
                                                const std::vector<uint8_t>& password,
                                                const std::vector<uint8_t>& encryptedArea,
                                                int sectorSize = 512);
};