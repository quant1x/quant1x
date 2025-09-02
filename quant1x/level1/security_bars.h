#pragma once
#ifndef QUANT1X_LEVEL1_SECURITY_BARS_H
#define QUANT1X_LEVEL1_SECURITY_BARS_H 1

#include "protocol.h"

// ==============================
// K线
// ==============================

namespace level1 {

    constexpr int32_t security_bars_max = 800; // 单次最大获取800条K线数据
    // K线类型 (使用带底层类型的枚举)
    enum KLineType : u8 {
        _5MIN     = 0,   // 5分钟K线
        _15MIN    = 1,   // 15分钟K线
        _30MIN    = 2,   // 30分钟K线
        _1HOUR    = 3,   // 1小时K线
        DAILY     = 4,   // 日K线
        WEEKLY    = 5,   // 周K线
        MONTHLY   = 6,   // 月K线
        EXHQ_1MIN = 7,   // 扩展市场1分钟
        _1MIN     = 8,   // 普通1分钟K线
        RI_K      = 9,   // 日K线(同DAILY)
        _3MONTH   = 10,  // 季K线
        YEARLY    = 11   // 年K线
    };

    // K线类型转字符串
    inline const char *klineTypeToString(KLineType type) {
        switch (type) {
            case KLineType::_5MIN:
                return "5MIN";
            case KLineType::_15MIN:
                return "15MIN";
            case KLineType::_30MIN:
                return "30MIN";
            case KLineType::_1HOUR:
                return "1HOUR";
            case KLineType::DAILY:
                return "DAILY";
            case KLineType::WEEKLY:
                return "WEEKLY";
            case KLineType::MONTHLY:
                return "MONTHLY";
            case KLineType::EXHQ_1MIN:
                return "EXHQ_1MIN";
            case KLineType::_1MIN:
                return "1MIN";
            case KLineType::RI_K:
                return "RI_K";
            case KLineType::_3MONTH:
                return "3MONTH";
            case KLineType::YEARLY:
                return "YEARLY";
            default:
                return "UNKNOWN_KLINE";
        }
    }
#pragma pack(push, 1)  // 确保1字节对齐

    struct SecurityBarsParameter {
        u16 Market;
        char Code[6];
        u16 Category;  // 种类 5分钟 10分钟
        u16 I = 1;         // 未知 填充
        u16 Start;
        u16 Count;
    };

    struct SecurityBar {
        f64 Open;
        f64 Close;
        f64 High;
        f64 Low;
        f64 Vol;
        f64 Amount;
        int Year;
        int Month;
        int Day;
        int Hour;
        int Minute;
        std::string DateTime;
        u16 UpCount;   // 指数有效, 上涨家数
        u16 DownCount; // 指数有效, 下跌家数

        friend std::ostream &operator<<(std::ostream &os, const SecurityBar &bar) {
            os << "Open: " << bar.Open << " Close: " << bar.Close << " High: " << bar.High << " Low: " << bar.Low
               << " Vol: " << bar.Vol << " Amount: " << bar.Amount << " Year: " << bar.Year << " Month: " << bar.Month
               << " Day: " << bar.Day << " Hour: " << bar.Hour << " Minute: " << bar.Minute << " DateTime: "
               << bar.DateTime << " UpCount: " << bar.UpCount << " DownCount: " << bar.DownCount;
            return os;
        }
    };

    // K线 - 请求
    struct SecurityBarsRequest : public RequestHeader<SecurityBarsRequest> {
        SecurityBarsParameter param{};
        std::vector<u8> padding{};
        bool isIndex = false;

        SecurityBarsRequest(const std::string &securityCode, u16 category, u16 start, u16 count) : RequestHeader<SecurityBarsRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x00;
            Method = StdCommand::SECURITY_BARS;

            param.Category = category;
            param.I = 1;
            param.Start = start;
            param.Count = count;
            {
                auto [id, _, symbol] = exchange::DetectMarket(securityCode);
                param.Market = id;
                const char * const tmp = symbol.c_str();
                std::memcpy(param.Code, tmp, sizeof(param.Code));
                if(exchange::AssertIndexByMarketAndCode(id, symbol)) {
                    isIndex = true;
                }
            }

