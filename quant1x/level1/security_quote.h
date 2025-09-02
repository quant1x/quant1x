#pragma once
#ifndef QUANT1X_LEVEL1_SECURITY_QUOTE_H
#define QUANT1X_LEVEL1_SECURITY_QUOTE_H 1

#include "protocol.h"
#include "encoding.h"
#include "quant1x/exchange/code.h"
#include "quant1x/exchange/session.h"

// ==============================
// 即时行情
// ==============================

namespace level1 {
    constexpr int security_quotes_max = 80;
#pragma pack(push, 1)  // 确保1字节对齐
    enum TradeState : u8 {
        DELISTING, ///< 终止上市
        NORMAL,    ///< 正常交易
        SUSPEND,   ///< 停牌
        IPO        ///< IPO阶段
    };

    struct Level {
        f64 price;
        i64 vol;
    };

    struct StockInfo {
        u8 market = 0;
        std::string code;
    };

    inline f64 defaultBaseUnit(int marketId, const char * const code) {
        (void) marketId; // 忽略未使用的参数，避免编译警告
        // 检查前两位字符
        if (strncmp(code, "60", 2) == 0 ||
            strncmp(code, "68", 2) == 0 ||
            strncmp(code, "00", 2) == 0 ||
            strncmp(code, "30", 2) == 0 ||
            strncmp(code, "39", 2) == 0) {
            return 100.0;
        } else if (strncmp(code, "510", 3) == 0) {
            // 检查前三位字符
            return 1000.0;
        } else {
            // 默认返回值
            return 100.0;
        }
    }

    /// 即时行情 - 请求
    struct SecurityQuoteRequest : public RequestHeader<SecurityQuoteRequest> {
        std::vector<u8> padding;
        std::vector<StockInfo> list;

        SecurityQuoteRequest(const std::vector<std::string> &codes) : RequestHeader<SecurityQuoteRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x01;
            Method = StdCommand::SECURITY_QUOTES_OLD;
            padding = strings::hexToBytes("0500000000000000");
            list.resize(0);
            for (auto const &securityCode: codes) {
                auto sc = strings::trim(securityCode);
                if (sc.empty()) {
                    continue;
                }
                auto [id, _, symbol] = exchange::DetectMarket(securityCode);
                StockInfo stockInfo{};
                stockInfo.market = u8(id);
                stockInfo.code   = symbol;
                list.emplace_back(stockInfo);
                //count++;
            }
        }

        // 编码
        std::vector<u8> serializeImpl() {
            auto count = list.size();
            PkgLen1 = 2 + u16(count * 7) + 10;
            PkgLen2 = PkgLen1;
            auto buf = RequestHeader<SecurityQuoteRequest>::headerSerialize();
            buf.insert(buf.end(), padding.begin(), padding.end());
            BinaryStream stream;
            stream.push_u16(u16(count));
            for (auto const &v: list) {
                stream.push_u8(v.market);
                stream.push_string(v.code);
            }
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        friend std::ostream &operator<<(std::ostream &os, const SecurityQuoteRequest &request) {
            os << request.toString();
            return os;
        }

        [[nodiscard]] std::string toString() const {
            std::ostringstream oss;
            oss << RequestHeader<SecurityQuoteRequest>::headerStringImpl()
                << ",{padding:"
                << strings::bytesToHex(padding)
                << '}';
            return oss.str();
        }
    };

    struct SecurityQuote {
        // 交易状态
        TradeState state;       ///< 0-终止上市 1-正常交易 2-停牌
        // 基础信息
        u8 market;              // 市场代码(0:深圳 1:上海 2:北交所)
        //char code[6];         // 证券代码(6位数字)
        std::string code;
        u16 active1;            // 活跃度指标(个股有效)

        // 价格信息
        f64 price;              // 现价
        f64 lastClose;          // 昨收价
        f64 open;               // 今开价
        f64 high;               // 最高价
        f64 low;                // 最低价

        // 时间信息
        std::string serverTime; // 服务器时间(HHMMSS格式)
        i64 reversedBytes0;     // 保留字段(时间相关)
        i64 reversedBytes1;     // 保留字段

        // 量能信息
        i64 vol;                // 总成交量(单位：手)
        i64 curVol;             // 当前成交量(个股-股数/板块-成交额)
        f64 amount;             // 总成交额(单位：元)
        i64 sVol;               // 内盘成交量(个股有效)
        i64 bVol;               // 外盘成交量(个股有效)

        // 集合竞价信息
        i64 indexOpenAmount;    // 指数开盘竞价金额(单位：元)
        i64 stockOpenAmount;    // 个股开盘竞价金额(单位：元)
        i64 openVolume;         // 开盘量(集合竞价成交量)
        i64 closeVolume;        // 收盘量(收盘集合竞价成交量)

