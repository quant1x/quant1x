//============================================================
// exchange 证券代码相关                                      //
//============================================================
#include <quant1x/data/exchange/code.h>
#include <quant1x/std/api.h>
#include <vector>

namespace exchange {

    /**
     * @brief 根据市场类型和代码生成完整证券代码
     * @param market 市场类型
     * @param symbol 原始代码
     * @return 完整证券代码（格式：市场标识+代码）
     */
    std::string GetSecurityCode(ExchangeId market, const std::string &symbol) {
        switch (market) {
            case ExchangeId::USA:
                return ExchangeUS.String() + symbol;
            case ExchangeId::HongKong:
                return ExchangeHKEX.String() + symbol.substr(0, 5);
            case ExchangeId::BeiJing:
                return ExchangeBSE.String() + symbol.substr(0, 6);
            case ExchangeId::ShenZhen:
                return ExchangeSZSE.String() + symbol.substr(0, 6);
            default:
                return ExchangeSSE.String() + symbol.substr(0, 6);
        }
    }

    // 上海证券交易所
    // 主板: 60xxxx
    // 科创板: 688xxx
    // B股: 900xxx
    // 优先股: 360xxx
    // 科创板存托凭证: 689xxx
    // 申购/配股/投票: 7xxxxx
    // 上海总规则: https://zhuanlan.zhihu.com/p/719045287
    // 0: 国债/指数, 000 上证指数系列和中证指数系列, 00068x科创板指数
    // 1: 债券
    // 2: 回购
    // 3: 期货
    // 4: 备用
    // 5: 基金/权证
    // 6: A股
    // 7: 非交易业务(发行, 权益分配)
    // 8: 备用, 通达信编制板块指数占用880,881
    // 9: B股
    static const std::vector<std::string> shanghaiMainBoardPrefixes = {"50", "51", "60", "68", "90", "110", "113", "132", "204"};
    static const std::vector<std::string> shanghaiSpecialPrefixes = {"5", "6", "9", "7"};
    // 板块指数: 880,881
    static const std::vector<std::string> sectorPrefixes = {"880", "881"};
    // 深圳交易所
    // 主板: 000,001
    // 中小板: 002,003,004
    // 创业板: 30xxxx
    // 优先股: 140xxx
    // 深圳总规则: https://zhuanlan.zhihu.com/p/63064991
    // 0: 股票
    // 1: 国债/基金
    // 2: B股
    // 30: 创业板
    // 36: 投票, 369999用于深交所认证业务的密码激活/密码挂失
    // 37: 增发/可转债申购
    // 38: 配股/可转债优先权
    // 395: 成交量统计指数
    // 399: 指数
    static const std::vector<std::string> shenzhenMainBoardPrefixes = {"00", "12", "13", "18", "15", "16", "18", "20", "30", "39",
                                                                       "115", "1318"};
    // 北京交易所证券代码段
    // 关于发布《北京证券交易所 全国中小企业股份转让系统证券代码、证券简称编制指引》的公告 https://www.bse.cn/jygl_list/200021626.html
	// 北交所指数: 899
	// 新三板: 40,43,83,87
	// 88开头: 通常表示公开发行的股票, 与新三板市场中的其他类型股票进行区分
	// 三板A: 400,430,830-839,870-873
	// 三板B: 420
	// 优先股: 820
	// 新代码段: 920
    static const std::vector<std::string> beijingMainBoardPrefixes = {"40", "43", "83", "87", "88", "420", "820", "899", "920"};

    /**
     * @brief 根据代码判断所属市场
     * @param symbol 证券代码
     * @return 市场标识（sh/sz/bj等）
     */
    std::string GetMarket(const std::string &symbol) {
        std::string code = strings::trim(symbol);
        std::string market = ExchangeSSE.String();

        if (strings::startsWith(code, marketFlags)) {
            market = code.substr(0, 2);
            market = strings::to_lower(market);
        } else if (strings::endsWith(code, marketFlags)) {
            size_t len = code.size();
            market = code.substr(len - 2);
            market = strings::to_lower(market);
        } else if (strings::startsWith(code, shanghaiMainBoardPrefixes)) {
            market = ExchangeSSE.String();
        } else if (strings::startsWith(code, shenzhenMainBoardPrefixes)) {
            market = ExchangeSZSE.String();
        } else if (strings::startsWith(code, shanghaiSpecialPrefixes)) {
            market = ExchangeSSE.String();
        } else if (strings::startsWith(code, sectorPrefixes)) {
            market = ExchangeSSE.String();
        } else if (strings::startsWith(code, beijingMainBoardPrefixes)) {
            market = ExchangeBSE.String();
        }
        return market;
    }

