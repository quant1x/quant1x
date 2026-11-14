@echo off
rem CMake configure helper for a local MSVC + Conan build.
rem
rem Requires: Conan 2.x on PATH, Visual Studio 2022 (vcvars64.bat).
rem The dependency profile pins MSVC static runtime (/MT) so it matches
rem USE_STATIC_CRT=ON in cmake/compile_options.cmake.
rem
rem NOTE: keep this file ASCII-only -- cmd.exe decodes .bat with the ANSI
rem codepage, so non-ASCII bytes in rem lines can be parsed as commands.
setlocal

set "REPO_ROOT=%~dp0.."
pushd "%REPO_ROOT%"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

conan install . -of build-msvc -pr:h conan/profiles/windows-msvc-static -pr:b default -s build_type=Release --build=missing
if errorlevel 1 (
    popd
    exit /b 1
)

cmake --fresh -S . -B build-msvc -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=Release ^
      -DCMAKE_TOOLCHAIN_FILE=build-msvc/conan_toolchain.cmake

popd