        // 板块指数信息
        i64 indexUp;            // 上涨家数(板块指数有效)
        i64 indexUpLimit;       // 涨停家数(板块指数有效)
        i64 indexDown;          // 下跌家数(板块指数有效)
        i64 indexDownLimit;     // 跌停家数(板块指数有效)

        // 五档委买
        f64 bid1;               // 买一价
        f64 ask1;               // 卖一价
        i64 bidVol1;            // 买一量 / 上涨家数(板块指数时)
        i64 askVol1;            // 卖一量 / 下跌家数(板块指数时)

        f64 bid2;               // 买二价
        f64 ask2;               // 卖二价
        i64 bidVol2;            // 买二量 / 涨停家数(板块指数时)
        i64 askVol2;            // 卖二量 / 跌停家数(板块指数时)

        f64 bid3;               // 买三价
        f64 ask3;               // 卖三价
        i64 bidVol3;            // 买三量
        i64 askVol3;            // 卖三量

        f64 bid4;               // 买四价
        f64 ask4;               // 卖四价
        i64 bidVol4;            // 买四量
        i64 askVol4;            // 卖四量

        f64 bid5;               // 买五价
        f64 ask5;               // 卖五价
        i64 bidVol5;            // 买五量
        i64 askVol5;            // 卖五量

        // 保留字段
        u16 reversedBytes4;     // 保留字段
        i64 reversedBytes5;     // 保留字段
        i64 reversedBytes6;     // 保留字段
        i64 reversedBytes7;     // 保留字段
        i64 reversedBytes8;     // 保留字段

        // 动态指标
        f64 rate;               // 涨速(%/分钟)
        u16 active2;            // 活跃度指标2(个股与active1相同)

        // 时间戳
        std::string timeStamp;  // 本地时间戳(格式:YYYYMMDDHHMMSSmmm)

        friend std::ostream &operator<<(std::ostream &os, const SecurityQuote &quote) {
            std::cout << std::dec;
            os << "{state:" << magic_enum::enum_name(quote.state) << " market:" << (int) quote.market << " code:" << quote.code
               << " active1:"
               << quote.active1 << " price:" << quote.price << " lastClose:" << quote.lastClose << " open:"
               << quote.open << " high:" << quote.high << " low:" << quote.low << " serverTime:" << quote.serverTime
               << " reversedBytes0:" << quote.reversedBytes0 << " reversedBytes1:" << quote.reversedBytes1 << " vol:"
               << quote.vol << " curVol:" << quote.curVol << " amount:" << quote.amount << " sVol:" << quote.sVol
               << " bVol:" << quote.bVol << " indexOpenAmount:" << quote.indexOpenAmount << " stockOpenAmount:"
               << quote.stockOpenAmount << " openVolume:" << quote.openVolume << " closeVolume:" << quote.closeVolume
               << " indexUp:" << quote.indexUp << " indexUpLimit:" << quote.indexUpLimit << " indexDown:"
               << quote.indexDown << " indexDownLimit:" << quote.indexDownLimit << " bid1:" << quote.bid1 << " ask1:"
               << quote.ask1 << " bidVol1:" << quote.bidVol1 << " askVol1:" << quote.askVol1 << " bid2:"
               << quote.bid2 << " ask2:" << quote.ask2 << " bidVol2:" << quote.bidVol2 << " askVol2:"
               << quote.askVol2 << " bid3:" << quote.bid3 << " ask3:" << quote.ask3 << " bidVol3:" << quote.bidVol3
               << " askVol3:" << quote.askVol3 << " bid4:" << quote.bid4 << " ask4:" << quote.ask4 << " bidVol4:"
               << quote.bidVol4 << " askVol4:" << quote.askVol4 << " bid5:" << quote.bid5 << " ask5:" << quote.ask5
               << " bidVol5:" << quote.bidVol5 << " askVol5:" << quote.askVol5 << " reversedBytes4:"
               << quote.reversedBytes4 << " reversedBytes5:" << quote.reversedBytes5 << " reversedBytes6:"
               << quote.reversedBytes6 << " reversedBytes7:" << quote.reversedBytes7 << " reversedBytes8:"
               << quote.reversedBytes8 << " rate:" << quote.rate << " active2:" << quote.active2 << " timeStamp:"
               << quote.timeStamp << "}";
            return os;
        }
    };

    /// 即时行情 - 响应
    struct SecurityQuoteResponse : public ResponseHeader<SecurityQuoteResponse> {
        u16 count = 0;
        std::vector<SecurityQuote> list = {};
        //tsl::robin_map<std::string, StockInfo> mapCode;

