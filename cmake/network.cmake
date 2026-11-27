# ==============================
# asio —— Conan 提供
# ==============================
find_package(asio CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE asio::asio)
set(ASIO_VERSION_FULL "${asio_VERSION}")
if (NOT ASIO_VERSION_FULL)
    set(ASIO_VERSION_FULL "unknown")
endif ()
message(STATUS "asio version: ${ASIO_VERSION_FULL}")

# 链接不同平台的库
if (WIN32)
    target_link_libraries(third_libs INTERFACE ws2_32)   # Windows 网络库, Winsock API
    target_link_libraries(third_libs INTERFACE mswsock)  # Windows 网络扩展（IOCP）
endif ()

# C++ Requests Library (Cpr for curl)
find_package(cpr CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE cpr::cpr)
echo_lib_version(cpr ${cpr_VERSION})
