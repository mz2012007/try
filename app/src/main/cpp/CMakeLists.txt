cmake_minimum_required(VERSION 3.22)
project("luks2manager")

# cJSON (local)
add_library(cjson STATIC
    cjson/cJSON.c)
target_include_directories(cjson PUBLIC cjson)

# Argon2 (submodule)
add_subdirectory(${CMAKE_SOURCE_DIR}/../argon2 ${CMAKE_BINARY_DIR}/argon2)
# lwext4 (submodule)
add_subdirectory(${CMAKE_SOURCE_DIR}/../lwext4 ${CMAKE_BINARY_DIR}/lwext4)

add_library(native-lib SHARED
    native-lib.cpp
    luks/luks2_parser.cpp
    crypto/aes_xts.cpp
    crypto/disk_decryptor.cpp
    crypto/keyslot_engine.cpp
    io/container_reader.cpp
    fs/luks_blockdev.cpp
    fs/fs_utils.cpp)

target_include_directories(native-lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../lwext4/include
    ${CMAKE_SOURCE_DIR}/../argon2/include)

target_link_libraries(native-lib
    cjson
    argon2
    lwext4
    android
    log)

# OpenSSL crypto library (usually provided by NDK)
find_library(crypto-lib crypto)
target_link_libraries(native-lib ${crypto-lib})