        static f64 getPrice(f64 baseUnit, i64 price, i64 diff) {
            return f64(price + diff) / baseUnit;
        }

        void deserializeImpl(const std::vector<u8> &data) {
            auto now = exchange::timestamp::now();
            auto [_, status] = exchange::can_update_in_realtime(now);
            auto timestamp = now.toString();
            BinaryStream stream(data);
            stream.skip(2);
            count = stream.get_u16();
            list.reserve(count);
            for (int i = 0; i < count; ++i) {
                SecurityQuote ele = {};
                ele.market = stream.get_u8();
                ele.code = stream.get_string(6);
                f64 baseUnit = defaultBaseUnit(ele.market, ele.code.c_str());
                ele.active1 = stream.get_u16();

                i64 price_base = stream.varint_decode();
                ele.price = getPrice(baseUnit, price_base, 0);
                i64 tmp = stream.varint_decode();
                ele.lastClose = getPrice(baseUnit, price_base, tmp);
                ele.open = getPrice(baseUnit, price_base, stream.varint_decode());
                ele.high = getPrice(baseUnit, price_base, stream.varint_decode());
                ele.low = getPrice(baseUnit, price_base, stream.varint_decode());

                ele.reversedBytes0 = stream.varint_decode();
                if (ele.reversedBytes0 > 0) {
                    //ele.ServerTime = timeFromStr(fmt.Sprintf("%d",ele.ReversedBytes0))
                    ele.serverTime = encoding::format_time(ele.reversedBytes0);
                } else {
                    ele.serverTime = "0";
                    // 如果出现这种情况, 可能是退市或者其实交易状态异常的数据, 摘牌的情况下, 证券代码是错的
                    //ele.Code = proto.stock_delisting
                    // 证券代码可能部证券, 上海交易所的退市代码有机会填写成600839
                }
                ele.reversedBytes1 = stream.varint_decode();

                ele.vol = stream.varint_decode();
                ele.vol *= 100;
                ele.curVol = stream.varint_decode();
                u32 rawAmount = stream.get_u32();
                ele.amount = encoding::IntToFloat64(int(rawAmount));

                ele.sVol = stream.varint_decode();
                ele.bVol = stream.varint_decode();

                // 开盘金额需要 * 100
                ele.indexOpenAmount = stream.varint_decode() * 100;
                ele.stockOpenAmount = stream.varint_decode() * 100;

                // 确定当前数据是指数或者板块
                bool isIndexOrBlock = exchange::AssertIndexByMarketAndCode(static_cast<exchange::MarketType>(ele.market), ele.code);
                f64 tmpOpenVolume = 0.00f;
                if (isIndexOrBlock) {
                    // 指数或者板块, 单位是"股"
                    tmpOpenVolume = std::round(f64(ele.indexOpenAmount) / ele.open);
                } else {
                    // 个股, 单位是"股"
                    tmpOpenVolume = std::round(f64(ele.stockOpenAmount) / ele.open);
                }
                if (std::isnan(tmpOpenVolume)) {
                    tmpOpenVolume = 0.00;
                }
                ele.openVolume = i64(tmpOpenVolume);
                Level bidLevels[5] = {};
                Level askLevels[5] = {};
                for (int l = 0; l < 5; l++) {
                    auto bid = &bidLevels[l];
                    auto ask = &askLevels[l];
                    bid->price = getPrice(baseUnit, stream.varint_decode(), price_base);
                    ask->price = getPrice(baseUnit, stream.varint_decode(), price_base);
                    bid->vol = stream.varint_decode();
                    ask->vol = stream.varint_decode();
                }
                ele.bid1 = bidLevels[0].price;
                ele.bid2 = bidLevels[1].price;
                ele.bid3 = bidLevels[2].price;
                ele.bid4 = bidLevels[3].price;
                ele.bid5 = bidLevels[4].price;
                ele.ask1 = askLevels[0].price;
                ele.ask2 = askLevels[1].price;
                ele.ask3 = askLevels[2].price;
                ele.ask4 = askLevels[3].price;
                ele.ask5 = askLevels[4].price;

                ele.bidVol1 = bidLevels[0].vol;
                ele.bidVol2 = bidLevels[1].vol;
                ele.bidVol3 = bidLevels[2].vol;
                ele.bidVol4 = bidLevels[3].vol;
                ele.bidVol5 = bidLevels[4].vol;

                ele.askVol1 = askLevels[0].vol;
                ele.askVol2 = askLevels[1].vol;
                ele.askVol3 = askLevels[2].vol;
                ele.askVol4 = askLevels[3].vol;
                ele.askVol5 = askLevels[4].vol;

                ele.reversedBytes4 = stream.get_u16();
                ele.reversedBytes5 = stream.varint_decode();
                ele.reversedBytes6 = stream.varint_decode();
                ele.reversedBytes7 = stream.varint_decode();
                ele.reversedBytes8 = stream.varint_decode();

                i16 reversedBytes9 = stream.get_i16();
                ele.rate = f64(reversedBytes9) / 100.0;
                ele.active2 = stream.get_u16();

                // 交易状态判断
                if (ele.lastClose == f64(0) && ele.open == f64(0)) {
                    // 设置为退市状态, 先给个退市状态
                    ele.state = DELISTING;
                } else {
                    // 如果不是退市状态, 从临时映射中删除
                    std::string securityCode = exchange::GetMarketFlag(static_cast<exchange::MarketType>(ele.market)) + ele.code;
                    //delete(obj.mapCode, securityCode)
                    // 如果开盘价非0, 交易状态正常
                    if (ele.open != f64(0)) {
                        ele.state = NORMAL;
                    } else {
                        // 开盘价等于0, 停牌
                        ele.state = SUSPEND;
                    }
                }

                if (isIndexOrBlock) {
                    ele.indexUp = ele.bidVol1;
                    ele.indexDown = ele.askVol1;
                    ele.indexUpLimit = ele.bidVol2;
                    ele.indexUpLimit = ele.askVol2;
                }
                if (status == exchange::TimeStatus::ExchangeClosing) {
                    // 收盘
                    if (isIndexOrBlock) {
                        ele.closeVolume = i64(f64(ele.curVol * 100) / ele.price);
                    } else {
                        ele.closeVolume = ele.curVol * 100;
                    }
                }
                ele.timeStamp = timestamp;
                //obj.reply.List = append(obj.reply.List, *ele)
                //poolSecurityQuote.Release(ele)
                //std::cout << ele << std::endl;
                list.emplace_back(ele);
            }
        }

