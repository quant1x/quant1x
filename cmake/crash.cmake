# ==============================
# backward-cpp 依赖
# find_package(backward-cpp CONFIG REQUIRED)
# ==============================
# 链接不同平台的库
if (WIN32)
    target_link_libraries(third_libs INTERFACE Imagehlp) # Windows Crash Dump, PE 文件结构解析
    target_link_libraries(third_libs INTERFACE Dbghelp)  # Windows Crash Dump, 堆栈回溯（Stack Walking）
    target_link_libraries(third_libs INTERFACE advapi32) # Windows 服务管理
    target_link_libraries(third_libs INTERFACE psapi)
    if (MINGW)
        #find_package(libdwarf CONFIG REQUIRED)
        #target_link_libraries(third_libs INTERFACE libdwarf::dwarf)
        target_link_libraries(third_libs INTERFACE bfd)
        target_compile_definitions(third_libs INTERFACE BACKWARD_HAS_BFD=1)
    endif ()
elseif (LINUX)
    # vcpkg版本的libdwarf未实验成功
    #target_link_libraries(third_libs PUBLIC libdwarf::dwarf)
    # #define BACKWARD_HAS_DW 1
    target_link_libraries(third_libs INTERFACE dw bfd) # apt-get install libdw-dev
    # #define BACKWARD_HAS_DWARF 1
    #target_link_libraries(third_libs INTERFACE elf dwarf) # apt-get install libdwarf-dev
elseif (APPLE)
    target_link_libraries(third_libs INTERFACE dwarf) # apt-get install libdw-dev
endif ()