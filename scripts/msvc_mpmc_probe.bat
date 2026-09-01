@echo off
rem 8P8C contention probe: build benches/ringbuffer_mpmc_probe.cpp with MSVC (cl) and clang-cl.
rem Purpose: compare hot-path codegen between the two backends. The single-thread
rem ablation (msvc_mini_bench.bat) cannot expose cache-line contention, and the
rem Windows x86_64 slowdown only shows up under multi-core contention.
rem
rem Requires safe.cpp (implementation of safe::aligned_alloc / aligned_free).
rem NOTE: keep this file ASCII-only -- cmd.exe decodes .bat with the ANSI codepage,
rem non-ASCII bytes in rem lines can be parsed as commands.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set ROOT=D:\projects\quant1x\quant1x
set SRC=%ROOT%\benches\ringbuffer_mpmc_probe.cpp
set SAFE=%ROOT%\quant1x\base\safe.cpp
set OUT=%ROOT%\build-msvc
if not exist %OUT% mkdir %OUT%

echo ===== build: cl (MSVC) =====
cl /nologo /O2 /Ob2 /std:c++20 /utf-8 /EHsc /I %ROOT% /Fo:%OUT%\ /Fe:%OUT%\probe_cl.exe %SRC% %SAFE%
if errorlevel 1 exit /b 1

echo ===== build: clang-cl =====
clang-cl /nologo /O2 /std:c++20 /EHsc /I %ROOT% /Fe:%OUT%\probe_clang.exe %SRC% %SAFE%
if errorlevel 1 exit /b 1

echo.
echo ---------------- cl (MSVC) ----------------
%OUT%\probe_cl.exe
echo ---------------- clang-cl ----------------
%OUT%\probe_clang.exe
