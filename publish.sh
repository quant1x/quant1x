#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# =============================================================
#  publish.sh  -  发布 Python 包到 PyPI
#
#  功能:
#    1. 构建 (sdist + wheel)
#    2. 上传到 PyPI (twine)
#    3. 清理临时构建产物 (dist, build, *.egg-info, .eggs)
#
#  使用:
#    bash publish.sh
#
#  可选参数:
#    --skip-clean    跳过清理临时目录
#    --no-rich       禁用进度条
#    --allow-existing 允许上传已存在的版本
#    -h, --help      显示帮助
#
#  先决条件:
#    - 已安装 python, pip, twine
#    - 已正确配置 PyPI 凭证 (例如环境变量 TWINE_USERNAME / TWINE_PASSWORD 或 keyring)
# =============================================================

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORIG_DIR="$(pwd)"
cd "$SCRIPT_DIR"

# -------- Encoding ---------
export PYTHONIOENCODING='utf-8'
export TWINE_NON_INTERACTIVE='1'

# -------- Logging ---------
if [[ -t 1 ]]; then
  C_INFO="\033[36m"; C_OK="\033[32m"; C_WARN="\033[33m"; C_ERR="\033[31m"; C_RESET="\033[0m"
else
  C_INFO=""; C_OK=""; C_WARN=""; C_ERR=""; C_RESET=""
fi
info() { echo -e "${C_INFO}[INFO ]${C_RESET} $*"; }
ok()   { echo -e "${C_OK}[ OK  ]${C_RESET} $*"; }
warn() { echo -e "${C_WARN}[WARN ]${C_RESET} $*"; }
err()  { echo -e "${C_ERR}[FAIL ]${C_RESET} $*" >&2; }

# -------- Params ----------
SKIP_CLEAN=0
NO_RICH=0
ALLOW_EXISTING=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-clean)     SKIP_CLEAN=1; shift;;
    --no-rich)        NO_RICH=1; shift;;
    --allow-existing) ALLOW_EXISTING=1; shift;;
    -h|--help)
      cat <<EOF
Usage: $0 [options]
  --skip-clean       跳过清理临时目录
  --no-rich          禁用进度条
  --allow-existing   允许上传已存在的版本
  -h, --help         显示帮助
EOF
      exit 0;;
    *) warn "Unknown arg: $1"; shift;;
  esac
done

START_EPOCH=$(date +%s)
trap 'err "Aborted"; cd "$ORIG_DIR"' INT TERM

# -------- Helper Functions ---------
need() { command -v "$1" >/dev/null 2>&1 || { err "Missing command: $1"; exit 1; }; }
step() { local title="$1"; shift; info "$title"; "$@"; local rc=$?; if [[ $rc -ne 0 ]]; then err "Step failed: $title (code=$rc)"; exit $rc; fi; }

# -------- Dependency Check ---------
info "Checking required commands..."
need python
need twine
need pip
info "Checking required Python modules..."
python -c "import build" 2>/dev/null || { err 'Missing Python module: build (install with pip install build)'; exit 1; }
ok "All commands ok"

# -------- Pre Clean ---------
step "Remove old artifacts (if any)" bash -c "
  rm -rf dist build .eggs
  for d in *.egg-info; do [[ -d \"\$d\" ]] && rm -rf \"\$d\"; done
  true
"
ok "Workspace clean"

# -------- Build ---------
step "Building sdist + wheel" python -m build --sdist --wheel
ok "Build done"

# -------- Version Read from Artifacts ---------
info "Reading version from built artifacts..."
PKG_VERSION=""
for f in dist/*; do
  # 包名不含横杠 (PEP 427: hyphens → underscores), 用 [^-]+ 避免 (.+) 贪婪匹配吃掉版本号中的横杠
  if [[ "$f" =~ ^dist/([^-]+)-(.+)\.(tar\.gz|zip|whl)$ ]]; then
    PKG_VERSION="${BASH_REMATCH[2]}"
    break
  fi
done
if [[ -z "${PKG_VERSION}" ]]; then err "Cannot determine version from built artifacts"; exit 1; fi
info "Version: ${PKG_VERSION}"

# -------- Existing Version Check (blocking) ---------
EXISTING=0
PYPI_JSON_URL="https://pypi.org/pypi/quant1x/json"
if command -v curl >/dev/null 2>&1; then
  if curl -fsS --connect-timeout 5 "$PYPI_JSON_URL" 2>/dev/null | grep -q "\"${PKG_VERSION}\""; then
    EXISTING=1
  fi
else
  warn "Skip version existence check (curl not available)"
fi

if [[ $EXISTING -eq 1 && $ALLOW_EXISTING -eq 0 ]]; then
  err "Version ${PKG_VERSION} already exists on PyPI. Use --allow-existing to force build/upload."
  exit 2
elif [[ $EXISTING -eq 1 && $ALLOW_EXISTING -eq 1 ]]; then
  warn "Version ${PKG_VERSION} exists; proceeding due to --allow-existing"
fi

# -------- Progress Bar Handling ---------
if [[ $NO_RICH -eq 1 ]]; then
  export TWINE_DISABLE_PROGRESS_BAR='1'
  warn "Progress bar disabled (--no-rich)"
elif [[ ! -t 1 ]]; then
  export TWINE_DISABLE_PROGRESS_BAR='1'
  warn "Progress bar disabled (non-TTY output)"
fi

# -------- Upload ---------
step "Uploading to PyPI" twine upload dist/*
ok "Upload done"

# -------- Post Clean ---------
if [[ $SKIP_CLEAN -eq 1 ]]; then
  warn "Skip clean (--skip-clean)"
else
  step "Cleaning build artifacts" bash -c "
    rm -rf dist build .eggs
    for d in *.egg-info; do [[ -d \"\$d\" ]] && rm -rf \"\$d\"; done
    true
  "
  ok "Clean done"
fi

# -------- Finish ---------
END_EPOCH=$(date +%s)
DUR=$((END_EPOCH-START_EPOCH))
info "Elapsed: ${DUR}s"
cd "$ORIG_DIR"
exit 0
