# ==============================
# BS::thread_pool(bshoshany-thread-pool) —— Conan 提供
# ==============================
set(BSHOSHANY_THREAD_POOL_VERSION "${bshoshany-thread-pool_VERSION}")
if (NOT BSHOSHANY_THREAD_POOL_VERSION)
    set(BSHOSHANY_THREAD_POOL_VERSION "unknown")
endif ()
echo_lib_version(BS::thread_pool ${BSHOSHANY_THREAD_POOL_VERSION})
