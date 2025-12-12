#include "base.h"
#include <quant1x/std/safe.h>
#include <quant1x/std/filepath.h>
#include <mutex>
#include <filesystem>
#include <cstdlib>
#include <vector>

namespace quant1x {
namespace core {

namespace {
    const std::string LANGUAGE = "cpp";
    const std::string DEFAULT_BASE_PATH = "~/.q1x-" + LANGUAGE;
    std::string quant1x_base_path;
    std::once_flag base_path_flag;
}  // namespace

static void LazyInitBasePath() {
    std::string path = filepath::expand_user(DEFAULT_BASE_PATH);
    if (path.empty()) {
        quant1x_base_path = DEFAULT_BASE_PATH;
    } else {
        quant1x_base_path = path;
    }
}

std::string GetBasePath() {
    std::call_once(base_path_flag, LazyInitBasePath);
    return quant1x_base_path;
}

std::string GetMetaPath() {
    std::filesystem::path base(GetBasePath());
    return (base / "meta").string();
}

} // namespace core
} // namespace quant1x
