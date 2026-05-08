#include "luks_blockdev.h"
#include <cstring>

struct LuksCtx {
    DiskDecryptor* dec;
    uint32_t sectorSize;
};

static LuksCtx g_ctx;
static ext4_blockdev g_bd;
static ext4_blockdev_iface g_iface;

static int bd_open(struct ext4_blockdev* bdev) { return 0; }
static int bd_close(struct ext4_blockdev* bdev) { return 0; }
static int bd_bread(struct ext4_blockdev* bdev, void* buf, uint64_t blk_id, uint32_t blk_cnt) {
    LuksCtx* ctx = (LuksCtx*)bdev->bdif->p_user;
    for (uint32_t i = 0; i < blk_cnt; i++) {
        auto sec = ctx->dec->readSector(blk_id + i);
        if (sec.empty()) return -1;
        memcpy((uint8_t*)buf + i * ctx->sectorSize, sec.data(), ctx->sectorSize);
    }
    return 0;
}
static int bd_bwrite(struct ext4_blockdev*, const void*, uint64_t, uint32_t) { return -1; }

void setup_luks_blockdev(DiskDecryptor* dec, uint32_t sectorSize) {
    g_ctx.dec = dec;
    g_ctx.sectorSize = sectorSize;
    memset(&g_iface, 0, sizeof(g_iface));
    g_iface.open = bd_open;
    g_iface.close = bd_close;
    g_iface.bread = bd_bread;
    g_iface.bwrite = bd_bwrite;
    g_iface.p_user = &g_ctx;
    memset(&g_bd, 0, sizeof(g_bd));
    g_bd.bdif = &g_iface;
    g_bd.part_offset = 0;
    g_bd.part_size = 0xFFFFFFFFFFFFFFFF;
    g_bd.block_size = sectorSize;
}

ext4_blockdev* get_luks_blockdev() { return &g_bd; }