#pragma once
#ifndef QUANT1X_CORE_DECODE_H
#define QUANT1X_CORE_DECODE_H

#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <any>

namespace quant1x {
namespace core {

// DecodeTo 将源数据解码到目标结构体
// 与Go版本的DecodeTo保持一致
bool decode_to(void* dst, const std::any& src);

// LookupConfig 从配置map中按路径查找值
// 与Go版本的LookupConfig保持一致
std::any lookup_config(const std::string& path);

// DecodeConfig 从配置中解码指定路径到目标结构体, 并应用默认值
// 与Go版本的DecodeConfig保持一致
template<typename T>
bool decode_config(const std::string& path, T& dst) {
    auto val = lookup_config(path);
    if (!decode_to(&dst, val)) {
        return false;
    }
    apply_defaults(dst);
    return true;
}

} // namespace core
} // namespace quant1x

#endif // QUANT1X_CORE_DECODE_H