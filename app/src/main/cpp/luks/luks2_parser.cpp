#include "luks2_parser.h"
#include <fstream>
#include <cstring>
#include "cjson/cJSON.h"
#include <android/log.h>

#define TAG "LUKS2"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

Luks2Parser::Luks2Parser(const std::string& p) : path(p) {}

static std::string getJsonStr(cJSON* obj, const char* key) {
    cJSON* item = cJSON_GetObjectItem(obj, key);
    return cJSON_IsString(item) ? item->valuestring : "";
}
static int getJsonInt(cJSON* obj, const char* key, int def = 0) {
    cJSON* item = cJSON_GetObjectItem(obj, key);
    return cJSON_IsNumber(item) ? item->valueint : def;
}
static std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        unsigned int byte;
        sscanf(hex.c_str() + i, "%2x", &byte);
        bytes.push_back(static_cast<uint8_t>(byte));
    }
    return bytes;
}

bool Luks2Parser::readAndParse() {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    // read first 4096 bytes
    std::vector<uint8_t> header(4096);
    file.read(reinterpret_cast<char*>(header.data()), 4096);
    if (file.gcount() != 4096) return false;

    const uint8_t magic[6] = {0x4C, 0x55, 0x4B, 0x53, 0xBA, 0xBE};
    if (memcmp(header.data(), magic, 6) != 0) return false;
    uint16_t version = (header[6] << 8) | header[7];
    if (version != 2) return false;

    // read hdr_size (big-endian uint64 at offset 8)
    uint64_t hdr_size = 0;
    for (int i = 0; i < 8; i++) hdr_size = (hdr_size << 8) | header[8 + i];
    if (hdr_size < 4096) return false;

    uint64_t json_size = hdr_size - 4096;
    std::vector<char> jsonBuf(json_size);
    file.seekg(4096);
    file.read(jsonBuf.data(), json_size);
    if (static_cast<uint64_t>(file.gcount()) != json_size) return false;
    std::string jsonStr(jsonBuf.data(), json_size);
    // strip trailing null chars
    while (!jsonStr.empty() && jsonStr.back() == '\0') jsonStr.pop_back();

    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (!root) return false;

    // keyslots
    cJSON* ks = cJSON_GetObjectItem(root, "keyslots");
    if (cJSON_IsObject(ks)) {
        for (cJSON* slot = ks->child; slot; slot = slot->next) {
            KeyslotInfo info;
            cJSON* kdfObj = cJSON_GetObjectItem(slot, "kdf");
            if (cJSON_IsObject(kdfObj)) {
                info.kdf = getJsonStr(kdfObj, "type");
                cJSON* params = cJSON_GetObjectItem(kdfObj, "params");
                if (cJSON_IsObject(params)) {
                    info.time = getJsonInt(params, "time");
                    info.memory = getJsonInt(params, "mem");
                    info.parallel = getJsonInt(params, "parallel");
                    std::string saltStr = getJsonStr(params, "salt");
                    info.salt = hexToBytes(saltStr);
                }
            }
            info.key_size = getJsonInt(slot, "key_size", 256);
            cJSON* area = cJSON_GetObjectItem(slot, "area");
            if (cJSON_IsObject(area)) {
                info.area_offset = getJsonInt(area, "offset");
                info.area_size = getJsonInt(area, "size");
            }
            keyslots.push_back(info);
        }
    }

    // segments
    cJSON* segs = cJSON_GetObjectItem(root, "segments");
    if (cJSON_IsObject(segs)) {
        for (cJSON* seg = segs->child; seg; seg = seg->next) {
            SegmentInfo s;
            s.cipher = getJsonStr(seg, "encryption");
            s.sector_size = getJsonInt(seg, "sector_size", 512);
            s.offset = static_cast<uint64_t>(getJsonInt(seg, "offset"));
            segments.push_back(s);
        }
    }

    cJSON_Delete(root);
    return !keyslots.empty() && !segments.empty();
}

bool Luks2Parser::parseHeader() { return readAndParse(); }
const std::vector<KeyslotInfo>& Luks2Parser::getKeyslots() const { return keyslots; }
const std::vector<SegmentInfo>& Luks2Parser::getSegments() const { return segments; }