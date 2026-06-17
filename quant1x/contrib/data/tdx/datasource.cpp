#include "datasource.h"

#include <algorithm>
#include <cstdio>
#include "instruments.h"
#include "sector.h"
#include <quant1x/std/strings.h>

namespace tdx {

    bool is_need_ignore(const std::string &code) {
        auto opt = instruments::get_instrument_info(code);
        if (!opt.has_value()) {
            return true;
        }

        // 对齐 Python: ignored_keywords = ["退", "摘牌"]
        static const std::vector<std::string> ignored_keywords = {"退", "摘牌"};

        std::string upper_name = strings::to_upper(opt->name);

        return std::any_of(ignored_keywords.begin(), ignored_keywords.end(),
                           [&upper_name](const std::string &kw) {
                               return upper_name.find(kw) != std::string::npos;
                           });
    }

    std::vector<meta::Instrument> list_instruments() {
        std::vector<meta::Instrument> code_list;

        // 对齐 Python _constants.ALL_INDEX_LIST
        static const std::vector<std::string> ALL_INDEX_LIST = {
            "sh000001", // 上证综合指数
            "sz399001", // 深证成份指数
            "bj899050", // 北证50指数
            "sz399006", // 创业板指
            "sh000016", // 上证50
            "sh000300", // 沪深300指数
            "sh000688", // 科创50指数
            "sh000905", // 中证500指数
            "sh000852", // 中证1000指数
            "sh880005", // 通达信板块-涨跌家数
            "sh510050", // 上证50ETF
            "sh510300", // 沪深300ETF
            "sh588000", // 科创50ETF
            "sh510500", // 中证500ETF
            "sh512100", // 中证1000ETF
            "sh510900", // H股ETF
            "sh518880", // 黄金ETF
            "sh512480", // 半导体ETF
            "sh562500", // 机器人ETF
        };

        // 1. 指数, 包括指数, 重要板块以及ETF
        for (const auto& code : ALL_INDEX_LIST) {
            auto opt = instruments::get_instrument_info(code);
            if (opt.has_value()) {
                code_list.push_back(opt.value());
            }
        }

        // 2. 板块 (对齐 Python sector.get_sector_list)
        for (const auto& s : tdx::sector::get_sector_list()) {
            if (std::find(ALL_INDEX_LIST.begin(), ALL_INDEX_LIST.end(), s.code) != ALL_INDEX_LIST.end()) {
                continue;
            }
            auto opt = instruments::get_instrument_info(s.code);
            if (opt.has_value()) {
                code_list.push_back(opt.value());
            }
        }

        // 3. 个股, 包括只包含上市公司股票 (对齐 Python get_stock_list)
        std::vector<std::string> all_codes;

        // 上海证券交易所 (sh600000-sh609999)
        for (int i = 600000; i < 610000; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "sh%06d", i);
            if (!is_need_ignore(buf)) {
                all_codes.emplace_back(buf);
            }
        }

        // 科创板 (sh688000-sh689999)
        for (int i = 688000; i < 690000; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "sh%06d", i);
            if (!is_need_ignore(buf)) {
                all_codes.emplace_back(buf);
            }
        }

        // 深圳主板 (sz000000-sz000999)
        for (int i = 0; i < 1000; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "sz%06d", i);
            if (!is_need_ignore(buf)) {
                all_codes.emplace_back(buf);
            }
        }

        // 中小板 (sz001000-sz009999)
        for (int i = 1000; i < 10000; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "sz%06d", i);
            if (!is_need_ignore(buf)) {
                all_codes.emplace_back(buf);
            }
        }

        // 创业板 (sz300000-sz300999)
        for (int i = 300000; i < 310000; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "sz%06d", i);
            if (!is_need_ignore(buf)) {
                all_codes.emplace_back(buf);
            }
        }

        // 北交所 (bj920000-bj920999)
        for (int i = 920000; i < 921000; ++i) {
            char buf[16];
            snprintf(buf, sizeof(buf), "bj%06d", i);
            if (!is_need_ignore(buf)) {
                all_codes.emplace_back(buf);
            }
        }

        for (const auto& code : all_codes) {
            auto opt = instruments::get_instrument_info(code);
            if (opt.has_value()) {
                code_list.push_back(opt.value());
            }
        }

        return code_list;
    }

} // namespace tdx
