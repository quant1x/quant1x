#pragma once

#include <quant1x/exchange/code.h>

namespace exchange {

/**
 * @brief 检测并解析证券代码
 *
 * 该函数用于识别和解析输入的证券代码字符串，确定其所属交易所、证券类型等信息。
 * 支持多种格式的证券代码表示法，包括前缀形式(sh600000)、后缀形式(600000.sh)以及纯数字形式。
 *
 * @param input 输入的证券代码字符串
 * @return SecurityCode 包含交易所ID、证券代码和证券类型的结构体
 *
 * @note 处理流程:
 * 1. 首先对输入字符串进行标准化处理(去除空格并转为小写)
 * 2. 尝试通过显式市场标志(前缀或后缀)识别交易所
 * 3. 如果没有显式标志，则根据纯数字代码推断交易所
 * 4. 处理4字母全大写的美股代码
 * 5. 应用全局规则和特定市场规则确定证券类型
 *
 * @throws 无显式抛出异常，但可能返回包含未知值的SecurityCode结构体
 */
SecurityCode detect(const std::string& input);

} // namespace exchange
