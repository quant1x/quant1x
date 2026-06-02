@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

REM ==============================================================
REM  publish.bat  -  Publish Python package to PyPI
REM  Features:
REM    * Build sdist + wheel
REM    * Upload via twine
REM    * Optional skip clean (--skip-clean)
REM    * Basic dependency checks (python, twine, build)
REM    * Proper error handling (non-zero exit aborts)
REM ==============================================================

REM ---- Parse Args ------------------------------------------------
set SKIP_CLEAN=0
for %%A in (%*) do (
  if /I "%%~A"=="--skip-clean" set SKIP_CLEAN=1
)

REM ---- Dependency Check ------------------------------------------
where python >nul 2>&1 || (echo [FAIL] python not found & exit /b 1)
where twine  >nul 2>&1 || (echo [FAIL] twine  not found & exit /b 1)
python -c "import build" >nul 2>&1 || (echo [FAIL] Python module 'build' not found & exit /b 1)

REM ---- Clean old artifacts (pre) ---------------------------------
if exist dist  (rmdir /S /Q dist)
if exist build (rmdir /S /Q build)
for /d %%D in (*.egg-info) do rmdir /S /Q "%%D" >nul 2>&1
if exist .eggs (rmdir /S /Q .eggs)

REM ---- Build ------------------------------------------------------
echo [INFO] Building sdist + wheel...
python -m build --sdist --wheel || (echo [FAIL] build failed & exit /b 1)
echo [ OK ] Build done

REM ---- Read Version ------------------------------------------------
for /f "usebackq delims=" %%V in (`python -c "import os,re,sys; d='dist'; files=os.listdir(d) if os.path.isdir(d) else []; for f in files: m=re.match(r'^quant1x-(?P<version>[^-]+(?:[-+][^-]+)*)\.(?:tar\.gz|zip|whl)$', f);  if m: print(m.group('version')); sys.exit(0); sys.exit(1)"`) do set PKG_VERSION=%%V
if not defined PKG_VERSION (echo [FAIL] cannot determine version from dist artifacts & exit /b 1)
echo [INFO] Version: %PKG_VERSION%

REM ---- Upload -----------------------------------------------------
echo [INFO] Uploading to PyPI...
twine upload dist/*
if errorlevel 1 (echo [FAIL] upload failed & exit /b 1)
echo [ OK ] Upload done

REM ---- Post Clean -------------------------------------------------
if "%SKIP_CLEAN%"=="1" (
  echo [WARN] Skip clean (--skip-clean)
) else (
  echo [INFO] Cleaning artifacts...
  if exist dist  (rmdir /S /Q dist)
  if exist build (rmdir /S /Q build)
  for /d %%D in (*.egg-info) do rmdir /S /Q "%%D" >nul 2>&1
  if exist .eggs (rmdir /S /Q .eggs)
  echo [ OK ] Clean done
)

echo [INFO] Finished successfully.
endlocal
@echo on