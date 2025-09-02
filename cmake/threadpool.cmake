# ==============================
# BS::thread_pool(bshoshany-thread-pool)
# find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
# ==============================
# BS::thread_pool(bshoshany-thread-pool), header-only方式的第三方库收敛到third_party
# find_path(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
set(BSHOSHANY_THREAD_POOL_INCLUDE_DIRS "${INNER_THIRD_PARTY_INCLUDE_DIRS}")
MESSAGE(STATUS "BSHOSHANY_THREAD_POOL_INCLUDE_DIRS = ${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS}")
## 查找 thread_pool.hpp 文件路径
## find_path(THREAD_POOL_INCLUDE_DIR BS_thread_pool.hpp PATHS ${CMAKE_PREFIX_PATH}/include /usr/include /usr/local/include ${CMAKE_SOURCE_DIR}/third_party)
if (BSHOSHANY_THREAD_POOL_INCLUDE_DIRS)
    # 提取 BS_THREAD_POOL_VERSION_MAJOR 宏
    file(STRINGS "${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS}/BS_thread_pool.hpp" THREAD_POOL_VERSION_MAJOR_LINE REGEX "#define BS_THREAD_POOL_VERSION_MAJOR")
    if (THREAD_POOL_VERSION_MAJOR_LINE)
        string(REGEX MATCH "[0-9]+" THREAD_POOL_VERSION_MAJOR "${THREAD_POOL_VERSION_MAJOR_LINE}")
    endif ()

    # 提取 BS_THREAD_POOL_VERSION_MINOR 宏
    file(STRINGS "${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS}/BS_thread_pool.hpp" THREAD_POOL_VERSION_MINOR_LINE REGEX "#define BS_THREAD_POOL_VERSION_MINOR")
    if (THREAD_POOL_VERSION_MINOR_LINE)
        string(REGEX MATCH "[0-9]+" THREAD_POOL_VERSION_MINOR "${THREAD_POOL_VERSION_MINOR_LINE}")
    endif ()

    # 提取 BS_THREAD_POOL_VERSION_PATCH 宏
    file(STRINGS "${BSHOSHANY_THREAD_POOL_INCLUDE_DIRS}/BS_thread_pool.hpp" THREAD_POOL_VERSION_PATCH_LINE REGEX "#define BS_THREAD_POOL_VERSION_PATCH")
    if (THREAD_POOL_VERSION_PATCH_LINE)
        string(REGEX MATCH "[0-9]+" THREAD_POOL_VERSION_PATCH "${THREAD_POOL_VERSION_PATCH_LINE}")
    endif ()

    # 组合完整版本号
    set(THREAD_POOL_VERSION_FULL "${THREAD_POOL_VERSION_MAJOR}.${THREAD_POOL_VERSION_MINOR}.${THREAD_POOL_VERSION_PATCH}")

    # 显示版本信息
    message(STATUS "bshoshany-thread-pool version: ${THREAD_POOL_VERSION_FULL}")
    set(BSHOSHANY_THREAD_POOL_VERSION ${THREAD_POOL_VERSION_FULL})
else ()
    message(WARNING "bshoshany-thread-pool headers not found.")
endif ()
echo_lib_version(BS::thread_pool ${BSHOSHANY_THREAD_POOL_VERSION})