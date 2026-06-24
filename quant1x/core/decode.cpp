#include "decode.h"
#include "defaults.h"
#include <yaml-cpp/yaml.h>
#include <sstream>

namespace quant1x {
namespace core {

// 辅助函数: 递归查找map中的路径
YAML::Node lookup_map(const YAML::Node& node, const std::string& path) {
    std::istringstream iss(path);
    std::string token;
    YAML::Node current = node;
    while (std::getline(iss, token, '.')) {
        if (current.IsMap() && current[token]) {
            current = current[token];
        } else {
            return YAML::Node(); // 返回空节点表示未找到
        }
    }
    return current;
}

// LookupConfig 从配置map中按路径查找值
std::any lookup_config(const std::string& path) {
    const auto& config_map = get_config_map_ref();
    // 将unordered_map转换为YAML::Node进行查找
    YAML::Node root;
    for (const auto& pair : config_map) {
        root[pair.first] = pair.second;
    }
    YAML::Node result = lookup_map(root, path);
    if (result.IsDefined()) {
        // 转换为std::any, 这里简化, 返回YAML::Node的字符串表示或其他
        // 实际上, 需要根据类型转换
        if (result.IsScalar()) {
            return result.as<std::string>();
        } else if (result.IsMap()) {
            // 返回整个子map, 但这里简化
            return result;
        }
        // 其他类型类似
    }
    return std::any();
}

// DecodeTo 将源数据解码到目标结构体
bool decode_to(void* dst, const std::any& src) {
    // 这里简化实现, 假设src是YAML::Node或string
    // 实际需要根据dst类型进行反序列化
    // 使用yaml-cpp进行转换
    try {
        if (src.type() == typeid(YAML::Node)) {
            YAML::Node node = std::any_cast<YAML::Node>(src);
            // 假设dst是指向BaseConfig或其他结构体的指针
            // 这里需要模板或类型检查, 但C++没有反射
            // 简化: 假设是BaseConfig
            BaseConfig* config = static_cast<BaseConfig*>(dst);
            *config = node.as<BaseConfig>();
            return true;
        } else if (src.type() == typeid(std::string)) {
            std::string str = std::any_cast<std::string>(src);
            YAML::Node node = YAML::Load(str);
            BaseConfig* config = static_cast<BaseConfig*>(dst);
            *config = node.as<BaseConfig>();
            return true;
        }
    } catch (const YAML::Exception& e) {
        // 错误处理
        return false;
    }
    return false;
}

} // namespace core
} // namespace quant1x