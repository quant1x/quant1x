@echo off
rem Scheduling primitive cost probe (sleep_for / yield / Sleep / waitable timers).
rem
rem This is the tool that found the root cause of "C++ several times slower than
rem Rust on Windows x86_64": sleep_for(50us) costs ~15.6ms under MSVC (Windows
rem default timer granularity) while Rust's thread::sleep(50us) costs ~0.55ms.
rem Run it after changing machines, compilers or power policy to re-validate.
rem
rem NOTE: keep this file ASCII-only -- cmd.exe decodes .bat with the ANSI
rem codepage, so non-ASCII bytes in rem lines can be parsed as commands.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set ROOT=D:\projects\quant1x\quant1x
set SRC=%ROOT%\benches\ringbuffer_sched_cost_probe.cpp
set SAFE=%ROOT%\quant1x\base\safe.cpp
set OUT=%ROOT%\build-msvc
if not exist %OUT% mkdir %OUT%

cl /nologo /O2 /std:c++20 /utf-8 /EHsc /I %ROOT% /Fo:%OUT%\ /Fe:%OUT%\sched_probe.exe %SRC% %SAFE%
if errorlevel 1 exit /b 1
%OUT%\sched_probe.exe
