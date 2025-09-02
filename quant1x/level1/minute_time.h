#pragma once
#ifndef QUANT1X_LEVEL1_MINUTE_TIME_H
#define QUANT1X_LEVEL1_MINUTE_TIME_H 1

#include "protocol.h"

// ==============================
// 分时数据(历史), 当日分时数据和历史分时数据没区别, 只是命令字不同, 且ETF数据不准确
// ==============================

namespace level1 {

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

#pragma pack(pop)  // 恢复默认对齐方式

    struct HistoryMinuteTimeRequest : public RequestHeader<HistoryMinuteTimeRequest> {
        uint32_t Date;      // 日期
        uint8_t  Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)

        HistoryMinuteTimeRequest(const std::string &securityCode, u32 date) : RequestHeader<HistoryMinuteTimeRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x00;
            Method = StdCommand::HISTORY_MINUTE_DATA;
            {
                auto [id, _, symbol] = exchange::DetectMarket(securityCode);
                Market = id;
                const char * const tmp = symbol.c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Date = date;
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + 4 + 1 + 6;
            PkgLen2 = 2 + 4 + 1 + 6;
            auto buf = RequestHeader<HistoryMinuteTimeRequest>::headerSerialize();
            BinaryStream stream;
            stream.push_arithmetic(Date);
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<HistoryMinuteTimeRequest>::headerStringImpl()
                << "{"
                << "Market:" << int(Market)
                << ", Code:" << strings::from(Code)
                << "}";
            return oss.str();
        }

    };

    struct MinuteTime {
        f32 Price;
        i64 Vol;

        friend std::ostream &operator<<(std::ostream &os, const MinuteTime &minuteTime) {
            os << "Price: " << minuteTime.Price << " Vol: " << minuteTime.Vol;
            return os;
        }
    };

    struct HistoryMinuteTimeResponse : public ResponseHeader<HistoryMinuteTimeResponse> {
        uint16_t Count;                     // 返回的记录数
        std::vector<MinuteTime> List;  // 分笔成交数据列表
        int market_;
        const char *code_;

        HistoryMinuteTimeResponse(int market, const char* code) : ResponseHeader<HistoryMinuteTimeResponse>() {
            Count = 0;
            List = {};
            market_ = market;
            code_ = code;
        }

        void deserializeImpl(const std::vector<u8> &data) {
            if(data.size() < 2) {
                return;
            }
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            auto baseUnit = defaultBaseUnit(market_, code_);
            auto isIndex = exchange::AssertIndexByMarketAndCode(static_cast<exchange::MarketType>(market_), std::string(code_));
            i64 lastPrice = 0;
            bs.skip(4); // 历史分笔成交记录, 跳过4个字节
            for(int i = 0; i < Count; ++i) {
                MinuteTime e{};
                i64 rawPrice = bs.varint_decode();
                i64 reversed1 = bs.varint_decode();
                (void)reversed1;
                i64 vol = bs.varint_decode();
                e.Vol = vol;
                lastPrice += rawPrice;
                e.Price = f32(lastPrice)/f32(baseUnit);
                List.emplace_back(e);
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << ResponseHeader<HistoryMinuteTimeResponse>::headerStringImpl()
                << "{Count:" << Count
                << ", List:[";
            for(int i = 0; i < Count; i++) {
                oss << "{" << List[i] << "}";
            }
            oss << "]}";
            return oss.str();
        }
    };

}


#endif //QUANT1X_LEVEL1_MINUTE_TIME_H