    /**
     * @brief 获取市场ID
     * @param symbol 证券代码
     * @return 市场类型枚举值
     */
    ExchangeId GetMarketId(const std::string &symbol) {
        std::string market = GetMarket(symbol);
        if (market == ExchangeSSE.String()) return ExchangeId::ShangHai;
        if (market == ExchangeSZSE.String()) return ExchangeId::ShenZhen;
        if (market == ExchangeBSE.String()) return ExchangeId::BeiJing;
        return ExchangeId::ShangHai;
    }

    /**
     * @brief 根据市场ID获取市场标识
     * @param marketId 市场类型枚举
     * @return 市场标识字符串
     */
    std::string GetMarketFlag(ExchangeId marketId) {
        switch (marketId) {
            case ExchangeId::ShenZhen:
                return ExchangeSZSE.String();
            case ExchangeId::BeiJing:
                return ExchangeBSE.String();
            case ExchangeId::HongKong:
                return ExchangeHKEX.String();
            case ExchangeId::USA:
                return ExchangeUS.String();
            default:
                return ExchangeSSE.String();
        }
    }

    /**
     * @brief 综合解析证券代码
     * @param symbol 原始证券代码
     * @return 元组（市场ID，市场标识，纯代码）
     */
    std::tuple<ExchangeId, std::string, std::string> DetectMarket(const std::string &symbol) {
        std::string pureCode = strings::trim(symbol);
        std::string marketCode = ExchangeSSE.String();

        if (strings::startsWith(pureCode, marketFlags)) {
            marketCode = pureCode.substr(0, 2);
            //std::transform(marketCode.begin(), marketCode.end(), marketCode.begin(), ::tolower);
            marketCode = strings::to_lower(marketCode);
            pureCode = (pureCode[2] == '.') ? pureCode.substr(3) : pureCode.substr(2);
        } else if (strings::endsWith(pureCode, marketFlags)) {
            size_t len = pureCode.size();
            marketCode = pureCode.substr(len - 2);
            //std::transform(marketCode.begin(), marketCode.end(), marketCode.begin(), ::tolower);
            marketCode = strings::to_lower(marketCode);
            pureCode = pureCode.substr(0, len - 3);
        } else if (strings::startsWith(pureCode, shanghaiMainBoardPrefixes)) {
            marketCode = ExchangeSSE.String();
        } else if (strings::startsWith(pureCode, shenzhenMainBoardPrefixes)) {
            marketCode = ExchangeSZSE.String();
        } else if (strings::startsWith(pureCode, shanghaiSpecialPrefixes)) {
            marketCode = ExchangeSSE.String();
        } else if (strings::startsWith(pureCode, sectorPrefixes)) {
            marketCode = ExchangeSSE.String();
        } else if (strings::startsWith(pureCode, beijingMainBoardPrefixes)) {
            marketCode = ExchangeBSE.String();
        }

        ExchangeId marketId = ExchangeId::ShangHai;
        if (marketCode == ExchangeSSE.String()) marketId = ExchangeId::ShangHai;
        else if (marketCode == ExchangeSZSE.String()) marketId = ExchangeId::ShenZhen;
        else if (marketCode == ExchangeBSE.String()) marketId = ExchangeId::BeiJing;
        else if (marketCode == ExchangeHKEX.String()) marketId = ExchangeId::HongKong;

        return {marketId, marketCode, pureCode};
    }

    /**
     * @brief 判断是否为指数代码（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为指数
     */
    bool AssertIndexByMarketAndCode(ExchangeId marketId, const std::string &symbol) {
        // 上交所指数: 000, 880, 881
        if (marketId == ExchangeId::ShangHai && strings::startsWith(symbol, {"000", "880", "881"})) {
            return true;
        }
        // 深交所指数: 399
        if (marketId == ExchangeId::ShenZhen && strings::startsWith(symbol, {"399"})) {
            return true;
        }
        // 北交所指数: 899
        if (marketId == ExchangeId::BeiJing && strings::startsWith(symbol, {"899"})) {
            return true;
        }
        return false;
    }

    /**
     * @brief 判断是否为指数代码（通过完整证券代码）
     * @param securityCode 完整证券代码
     * @return 是否为指数
     */
    bool AssertIndexBySecurityCode(const std::string &securityCode) {
        auto [marketId, _, code] = DetectMarket(securityCode);
        return AssertIndexByMarketAndCode(marketId, code);
    }

    /**
     * @brief 判断并修正板块代码
     * @param securityCode 完整证券代码（会被修改）
     * @return 是否为板块代码
     */
    bool AssertBlockBySecurityCode(std::string *securityCode) {
        auto [marketId, flag, code] = DetectMarket(*securityCode);
        if (marketId != ExchangeId::ShangHai || !strings::startsWith(code, sectorPrefixes)) return false;
        *securityCode = flag + code;
        return true;
    }

