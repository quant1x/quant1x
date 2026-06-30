#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_SECURITY_BARS_H
#define QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_SECURITY_BARS_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/data/meta/instrument.h>
#include <stdexcept>
#include <quant1x/contrib/data/tdx/helpers.h>

// ==============================
// K线
// ==============================

namespace quant1x::contrib::data::tdx {

    constexpr int32_t security_bars_max = 800; // 单次最大获取800条K线数据
    // K线类型 (使用带底层类型的枚举)
    enum BarFreq : u8 {
        Freq5Min     = 0,   // 5分钟K线
        Freq15Min    = 1,   // 15分钟K线
        Freq30Min    = 2,   // 30分钟K线
        Freq1Hour    = 3,   // 1小时K线
        FreqDaily    = 4,   // 日K线
        FreqWeekly   = 5,   // 周K线
        FreqMonthly  = 6,   // 月K线
        FreqExHQ1Min = 7,   // 扩展市场1分钟
        Freq1Min     = 8,   // 普通1分钟K线
        FreqRIK      = 9,   // 日K线(同DAILY)
        Freq3Month   = 10,  // 季K线
        FreqYearly   = 11   // 年K线
    };

    // K线类型转字符串
    inline const char *klineTypeToString(BarFreq type) {
        switch (type) {
            case BarFreq::Freq5Min:
                return "5MIN";
            case BarFreq::Freq15Min:
                return "15MIN";
            case BarFreq::Freq30Min:
                return "30MIN";
            case BarFreq::Freq1Hour:
                return "1HOUR";
            case BarFreq::FreqDaily:
                return "DAILY";
            case BarFreq::FreqWeekly:
                return "WEEKLY";
            case BarFreq::FreqMonthly:
                return "MONTHLY";
            case BarFreq::FreqExHQ1Min:
                return "EXHQ_1MIN";
            case BarFreq::Freq1Min:
                return "1MIN";
            case BarFreq::FreqRIK:
                return "RI_K";
            case BarFreq::Freq3Month:
                return "3MONTH";
            case BarFreq::FreqYearly:
                return "YEARLY";
            default:
                return "UNKNOWN_KLINE";
        }
    }
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

    // K线 (对齐 Python SecurityBarsContext)
    struct SecurityBarsContext : public BaseFrame<SecurityBarsContext> {
        SecurityBarsParameter param{};
        std::vector<u8> padding{};
        bool isIndex = false;

        u16 Count;
        std::vector<SecurityBar> List;

        u16 category_;

        SecurityBarsContext(const meta::Instrument &inst, u16 category, u16 start, u16 count) : BaseFrame<SecurityBarsContext>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x00;
            request_header.cmd_id = StdCommand::SECURITY_BARS;

            category_ = category;
            param.Category = category;
            param.I = 1;
            param.Start = start;
            param.Count = count;
            {
                param.Market = static_cast<u16>(helpers::exchange_to_market(inst.exchange));
                const char *const tmp = inst.market_ticker().c_str();
                std::memcpy(param.Code, tmp, sizeof(param.Code));
                if (meta::instype_is_index(inst.type)) {
                    isIndex = true;
                }
            }

            padding = strings::hexToBytes("00000000000000000000");
        }

        std::vector<u8> serialize_request_body_impl() {
            BinaryStream bs;
            bs.push_arithmetic(param.Market);
            bs.push_array(param.Code);
            bs.push_arithmetic(param.Category);
            bs.push_arithmetic(param.I);
            bs.push_arithmetic(param.Start);
            bs.push_arithmetic(param.Count);
            auto data = bs.data();
            data.insert(data.end(), padding.begin(), padding.end());
            return data;
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
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
                e.Vol = helpers::integer_to_float64(ivol);

                u32 dbvol = bs.get_u32();
                e.Amount = helpers::integer_to_float64(i64(dbvol));

                e.Open = f64(price_open_diff+pre_diff_base) / 1000.0;
                price_open_diff += pre_diff_base;

                e.Close = f64(price_open_diff+price_close_diff) / 1000.0;
                e.High = f64(price_open_diff+price_high_diff) / 1000.0;
                e.Low = f64(price_open_diff+price_low_diff) / 1000.0;

                pre_diff_base = price_open_diff + price_close_diff;

                if (isIndex) {
                    e.UpCount = bs.get_u16();
                    e.DownCount = bs.get_u16();
                }
                List.emplace_back(e);
            }
        }

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl();
            oss << "{Market:" << int(param.Market)
                << ", Code:" << strings::from(param.Code)
                << ", Category:" << klineTypeToString(static_cast<BarFreq>(param.Category))
                << ", I:" << int(param.I)
                << ", Start:" << int(param.Start)
                << ", Count:" << int(param.Count)
                << ", padding:" << strings::bytesToHex(padding)
                << "}";
            oss << " {Count:" << int(Count) << ", List:[";
            for(int i = 0; i < Count; i++) {
                oss << "{" << List[i] << "}";
            }
            oss << "]}";
            return oss.str();
        }
    };

}
#endif // QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_SECURITY_BARS_H
