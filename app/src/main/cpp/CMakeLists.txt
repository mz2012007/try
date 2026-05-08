cmake_minimum_required(VERSION 3.22)
project("luks2reader")

# ======== cJSON ========
add_library(cjson STATIC cjson/cJSON.c)
target_include_directories(cjson PUBLIC cjson)

# ======== Argon2 ========
# جذر المستودع هو ../../../../ من موقع CMakeLists.txt الحالي
set(ROOT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)
add_subdirectory(${ROOT_PROJECT_DIR}/argon2 ${CMAKE_BINARY_DIR}/argon2)

# ======== lwext4 ========
add_subdirectory(${ROOT_PROJECT_DIR}/lwext4 ${CMAKE_BINARY_DIR}/lwext4)

# ======== المكتبة الرئيسية ========
add_library(native-lib SHARED
    native-lib.cpp
    luks/luks2_parser.cpp
    crypto/aes_xts.cpp
    crypto/disk_decryptor.cpp
    crypto/keyslot_engine.cpp
    io/container_reader.cpp
    fs/luks_blockdev.cpp
    fs/fs_utils.cpp
)

target_include_directories(native-lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ROOT_PROJECT_DIR}/lwext4/include
    ${ROOT_PROJECT_DIR}/argon2/include
)

# ⚠️ حاليًا لا نستخدم OpenSSL (سيُضاف لاحقًا عند الحاجة)
target_link_libraries(native-lib
    cjson
    argon2
    lwext4
    android
    log
)