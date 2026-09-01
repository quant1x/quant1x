@echo off
rem Single-thread ablation: raw push/pop throughput of the MSVC ringbuffer.
rem
rem LIMITATION (important): this cannot expose cache-line contention or backoff
rem stalls -- without cross-core contention a `lock`ed RMW costs about as much as
rem a plain `mov`, and backoff_spin is never reached with a single thread. The
rem Windows slowdown was invisible here (76.7 M/s, perfectly healthy).
rem Use scripts\msvc_mpmc_probe.bat (multi-threaded scaling curve) instead when
rem judging multi-threaded performance.
rem
rem NOTE: keep this file ASCII-only -- cmd.exe decodes .bat with the ANSI
rem codepage, so non-ASCII bytes in rem lines can be parsed as commands.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set ROOT=D:\projects\quant1x\quant1x
set SRC=%ROOT%\benches\ringbuffer_single_thread_ablation.cpp
set SAFE=%ROOT%\quant1x\base\safe.cpp
set OUT=%ROOT%\build-msvc
if not exist %OUT% mkdir %OUT%

cl /nologo /O2 /std:c++20 /utf-8 /EHsc /I %ROOT% /Fo:%OUT%\ /Fe:%OUT%\mini.exe %SRC% %SAFE%
if errorlevel 1 exit /b 1
%OUT%\mini.exe