        // 修正退市的证券代码
        /**
         * @brief 验证退市状态: 1-修正证券代码, tdx的数据, 如果退市, 证券代码将可能是600839, 总之不是这次请求的证券代码, 需要修复
         *                    2-IPO后等待上市的状态和退市在数据上一致, 即昨日收盘和今天开盘都是0, 但是证券代码正常, 需要调整成非退市状态
         * @param code_maps
         */
        void verify_delisted_securities(tsl::robin_map<std::string, StockInfo> &code_maps) {
            if(code_maps.empty()) {
                return;
            }
            std::deque<int> remains;
            // 1. 第一遍, 剔除代码正常的记录
            //spdlog::warn("count = {},{}", count, list.size());
            for (int i = 0; !code_maps.empty() && i < count; ++i) {
                //spdlog::warn("process = {}/{}", i, count);
                auto &v = list[i];
                std::string securityCode = exchange::GetMarketFlag(static_cast<exchange::MarketType>(v.market)) + v.code;
                //spdlog::warn("check security code:{}", securityCode);
                if (v.state == DELISTING) {
                    // 查询在快照请求列表中的证券代码
                    auto it = code_maps.find(securityCode);
                    if (it != code_maps.end()) {
                        // 找到, 代码正常, 数据也应该是正常, 这是新股再IPO状态等待上市
                        v.state = IPO;
                        code_maps.erase(it);
                    } else {
                        // 未找到, 则数据不匹配, 将数据异常的快照记录的索引缓存起来
                        spdlog::error("security code:{}, not found, index={}", securityCode, i);
                        remains.emplace_back(i);
                    }
                } else {
                    // 数据正常
                    code_maps.erase(securityCode);
                }
            }
            // 2. 第二遍, 剩下都是数据异常的, 且无法对应
            if (remains.empty()) {
                return;
            }
            for (const auto &[key, value]: code_maps) {
                spdlog::error("ignore code:{}", key);
                int idx = remains.front();
                spdlog::error("idx = {}", idx);
                if (idx >= 0) {
                    remains.pop_front();
                    auto v = &list[idx];
                    v->market = value.market;
                    v->code = value.code;
                }
                if(remains.empty()) {
                    break;
                }
            }
            if(!remains.empty()) {
                spdlog::error("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
            }
            assert(remains.empty());
        }

        std::string toStringImpl() const {
            std::ostringstream out;
            out << "{count:" << count << " list: [";
            for (int i = 0; i < count; i++) {
                out << list[i];
            }
            out << "]}";
            return out.str();
        }

        friend std::ostream &operator<<(std::ostream &os, const SecurityQuoteResponse &response) {
            os << response.toStringImpl();
            return os;
        }
    };

#pragma pack(pop)  // 恢复默认对齐方式

}

#endif //QUANT1X_LEVEL1_SECURITY_QUOTE_H
