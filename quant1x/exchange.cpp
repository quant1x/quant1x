#include <quant1x/exchange.h>

namespace exchange {

    /// 证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
    bool IsNeedIgnore(const std::string& code) {
        auto p = get_security_info(code);
        if (!p) {
            // 没找到直接忽略
            return true;
        }

        // 需要检查的关键字列表（静态常量，避免重复构造）
        static const std::array<std::string, 3> kIgnoredKeywords = {"ST", "退", "摘牌"};

        // 转换名称为大写（仅转换一次）
        std::string upper_name = strings::to_upper(p->name);

        // 使用算法检查是否存在任意关键字（短路求值）
        return std::any_of(
            kIgnoredKeywords.begin(), kIgnoredKeywords.end(),
            [&upper_name](const std::string& keyword) {
                return upper_name.find(keyword) != std::string::npos;
            }
        );
    }

    /// 获取证券代码列表, 过滤退市、摘牌和ST标记的个股
    std::vector<std::string> GetStockCodeList() {
        std::vector<std::string> allCodes = {};
        // 上海证券交易所 (sh600000-sh609999)
        {
            for (int i = 600000; i <= 609999; ++i) {
                std::string fc = std::format("sh{:06d}", i);
                if (!IsNeedIgnore(fc)) allCodes.emplace_back(fc);
            }
        }

        // 科创板 (sh688000-sh689999)
        {
            for (int i = 688000; i <= 689999; ++i) {
                std::string fc = std::format("sh{:06d}", i);
                if (!IsNeedIgnore(fc)) allCodes.emplace_back(fc);
            }
        }

        // 深圳主板 (sz000000-sz000999)
        {
            for (int i = 0; i <= 999; ++i) {
                std::string fc = std::format("sz{:06d}", i);
                if (!IsNeedIgnore(fc)) allCodes.emplace_back(fc);
            }
        }

        // 中小板 (sz001000-sz009999)
        {
            for (int i = 1000; i <= 9999; ++i) {
                std::string fc = std::format("sz{:06d}", i);
                if (!IsNeedIgnore(fc)) allCodes.emplace_back(fc);
            }
        }

        // 创业板 (sz300000-sz300999)
        {
            for (int i = 300000; i <= 309999; ++i) {
                std::string fc = std::format("sz{:06d}", i);
                if (!IsNeedIgnore(fc)) allCodes.emplace_back(fc);
            }
        }

        // 港股示例 (hk00001-hk09999)
        /*
        {
            for (int i = 1; i <= 9999; ++i) {
                std::string fc = std::format("hk{:0dd}", i);
                snprintf(buffer, sizeof(buffer), "hk%05d", i);
                allCodes.push_back(buffer);
            }
        }
        */

        return allCodes;
    }

    /// 加载全部指数、板块和个股的代码
    std::vector<std::string> GetCodeList() {
        std::vector<std::string> list;
        // 1. 指数
        list.insert(list.end(), AShareIndexList.begin(), AShareIndexList.end());
        // 2. 板块
        auto sectors = get_sector_list();
        for (const block_info & v : sectors) {
            list.emplace_back(v.code);
        }
        // 3. 个股, 包括场内开放式ETF基金
        auto stockCodeList = GetStockCodeList();
        list.insert(list.end(), stockCodeList.begin(), stockCodeList.end());
        return list;
    }

} // namespace exchange
