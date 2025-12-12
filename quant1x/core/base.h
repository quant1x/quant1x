#pragma once
#ifndef QUANT1X_CORE_BASE_H
#define QUANT1X_CORE_BASE_H

#include <string>

namespace quant1x {
namespace core {

// 返回默认的基础路径，如果无法展开用户目录则返回默认路径
std::string GetBasePath();

// 返回元数据存储的基础路径
// meta目录位于基础路径下的meta子目录中
std::string GetMetaPath();

} // namespace core
} // namespace quant1x

#endif // QUANT1X_CORE_BASE_H
