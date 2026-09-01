@echo off
rem CMake configure helper for a local MSVC + vcpkg build.
rem
rem WARNING: the paths below (Visual Studio install dir, vcpkg root, repo root)
rem are machine-specific. Adjust them before using this on another machine.
rem
rem NOTE: keep this file ASCII-only -- cmd.exe decodes .bat with the ANSI
rem codepage, so non-ASCII bytes in rem lines can be parsed as commands.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cmake -S D:\projects\quant1x\quant1x -B D:\projects\quant1x\quant1x\build-msvc -G "Visual Studio 17 2022" -A x64 "-DCMAKE_GENERATOR_INSTANCE=C:\Program Files\Microsoft Visual Studio\2022\Community" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=d:/runtime/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DVCPKG_MANIFEST_MODE=OFF
