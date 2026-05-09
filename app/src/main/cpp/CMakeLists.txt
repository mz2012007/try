cmake_minimum_required(VERSION 3.22)
project("luks2reader")

# ======== cJSON ========
add_library(cjson STATIC cjson/cJSON.c)
target_include_directories(cjson PUBLIC cjson)

# ======== argon2 (كامل) ========
set(ROOT_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../..)
if(EXISTS "${ROOT_PROJECT_DIR}/argon2/CMakeLists.txt")
    # لو وجد CMakeLists خاص بالأرجون (مثلا عند تثبيتها كـ submodule)
    add_subdirectory(${ROOT_PROJECT_DIR}/argon2 ${CMAKE_BINARY_DIR}/argon2)
else()
    # بناء يدوي لجميع الملفات الضرورية
    add_library(argon2 STATIC
        ${ROOT_PROJECT_DIR}/argon2/src/argon2.c
        ${ROOT_PROJECT_DIR}/argon2/src/core.c
        ${ROOT_PROJECT_DIR}/argon2/src/blake2/blake2b.c
        ${ROOT_PROJECT_DIR}/argon2/src/encoding.c
        ${ROOT_PROJECT_DIR}/argon2/src/thread.c
        ${ROOT_PROJECT_DIR}/argon2/src/ref.c
    )
    target_include_directories(argon2 PUBLIC ${ROOT_PROJECT_DIR}/argon2/include)
    # ربط pthread (مطلوبة لـ thread.c)
    find_library(PTHREAD_LIB pthread)
    if(PTHREAD_LIB)
        target_link_libraries(argon2 PUBLIC ${PTHREAD_LIB})
    endif()
endif()

# ======== lwext4: غير مستخدمة حاليًا (نستخدم stub) ========
# add_subdirectory(...)  محذوفة

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
    ${ROOT_PROJECT_DIR}/argon2/include
)

# رابط فقط المكتبات الضرورية (بدون lwext4)
target_link_libraries(native-lib
    cjson
    argon2
    android
    log
)