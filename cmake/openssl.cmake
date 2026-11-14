find_package(OpenSSL CONFIG REQUIRED)
# Config 模式只定义 OpenSSL_VERSION（模块模式的变量名是 OPENSSL_VERSION），
# 统一回填后供构建摘要输出使用。
if (DEFINED OpenSSL_VERSION AND NOT DEFINED OPENSSL_VERSION)
    set(OPENSSL_VERSION "${OpenSSL_VERSION}")
endif ()
target_link_libraries(third_libs INTERFACE OpenSSL::SSL OpenSSL::Crypto)
echo_lib_version(OpenSSL ${OPENSSL_VERSION})
