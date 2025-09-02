#pragma once
#ifndef QUANT1X_LEVEL1_TRANSACTION_HISTORY_H
#define QUANT1X_LEVEL1_TRANSACTION_HISTORY_H 1

#include "protocol.h"
#include "transaction_data.h"

// ==============================
// 历史分笔成交记录
// ==============================

namespace level1 {

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

#pragma pack(pop)  // 恢复默认对齐方式

    struct HistoryTransactionRequest : public RequestHeader<HistoryTransactionRequest> {
        uint32_t Date;      // 日期
        uint16_t Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)
        uint16_t Start;     // 起始位置
        uint16_t Count;     // 请求数量

        HistoryTransactionRequest(const std::string &securityCode, u32 date, u16 offset, u16 size) : RequestHeader<HistoryTransactionRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x00;
            Method = StdCommand::HISTORY_TRANSACTION_DATA;
            {
                auto [id, _, symbol] = exchange::DetectMarket(securityCode);
                Market = id;
                const char * const tmp = symbol.c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Date = date;
            Start = offset;
            Count = size;
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + 4 + 2 + 6 + 2 + 2;
            PkgLen2 = 2 + 4 + 2 + 6 + 2 + 2;
            auto buf = RequestHeader<HistoryTransactionRequest>::headerSerialize();
            BinaryStream stream;
            stream.push_arithmetic(Date);
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            stream.push_arithmetic(Start);
            stream.push_arithmetic(Count);
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<HistoryTransactionRequest>::headerStringImpl()
                << "{"
                << "Market:" << int(Market)
                << ", Code:" << strings::from(Code)
                << ", Start:" << Start
                << ", Count:" << Count
                << "}";
            return oss.str();
        }

    };

    struct HistoryTransactionResponse : public ResponseHeader<HistoryTransactionResponse> {
        uint16_t Count;                     // 返回的记录数
        std::vector<TickTransaction> List;  // 分笔成交数据列表
        int market_;
        const char *code_;

        HistoryTransactionResponse(int market, const char *code) : ResponseHeader<HistoryTransactionResponse>() {
            Count = 0;
            List = {};
            market_ = market;
            code_ = code;
        }

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            auto baseUnit = defaultBaseUnit(market_, code_);
            auto isIndex = exchange::AssertIndexByMarketAndCode(static_cast<exchange::MarketType>(market_), std::string(code_));
            i64 lastPrice = 0;
            bs.skip(4); // 历史分笔成交记录, 跳过4个字节
            for(int i = 0; i < Count; ++i) {
                TickTransaction e{};
                u16 minutes = bs.get_u16();
                auto h = minutes / 60;
                auto m = minutes % 60;
                e.time = fmt::format("{:02d}:{:02d}", h, m);
                i64 rawPrice = bs.varint_decode();
                e.vol = bs.varint_decode();
                //e.num = bs.varint_decode(); // 历史分笔成交记录没有这个字段
                e.buyOrSell = bs.varint_decode();
                lastPrice += rawPrice;
                e.price = f64(lastPrice)/baseUnit;
                if(isIndex) {
                    auto amount = e.vol * 100;
                    e.amount = f64(amount);
                    e.vol = i64(e.amount / e.price);
                } else {
                    e.vol *= 100;
                    e.amount = f64(e.vol) * e.price;
                }
                auto _ = bs.varint_decode();
                List.emplace_back(e);
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << ResponseHeader<HistoryTransactionResponse>::headerStringImpl()
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

#endif //QUANT1X_LEVEL1_TRANSACTION_HISTORY_H
