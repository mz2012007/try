cmake_minimum_required(VERSION 3.22)
project("luks2reader")

# ======== cJSON ========
add_library(cjson STATIC cjson/cJSON.c)
target_include_directories(cjson PUBLIC cjson)

# ======== Argon2 ========
add_subdirectory(${CMAKE_SOURCE_DIR}/../argon2 ${CMAKE_BINARY_DIR}/argon2)

# ======== lwext4 ========
add_subdirectory(${CMAKE_SOURCE_DIR}/../lwext4 ${CMAKE_BINARY_DIR}/lwext4)

# ======== OpenSSL (build from source) ========
include(ExternalProject)
ExternalProject_Add(
    openssl
    URL https://www.openssl.org/source/openssl-3.2.1.tar.gz
    URL_HASH SHA256=83c7329fe52c850677d75e5d0b0c4d5427fe6d91f4dd0fe7e0f21eacc046fa5e
    CONFIGURE_COMMAND ./Configure android-arm64 no-shared no-tests --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_COMMAND make
    INSTALL_COMMAND make install_sw
    BUILD_IN_SOURCE 1
)
ExternalProject_Get_Property(openssl SOURCE_DIR)
set(OPENSSL_INCLUDE_DIR ${SOURCE_DIR}/include)
file(MAKE_DIRECTORY ${OPENSSL_INCLUDE_DIR})
add_library(ssl STATIC IMPORTED)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${SOURCE_DIR}/libssl.a)
add_library(crypto STATIC IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${SOURCE_DIR}/libcrypto.a)
add_dependencies(ssl openssl)
add_dependencies(crypto openssl)

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
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/../lwext4/include
    ${CMAKE_SOURCE_DIR}/../argon2/include
)

target_link_libraries(native-lib
    cjson
    argon2
    lwext4
    ssl
    crypto
    android
    log
)