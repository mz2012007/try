cmake_minimum_required(VERSION 3.22)
project("luks2reader")

# ======== cJSON ========
add_library(cjson STATIC cjson/cJSON.c)
target_include_directories(cjson PUBLIC cjson)

# ======== argon2 ========
set(ROOT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
if(EXISTS "${ROOT_PROJECT_DIR}/argon2/CMakeLists.txt")
    add_subdirectory(${ROOT_PROJECT_DIR}/argon2 ${CMAKE_BINARY_DIR}/argon2)
else()
    add_library(argon2 STATIC
        ${ROOT_PROJECT_DIR}/argon2/src/argon2.c
        ${ROOT_PROJECT_DIR}/argon2/src/core.c
        ${ROOT_PROJECT_DIR}/argon2/src/blake2/blake2b.c
    )
    target_include_directories(argon2 PUBLIC ${ROOT_PROJECT_DIR}/argon2/include)
endif()

# ======== lwext4 ========
if(EXISTS "${ROOT_PROJECT_DIR}/lwext4/CMakeLists.txt")
    add_subdirectory(${ROOT_PROJECT_DIR}/lwext4 ${CMAKE_BINARY_DIR}/lwext4)
    # إضافة مسار الملفات المُولّدة (generated/ext4_config.h)
    target_include_directories(native-lib PRIVATE ${CMAKE_BINARY_DIR}/lwext4/include)
else()
    message(FATAL_ERROR "lwext4 not found – submodule not initialized?")
endif()

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

target_link_libraries(native-lib
    cjson
    argon2
    lwext4
    android
    log
)