            padding = strings::hexToBytes("00000000000000000000");
        }

        std::vector<u8> serializeImpl() {
            PkgLen1 = u16(2 + sizeof(SecurityBarsParameter) + padding.size());
            PkgLen2 = PkgLen1;
            auto buf = RequestHeader<SecurityBarsRequest>::headerSerialize();
            BinaryStream bs;
            bs.push_arithmetic(param.Market);
            bs.push_array(param.Code);
            bs.push_arithmetic(param.Category);
            bs.push_arithmetic(param.I);
            bs.push_arithmetic(param.Start);
            bs.push_arithmetic(param.Count);
            auto &data = bs.data();
            buf.insert(buf.end(), data.begin(), data.end());
            buf.insert(buf.end(), padding.begin(), padding.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<SecurityBarsRequest>::headerStringImpl();
            oss << "{Market:" << int(param.Market)
                << ", Code:" << strings::from(param.Code)
                << ", Category:" << klineTypeToString(static_cast<KLineType>(param.Category))
                << ", I:" << int(param.I)
                << ", Start:" << int(param.Start)
                << ", Count:" << int(param.Count)
                << ", padding:" << strings::bytesToHex(padding)
                << "}";
            return oss.str();
        }
    };

    // K线 - 响应
    struct SecurityBarsResponse : public ResponseHeader<SecurityBarsResponse> {
        u16 Count;
        std::vector<SecurityBar> List;

        bool isIndex_;
        u16 category_;

        SecurityBarsResponse(bool isIndex, u16 category) : ResponseHeader<SecurityBarsResponse>() {
            Count = 0;
            List = {};

            isIndex_ = isIndex;
            category_ = category;
        }

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            i64 pre_diff_base = 0;
            for(int i = 0; i < Count; i++) {
                SecurityBar e{};
                int year = 0, month = 0, day = 0, hour = 15, minute = 0;
                if(category_ < 4 || category_ == 7 || category_ == 8){
                    u16 zipday = 0, tminutes = 0;
                    zipday = bs.get_u16();
                    tminutes = bs.get_u16();

                    year = int((zipday >> 11) + 2004);
                    month = int((zipday % 2048) / 100);
                    day = int((zipday % 2048) % 100);
                    hour = int(tminutes / 60);
                    minute = int(tminutes % 60);
                } else {
                    u32 zipday = bs.get_u32();
                    year = int(zipday / 10000);
                    month = int((zipday % 10000) / 100);
                    day = int(zipday % 100);
                }
                e.Year = year;
                e.Month = month;
                e.Day = day;
                e.Hour = hour;
                e.Minute = minute;
                e.DateTime = fmt::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:00", e.Year, e.Month, e.Day, e.Hour, e.Minute);

                auto price_open_diff = bs.varint_decode();
                auto price_close_diff = bs.varint_decode();
                auto price_high_diff = bs.varint_decode();
                auto price_low_diff = bs.varint_decode();

                u32 ivol = bs.get_u32();
                e.Vol = encoding::IntToFloat64(ivol);

                u32 dbvol = bs.get_u32();
                e.Amount = encoding::IntToFloat64(i64(dbvol));

                e.Open = f64(price_open_diff+pre_diff_base) / 1000.0;
                price_open_diff += pre_diff_base;

                e.Close = f64(price_open_diff+price_close_diff) / 1000.0;
                e.High = f64(price_open_diff+price_high_diff) / 1000.0;
                e.Low = f64(price_open_diff+price_low_diff) / 1000.0;

                pre_diff_base = price_open_diff + price_close_diff;

                if (isIndex_) {
                    e.UpCount = bs.get_u16();
                    e.DownCount = bs.get_u16();
                }
                List.emplace_back(e);
            }
        }

        std::string toString() const {
            std::ostringstream oss;
            oss << ResponseHeader<SecurityBarsResponse>::headerStringImpl();
            oss << "{Count:" << int(Count);
            oss << ", List:[";
            for(int i = 0; i < Count; i++) {
                oss << "{" << List[i] << "}";
            }
            oss << "]}";
            return oss.str();
        }
    };

#pragma pack(pop)  // 恢复默认对齐方式

}
#endif //QUANT1X_LEVEL1_SECURITY_BARS_H
