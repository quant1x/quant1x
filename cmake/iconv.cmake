# libiconv（Conan 提供 Iconv::Iconv）
find_package(Iconv CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE Iconv::Iconv)
echo_lib_version(iconv ${Iconv_VERSION})
