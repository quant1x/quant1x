#include <quant1x/instruments/markets.h>

namespace instruments {

    // A股指数列表
    static const std::vector<std::string> AShareIndexList = {
        "sh000001",  // 上证综合指数
        "sh000002",  // 上证A股指数
        "sh000300",  // 沪深300指数
        "sh000688",  // 科创50指数
        "sh000905",  // 中证500指数
        "sz399001",  // 深证成份指数
        "sz399006",  // 创业板指
        "sz399107",  // 深证A指
        "bj899050",  // 北证50指数
        "sh880005",  // 通达信板块-涨跌家数
        "sh510050",  // 上证50ETF
        "sh510300",  // 沪深300ETF
        "sh510900",  // H股ETF
    };

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

        // 北交所 (bj920000-bj920999)
        {
            for (int i = 920000; i <= 920999; ++i) {
                std::string fc = std::format("bj{:06d}", i);
                if (!IsNeedIgnore(fc)) allCodes.emplace_back(fc);
            }
        }
        
        return allCodes;
    }

    /// 加载全部指数、板块和个股的代码
    std::vector<std::string> GetCodeList() {
        std::vector<std::string> list;
        // 1. 指数
        list.insert(list.end(), AShareIndexList.begin(), AShareIndexList.end());
        // 2. 板块
        auto sectors = exchange::get_sector_list();
        for (const exchange::block_info & v : sectors) {
            list.emplace_back(v.code);
        }
        // 3. 个股, 包括场内开放式ETF基金
        auto stockCodeList = GetStockCodeList();
        list.insert(list.end(), stockCodeList.begin(), stockCodeList.end());
        return list;
    }

} // namespace instruments
