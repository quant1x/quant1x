# ==============================
# fmt / spdlog —— Conan 提供（spdlog 使用外部 fmt）
# ==============================
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
if (TARGET spdlog::spdlog_header_only)
    set(_spdlog_target spdlog::spdlog_header_only)
else ()
    set(_spdlog_target spdlog::spdlog)
endif ()
target_link_libraries(third_libs INTERFACE fmt::fmt ${_spdlog_target})
echo_lib_version(fmt ${fmt_VERSION})
echo_lib_version(spdlog ${spdlog_VERSION})

if(NOT WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # 非 Windows 的 clang 需要 date::date-tz 提供时区数据
    find_package(date CONFIG REQUIRED)
    target_link_libraries(third_libs INTERFACE date::date date::date-tz)
endif()