    /**
     * @brief 判断是否为ETF（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为ETF
     */
    bool AssertETFByMarketAndCode(ExchangeId marketId, const std::string &symbol) {
        return marketId == ExchangeId::ShangHai && strings::startsWith(symbol, {"510"});
    }

    /**
     * @brief 判断是否为个股（通过市场ID和纯代码）
     * @param marketId 市场ID
     * @param symbol 纯代码
     * @return 是否为个股
     */
    bool AssertStockByMarketAndCode(ExchangeId marketId, const std::string &symbol) {
        if (marketId == ExchangeId::ShangHai && strings::startsWith(symbol, {"60", "68", "510"})) {
            return true;
        }
        if (marketId == ExchangeId::ShenZhen && strings::startsWith(symbol, {"00", "30"})) {
            return true;
        }
        if (marketId == ExchangeId::BeiJing && strings::startsWith(symbol, {"40", "43", "83", "87", "88", "420", "820","920"})) {
            return true;
        }
        return false;
    }

    /**
     * @brief 判断是否为个股（通过完整证券代码）
     * @param securityCode 完整证券代码
     * @return 是否为个股
     */
    bool AssertStockBySecurityCode(const std::string &securityCode) {
        auto [marketId, _, code] = DetectMarket(securityCode);
        return AssertStockByMarketAndCode(marketId, code);
    }

    /**
     * @brief 修正证券代码格式
     * @param symbol 原始代码
     * @return 标准化后的证券代码
     */
    std::string CorrectSecurityCode(const std::string &symbol) {
        if (symbol.empty()) return "";
        auto [_, mFlag, mSymbol] = DetectMarket(symbol);
        return mFlag + mSymbol;
    }

    /**
     * @brief 判断证券代码类型
     * @param securityCode 完整证券代码
     * @return 证券类型枚举
     */
    TargetKind AssertCode(const std::string &securityCode) {
        auto [marketId, _, code] = DetectMarket(securityCode);
        if (marketId == ExchangeId::ShangHai) {
            if (strings::startsWith(code, sectorPrefixes)) return TargetKind::BLOCK;
            if (strings::startsWith(code, {"000"})) return TargetKind::INDEX;
            if (strings::startsWith(code, {"5"})) return TargetKind::ETF;
        }
        if (marketId == ExchangeId::ShenZhen) {
            if (strings::startsWith(code, {"399"}))  return TargetKind::INDEX;
            if (strings::startsWith(code, {"159"})) return TargetKind::ETF;
        }
        if (marketId == ExchangeId::BeiJing && strings::startsWith(code, {"899"})) {
            return TargetKind::INDEX;
        }
        return TargetKind::STOCK;
    }

    // 检查指数和个股
    bool checkIndexAndStock(const std::string &securityCode) {
        if (AssertIndexBySecurityCode(securityCode)) {
            return true;
        }
        if (AssertStockBySecurityCode(securityCode)) {
            return true;
        }
        return false;
    }
}


namespace exchange {

ExchangeId ExchangeCode::Id() const {
    if (value == std::string_view("sz")) return ExchangeId::ShenZhen;
    if (value == std::string_view("sh")) return ExchangeId::ShangHai;
    if (value == std::string_view("bj")) return ExchangeId::BeiJing;
    if (value == std::string_view("hk")) return ExchangeId::HongKong;
    if (value == std::string_view("us")) return ExchangeId::USA;
    return ExchangeId::Unknown;
}

std::string String(ExchangeId m) {
    switch (m) {
    case ExchangeId::ShenZhen: return std::string(ExchangeSZSE.String());
    case ExchangeId::ShangHai: return std::string(ExchangeSSE.String());
    case ExchangeId::BeiJing: return std::string(ExchangeBSE.String());
    case ExchangeId::HongKong: return std::string(ExchangeHKEX.String());
    case ExchangeId::USA: return std::string(ExchangeUS.String());
    default: throw std::runtime_error("unknown market id");
    }
}

std::string ExchangeInfo::ToString() const {
    return fmt::format("{}({})", name, code);
}

void ExchangeInfo::Validate() const {
    if (code.empty()) throw std::invalid_argument("exchange code cannot be empty");
    if (name.empty()) throw std::invalid_argument("exchange name cannot be empty");
}

ExchangeInfo ExchangeInfo::NewExchange(const std::string& code,
                                       const std::string& name,
                                       const std::string& desc,
                                       ExchangeId id) {
    ExchangeInfo e;
    e.code = code;
    e.name = name;
    e.description = desc;
    e.id = id;
    e.is_active = true;
    return e;
}

std::string SecurityCode::ToString() const {
    return fmt::format("{}{}", String(market), symbol);
}

void SecurityCode::Validate() const {
    if (symbol.empty()) throw std::invalid_argument("security code symbol cannot be empty");
}

} // namespace exchange
