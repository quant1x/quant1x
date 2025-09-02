# ==============================
# xsimd 13.2.0
# find_package(xsimd CONFIG REQUIRED)
# ==============================
#message("xsimd_INCLUDE_DIRS = ${xsimd_INCLUDE_DIRS}")
file(STRINGS "${INNER_THIRD_PARTY_INCLUDE_DIRS}/xsimd/config/xsimd_config.hpp" xsimd_version_defines
    REGEX "#define XSIMD_VERSION_(MAJOR|MINOR|PATCH)")
foreach (ver ${xsimd_version_defines})
    if (ver MATCHES "#define XSIMD_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XSIMD_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif ()
endforeach ()
set(xsimd_VERSION ${XSIMD_VERSION_MAJOR}.${XSIMD_VERSION_MINOR}.${XSIMD_VERSION_PATCH})
echo_lib_version(xsimd ${xsimd_VERSION})

# AVX2
if (MSVC)
    # Microsoft compiler
    # AVX2 for Release
    string(APPEND CMAKE_C_FLAGS_RELEASE " /arch:AVX2")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /arch:AVX2")

    # AVX for Debug
    string(APPEND CMAKE_C_FLAGS_DEBUG " /arch:AVX")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /arch:AVX")
else ()
    # gcc, clang, ...
    #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mavx -mavx2 -march=native -mtune=native -fno-tree-vectorize")
    #set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mavx -mavx2 -march=native -mtune=native -fno-tree-vectorize")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mavx -mavx2 -march=native -mtune=native -fno-tree-vectorize")
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx -mavx2 -march=native -mtune=native -fno-tree-vectorize")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mavx -mavx2 -march=native -mtune=native")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mavx -mavx2 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -mavx -mavx2 -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx -mavx2 -march=native -mtune=native")
endif ()

# ==============================
# xtensor 0.26.0
# find_package(xtensor CONFIG REQUIRED)
# ==============================
file(STRINGS "${INNER_THIRD_PARTY_INCLUDE_DIRS}/xtensor/core/xtensor_config.hpp" xtensor_version_defines
    REGEX "#define XTENSOR_VERSION_(MAJOR|MINOR|PATCH)")
foreach (ver ${xtensor_version_defines})
    if (ver MATCHES "#define XTENSOR_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XTENSOR_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif ()
endforeach ()
set(xtensor_VERSION ${XTENSOR_VERSION_MAJOR}.${XTENSOR_VERSION_MINOR}.${XTENSOR_VERSION_PATCH})
echo_lib_version(xtensor ${xtensor_VERSION})

# ==============================
# xtl 0.8.0
# find_package(xtl CONFIG REQUIRED)
# ==============================
file(STRINGS "${INNER_THIRD_PARTY_INCLUDE_DIRS}/xtl/xtl_config.hpp" xtl_version_defines
    REGEX "#define XTL_VERSION_(MAJOR|MINOR|PATCH)")
foreach (ver ${xtl_version_defines})
    if (ver MATCHES "#define XTL_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XTL_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif ()
endforeach ()
set(xtl_VERSION ${XTL_VERSION_MAJOR}.${XTL_VERSION_MINOR}.${XTL_VERSION_PATCH})
echo_lib_version(xtl ${xtl_VERSION})

# nlohmann-json 依赖 inja:x64-mingw-static@3.4.0
set(inja_VERSION 3.4.0)
set(nlohmann_json_VERSION "3.1.1")
add_library(xtensor_optimize INTERFACE)
if(MSVC)
    target_compile_options(xtensor_optimize INTERFACE /EHsc)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(global_compile_options INTERFACE
            /MP           # 多进程编译
        )
    endif ()
    target_compile_options(xtensor_optimize INTERFACE /bigobj)
else()
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-march=native arch_native_supported)
    if(arch_native_supported)
        target_compile_options(xtensor_optimize INTERFACE -march=native)
    endif()
endif()
target_link_libraries(third_libs INTERFACE xtensor_optimize)
