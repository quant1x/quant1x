# libiconv
find_package(Iconv REQUIRED)  # CMake 官方模块名称是 Iconv（大写 I）
if (NOT DEFINED Iconv_VERSION) # 如果没有定义Iconv_VERSION变量, 去查找iconv.h文件路径
    if (DEFINED CACHE{ICONV_INCLUDE_DIR})
        message(WARNING "Cache exists: ICONV_INCLUDE_DIR=${ICONV_INCLUDE_DIR}")
        unset(ICONV_INCLUDE_DIR CACHE)
    endif ()
    find_path(ICONV_INCLUDE_DIR iconv.h PATHS ${VCPKG_INCLUDE_DIRS}  /usr/local/include /usr/include /usr/local/opt/libiconv/include NO_DEFAULT_PATH)
    if (ICONV_INCLUDE_DIR)
        message(WARNING "Iconv exists: ICONV_INCLUDE_DIR=${ICONV_INCLUDE_DIR}")
        # 读取版本宏定义行
        file(STRINGS "${ICONV_INCLUDE_DIR}/iconv.h" ICONV_VERSION_LINE
            REGEX "^#define _LIBICONV_VERSION 0x[0-9A-Fa-f]+")

        # 提取纯十六进制数值部分（去掉注释等）
        string(REGEX MATCH "0x[0-9A-Fa-f]+" ICONV_VERSION_HEX "${ICONV_VERSION_LINE}")

        if (ICONV_VERSION_HEX)
            # 计算主次版本号
            math(EXPR ICONV_VERSION_MAJOR "${ICONV_VERSION_HEX} >> 8" OUTPUT_FORMAT DECIMAL)
            math(EXPR ICONV_VERSION_MINOR "${ICONV_VERSION_HEX} & 0xFF" OUTPUT_FORMAT DECIMAL)

            set(Iconv_VERSION "${ICONV_VERSION_MAJOR}.${ICONV_VERSION_MINOR}")
            message(STATUS "Found libiconv version: ${Iconv_VERSION}")
        else ()
            message(WARNING "Could not extract version number from iconv.h")
        endif ()
    else ()
        message(WARNING "Could not find iconv.h")
    endif ()
endif ()
target_link_libraries(third_libs INTERFACE Iconv::Iconv iconv charset)
echo_lib_version(iconv ${Iconv_VERSION})
if (APPLE OR LINUX)
    # 获取头文件所在的基础目录（通常是 "installed/x64-osx"）
    get_filename_component(ICONV_BASE_DIR "${ICONV_INCLUDE_DIR}" DIRECTORY)

    # 拼接 lib 目录
    set(ICONV_LIB_DIR "${ICONV_BASE_DIR}/lib")

    # 验证 lib 目录是否存在
    if (NOT EXISTS "${ICONV_LIB_DIR}")
        message(FATAL_ERROR "libiconv 的库目录不存在: ${ICONV_LIB_DIR}")
    else ()
        target_link_directories(third_libs INTERFACE ${ICONV_LIB_DIR})
    endif ()
endif ()
