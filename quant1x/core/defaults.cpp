#include "defaults.h"
#include "base.h"

namespace quant1x {
namespace core {

// 特化为BaseConfig的默认值应用
void apply_defaults(BaseConfig& config) {
    // 对于BaseConfig，basedir和logdir如果为空，则设置默认值
    if (config.basedir.empty()) {
        config.basedir = get_base_path();
    }
    if (config.logdir.empty()) {
        config.logdir = config.basedir + "/logs";
    }
    // debug默认为false，已经在结构体定义中设置
    // 其他字段根据需要添加
}

} // namespace core
} // namespace quant1x