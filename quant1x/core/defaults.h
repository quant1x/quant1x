#pragma once
#ifndef QUANT1X_CORE_DEFAULTS_H
#define QUANT1X_CORE_DEFAULTS_H

#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace quant1x {
namespace core {

// Defaultable CRTP模板, 要求派生类实现do_apply_defaults方法
template<typename Derived>
class Defaultable {
public:
    void apply_defaults() {
        static_cast<Derived*>(this)->do_apply_defaults();
    }
protected:
    // 纯虚函数, 要求派生类实现
    virtual void do_apply_defaults() = 0;
};

// 全局apply_defaults函数, 调用对象的apply_defaults方法
template<typename T>
void apply_defaults(T& target) {
    target.apply_defaults();
}

} // namespace core
} // namespace quant1x

#endif // QUANT1X_CORE_DEFAULTS_H