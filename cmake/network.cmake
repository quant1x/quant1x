# ==============================
# asio 依赖
# find_package(asio CONFIG REQUIRED)
# ==============================

#find_package(asio CONFIG REQUIRED)
## 查找 Asio 头文件
if (DEFINED CACHE{ASIO_INCLUDE_DIR})
    message(WARNING "Cache exists: ASIO_INCLUDE_DIR=${ASIO_INCLUDE_DIR}")
    unset(ASIO_INCLUDE_DIR CACHE)
endif ()
find_path(ASIO_INCLUDE_DIR asio.hpp PATHS ${INNER_THIRD_PARTY_INCLUDE_DIRS})
if (ASIO_INCLUDE_DIR)
    # 提取版本信息
    file(STRINGS "${ASIO_INCLUDE_DIR}/asio/version.hpp" ASIO_VERSION_LINE REGEX "#define ASIO_VERSION")
    if (ASIO_VERSION_LINE)
        string(REGEX MATCH "[0-9]+" ASIO_VERSION "${ASIO_VERSION_LINE}")
        math(EXPR ASIO_MAJOR_VERSION "${ASIO_VERSION} / 100000")
        math(EXPR ASIO_MINOR_VERSION "(${ASIO_VERSION} / 100) % 1000")
        math(EXPR ASIO_PATCH_VERSION "${ASIO_VERSION} % 100")
        set(ASIO_VERSION_FULL "${ASIO_MAJOR_VERSION}.${ASIO_MINOR_VERSION}.${ASIO_PATCH_VERSION}")
    endif ()

    file(STRINGS "${ASIO_INCLUDE_DIR}/asio/version.hpp" ASIO_VERSION_STR_LINE REGEX "#define ASIO_VERSION_STR")
    if (ASIO_VERSION_STR_LINE)
        string(REGEX MATCH "\"[^\"]+\"" ASIO_VERSION_STR "${ASIO_VERSION_STR_LINE}")
        string(REPLACE "\"" "" ASIO_VERSION_STR "${ASIO_VERSION_STR}")
    endif ()

    # 显示版本信息
    message(STATUS "asio version (numeric): ${ASIO_VERSION_FULL}")
    message(STATUS "asio version (string): ${ASIO_VERSION_STR}")

    # 添加头文件路径
    include_directories(${ASIO_INCLUDE_DIR})
    set(ASIO_VERSION ${ASIO_VERSION_FULL})
else ()
    message(FATAL_ERROR "Asio headers not found. Please install Asio and specify its path.")
endif ()

# 链接不同平台的库
if (WIN32)
    target_link_libraries(third_libs INTERFACE ws2_32)   # Windows 网络库, 标准 Winsock API 支持, TCP/UDP 通信、客户端/服务器, winsock2.h
    target_link_libraries(third_libs INTERFACE mswsock)  # Windows 网络库, 微软扩展函数（高性能网络）, IOCP、异步网络、高性能服务端, mswsock.h
endif ()

# C++ Requests Library (Cpr for curl)
find_package(cpr CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE cpr::cpr)
echo_lib_version(cpr ${cpr_VERSION})
