#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# =============================================================
#  publish.sh  -  Publish Python package to PyPI
#  Features:
#    * Build (sdist + wheel)
#    * Upload via twine
#    * Parameters: --skip-clean --dry-run --repository <name>
#    * Dependency checks (python, twine, git)
#    * Version duplication check (PyPI JSON) (best effort)
#    * Colored logs (auto disable if not TTY)
# =============================================================

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORIG_DIR="$(pwd)"
cd "$SCRIPT_DIR"

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
DRY_RUN=0
REPOSITORY="pypi"   # can be testpypi

while [[ $# -gt 0 ]]; do
  case "$1" in
    --skip-clean) SKIP_CLEAN=1; shift;;
    --dry-run)    DRY_RUN=1; shift;;
    --repository) REPOSITORY="$2"; shift 2;;
    -h|--help)
      cat <<EOF
Usage: $0 [options]
  --skip-clean        Do not remove dist/build after upload
  --dry-run           Build only (no upload)
  --repository <name> Use custom repository (default: pypi)
  -h, --help          Show this help
EOF
      exit 0;;
    *) warn "Unknown arg: $1"; shift;;
  esac
done

START_EPOCH=$(date +%s)
trap 'err "Aborted"; cd "$ORIG_DIR"' INT TERM

# -------- Dependency Check ---------
need() { command -v "$1" >/dev/null 2>&1 || { err "Missing command: $1"; exit 1; }; }
need python
if [[ $DRY_RUN -eq 0 ]]; then need twine; fi
ok "Dependencies ok"

# -------- Version Read ---------
info "Reading version..."
PKG_VERSION=$(python setup.py --version 2>/dev/null | tr -d '\r') || true
if [[ -z "${PKG_VERSION}" ]]; then err "Cannot read version"; exit 1; fi
info "Version: ${PKG_VERSION}"

# -------- Existing Version Check (best effort) ---------
if [[ $DRY_RUN -eq 0 ]]; then
  PYPI_JSON_URL="https://pypi.org/pypi/quant1x/json"
  if command -v curl >/dev/null 2>&1; then
    if curl -fsS "$PYPI_JSON_URL" | grep -q "\"${PKG_VERSION}\""; then
      warn "Version ${PKG_VERSION} already appears on PyPI (continuing; upload may fail)"
    fi
  fi
fi

# -------- Pre Clean ---------
info "Cleaning previous artifacts..."
rm -rf dist build .eggs *.egg-info
ok "Workspace clean"

# -------- Build ---------
info "Building distributions..."
python setup.py sdist bdist_wheel
ok "Build done"

# -------- Dry Run Short Circuit ---------
if [[ $DRY_RUN -eq 1 ]]; then
  warn "Dry run: skip upload"
  cd "$ORIG_DIR"; exit 0
fi

# -------- Upload ---------
UPLOAD_ARGS=(dist/*)
if [[ "$REPOSITORY" != "pypi" ]]; then
  info "Using repository: $REPOSITORY"
  UPLOAD_ARGS=(--repository "$REPOSITORY" dist/*)
fi

info "Uploading to ${REPOSITORY}..."
if ! twine upload "${UPLOAD_ARGS[@]}"; then
  err "Upload failed"; cd "$ORIG_DIR"; exit 1
fi
ok "Upload done"

# -------- Post Clean ---------
if [[ $SKIP_CLEAN -eq 1 ]]; then
  warn "Skip clean (--skip-clean)"
else
  info "Removing build artifacts..."
  rm -rf dist build .eggs *.egg-info
  ok "Clean done"
fi

# -------- Finish ---------
END_EPOCH=$(date +%s)
DUR=$((END_EPOCH-START_EPOCH))
info "Elapsed: ${DUR}s"
cd "$ORIG_DIR"
exit 0
