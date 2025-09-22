<#
    publish.ps1  -  发布 Python 包到 PyPI

    功能:
      1. 构建 (sdist + wheel)
      2. 上传到 PyPI (twine)
      3. 清理临时构建产物 (dist, build, *.egg-info, .eggs)

    使用:
      powershell -ExecutionPolicy Bypass -File .\publish.ps1

    可选参数:
      -SkipClean   跳过清理临时目录
      -Verbose     输出详细日志 (等价 $VerbosePreference = 'Continue')

    先决条件:
      - 已安装 python, pip, twine
      - 已正确配置 PyPI 凭证 (例如环境变量 TWINE_USERNAME / TWINE_PASSWORD 或 keyring)

#>
[CmdletBinding()]
param(
    [switch]$SkipClean,
    [switch]$NoRich
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
try { chcp 65001 > $null 2>&1 } catch {}
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$env:PYTHONIOENCODING = 'utf-8'
$env:TWINE_NON_INTERACTIVE = '1'
$script:StartTime = Get-Date

function Write-Info($msg)  { Write-Host "[INFO ] $msg" -ForegroundColor Cyan }
function Write-Ok($msg)    { Write-Host "[ OK  ] $msg" -ForegroundColor Green }
function Write-Warn($msg)  { Write-Host "[WARN ] $msg" -ForegroundColor Yellow }
function Write-Err($msg)   { Write-Host "[FAIL ] $msg" -ForegroundColor Red }

function Test-Command($name) { if (-not (Get-Command $name -ErrorAction SilentlyContinue)) { Write-Err "Missing command: $name"; throw "Please install $name" } }
function Invoke-Step { param([string]$Title,[scriptblock]$Action); Write-Info $Title; & $Action; if ($LASTEXITCODE -ne 0) { throw "Step failed: $Title (code=$LASTEXITCODE)" } }

try {
    Write-Info "Console CodePage: $([Console]::OutputEncoding.CodePage)"
    Invoke-Step 'Checking required commands...' { Test-Command python; Test-Command twine; Test-Command pip }
    Write-Ok 'All commands ok'

    Invoke-Step 'Reading version...' { $script:Version = (python setup.py --version).Trim(); if (-not $script:Version) { throw 'Version not found'} }
    Write-Info "Version: $script:Version"

    Invoke-Step 'Remove old artifacts (if any)' {
        if (Test-Path dist) { Remove-Item dist -Recurse -Force }
        if (Test-Path build) { Remove-Item build -Recurse -Force }
        Get-ChildItem -Filter '*.egg-info' -Directory -ErrorAction SilentlyContinue | ForEach-Object { Remove-Item $_.FullName -Recurse -Force }
        if (Test-Path .eggs) { Remove-Item .eggs -Recurse -Force }
    }
    Write-Ok 'Workspace clean'

    Invoke-Step 'Building sdist + wheel' { python setup.py sdist bdist_wheel }
    Write-Ok 'Build done'

    # Progress bar handling (avoid Rich Unicode issues on non-UTF8 consoles)
    if ($NoRich) {
        $env:TWINE_DISABLE_PROGRESS_BAR = '1'
        Write-Warn 'Progress bar disabled (NoRich)'
    }
    elseif (([Console]::OutputEncoding).CodePage -ne 65001) {
        $env:TWINE_DISABLE_PROGRESS_BAR = '1'
        Write-Warn 'Progress bar disabled (non-UTF8 console)'
    }

    Invoke-Step 'Uploading to PyPI' { twine upload dist/* }
    Write-Ok 'Upload done'

    if (-not $SkipClean) {
        Invoke-Step 'Cleaning build artifacts' {
            if (Test-Path dist) { Remove-Item dist -Recurse -Force }
            if (Test-Path build) { Remove-Item build -Recurse -Force }
            Get-ChildItem -Filter '*.egg-info' -Directory -ErrorAction SilentlyContinue | ForEach-Object { Remove-Item $_.FullName -Recurse -Force }
            if (Test-Path .eggs) { Remove-Item .eggs -Recurse -Force }
        }
        Write-Ok 'Clean done'
    }
    else {
        Write-Warn 'Skip clean (--SkipClean)'
    }
}
catch { Write-Err "Publish failed: $_"; exit 1 }
finally { $elapsed = (Get-Date) - $script:StartTime; Write-Info ("Elapsed: {0:N1} sec" -f $elapsed.TotalSeconds) }
