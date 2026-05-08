#pragma once
#include "ext4_blockdev.h"
#include "../crypto/disk_decryptor.h"

void setup_luks_blockdev(DiskDecryptor* dec, uint32_t sectorSize);
ext4_blockdev* get_luks_blockdev();