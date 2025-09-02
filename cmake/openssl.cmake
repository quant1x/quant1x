find_package(OpenSSL REQUIRED)
# 尝试直接输出版本（如果存在）
if (DEFINED OPENSSL_VERSION)
    message(STATUS "OpenSSL version (predefined): ${OPENSSL_VERSION}")
else ()
    # 如果 OPENSSL_VERSION 未定义，尝试通过命令行工具获取版本
    find_program(OPENSSL_EXECUTABLE openssl)
    if (OPENSSL_EXECUTABLE)
        execute_process(
            COMMAND "${OPENSSL_EXECUTABLE}" version
            OUTPUT_VARIABLE OPENSSL_VERSION_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        message(STATUS "OpenSSL version (from command line): ${OPENSSL_VERSION_OUTPUT}")
    else ()
        message(WARNING "Could not determine OpenSSL version. Please check your installation.")
    endif ()
endif ()
target_link_libraries(third_libs INTERFACE OpenSSL::SSL OpenSSL::Crypto)
echo_lib_version(OpenSSL ${OPENSSL_VERSION})