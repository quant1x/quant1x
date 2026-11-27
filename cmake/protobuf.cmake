# ==============================
# protobuf —— Conan 提供（3.21.12，与既有生成代码兼容）
# ==============================
find_package(protobuf CONFIG REQUIRED)
target_link_libraries(third_libs INTERFACE protobuf::protobuf)
set(Protobuf_VERSION "${protobuf_VERSION}")
if (NOT Protobuf_VERSION)
    set(Protobuf_VERSION "unknown")
endif ()
message(STATUS "Protobuf version: ${Protobuf_VERSION}")
echo_lib_version(protobuf ${Protobuf_VERSION})
