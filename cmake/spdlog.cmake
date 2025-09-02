## fmt
#find_package(fmt CONFIG REQUIRED)
#target_link_libraries(third_libs INTERFACE fmt::fmt)
set(fmt_VERSION "11.2.0")
echo_lib_version(fmt ${fmt_VERSION})
if(NOT WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # 非windows环境下clang 需要date::date-tz库
    find_package(date CONFIG REQUIRED)
    target_link_libraries(third_libs INTERFACE date::date date::date-tz)
endif()

target_compile_definitions(third_libs INTERFACE FMT_HEADER_ONLY)

# spdlog
#find_package(spdlog CONFIG REQUIRED)
#target_link_libraries(third_libs INTERFACE spdlog::spdlog_header_only)
#set(SPDLOG_FMT_EXTERNAL OFF)
#set(SPDLOG_NO_EXTERNAL_FMT ON)
set(SPDLOG_HEADER_PATH "${INNER_THIRD_PARTY_INCLUDE_DIRS}/spdlog/version.h")
# 检查文件是否存在
if(EXISTS ${SPDLOG_HEADER_PATH})
    # 读取并解析版本号
    file(READ ${SPDLOG_HEADER_PATH} SPDLOG_HEADER_CONTENTS)

    string(REGEX MATCH "#define SPDLOG_VER_MAJOR ([0-9]+)" _ ${SPDLOG_HEADER_CONTENTS})
    if(CMAKE_MATCH_1)
        set(SPDLOG_VERSION_MAJOR ${CMAKE_MATCH_1})
    else()
        message(WARNING "Could not determine SPDLOG_VER_MAJOR from version.h")
        set(SPDLOG_VERSION_MAJOR 0)
    endif()

    string(REGEX MATCH "#define SPDLOG_VER_MINOR ([0-9]+)" _ ${SPDLOG_HEADER_CONTENTS})
    if(CMAKE_MATCH_1)
        set(SPDLOG_VERSION_MINOR ${CMAKE_MATCH_1})
    else()
        message(WARNING "Could not determine SPDLOG_VER_MINOR from version.h")
        set(SPDLOG_VERSION_MINOR 0)
    endif()

    string(REGEX MATCH "#define SPDLOG_VER_PATCH ([0-9]+)" _ ${SPDLOG_HEADER_CONTENTS})
    if(CMAKE_MATCH_1)
        set(SPDLOG_VERSION_PATCH ${CMAKE_MATCH_1})
    else()
        message(WARNING "Could not determine SPDLOG_VER_PATCH from version.h")
        set(SPDLOG_VERSION_PATCH 0)
    endif()

    set(spdlog_VERSION "${SPDLOG_VERSION_MAJOR}.${SPDLOG_VERSION_MINOR}.${SPDLOG_VERSION_PATCH}")
    message(STATUS "Detected spdlog version: ${SPDLOG_VERSION}")
else()
    message(WARNING "spdlog version.h not found at: ${SPDLOG_HEADER_PATH}")
    set(spdlog_VERSION "0.0.0")
endif()
echo_lib_version(spdlog ${spdlog_VERSION})
target_compile_definitions(third_libs INTERFACE SPDLOG_HEADER_ONLY)
