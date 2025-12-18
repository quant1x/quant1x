#pragma once
#ifndef QUANT1X_LEVEL1_TRANSACTION_DATA_H
#define QUANT1X_LEVEL1_TRANSACTION_DATA_H 1

#include <quant1x/level1/protocol.h>
#include <quant1x/level1/security_quote.h>
#include <quant1x/exchange/code.h>
#include <ostream>
#include <stdexcept>

// ==============================
// 分笔成交记录(Tick-by-Tick Transaction)
// ==============================

namespace level1 {

    // 交易类型常量定义
    constexpr int32_t tick_buy = 0;      // 买入类型
    constexpr int32_t tick_sell = 1;     // 卖出类型
    constexpr int32_t tick_neutral = 2;  // 中性盘类型
    constexpr int32_t tick_unknown = 3;  // 未知类型(出现在09:27分的历史数据中)

    // 单次请求最大交易记录数
    constexpr uint16_t tick_transaction_per_request_max = 1800;

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐
    // 分笔成交数据结构
    struct TickTransaction {
        std::string time;     // 成交时间(HH:MM格式)
        f64 price;            // 成交价格
        i64 vol;              // 成交量(股数)
        i64 num;              // 成交笔数(历史数据中可能不存在)
        f64 amount;           // 成交金额(新增字段)
        i64 buyOrSell;        // 买卖方向(对应tick_xxx常量)

        friend std::ostream &operator<<(std::ostream &os, const TickTransaction &transaction) {
            os << "time: " << transaction.time << " price: " << transaction.price << " vol: " << transaction.vol
               << " num: " << transaction.num << " amount: " << transaction.amount << " buyOrSell: "
               << transaction.buyOrSell;
            return os;
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式

    struct TransactionRequest : public RequestHeader<TransactionRequest> {
        uint16_t Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)
        uint16_t Start;     // 起始位置
        uint16_t Count;     // 请求数量

        TransactionRequest(const std::string &securityCode, u16 offset, u16 size) : RequestHeader<TransactionRequest>() {
            ZipFlag = ZlibFlag::Uncompressed;
            SeqID = SequenceId();
            PacketType = 0x00;
            Method = StdCommand::TRANSACTION_DATA;
            {
                auto [id, _, symbol] = exchange::DetectMarket(securityCode);
                Market = static_cast<uint16_t>(id);
                const char * const tmp = symbol.c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Start = offset;
            Count = size;
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + 2 + 6 + 2 + 2;
            PkgLen2 = 2 + 2 + 6 + 2 + 2;
            auto buf = RequestHeader<TransactionRequest>::headerSerialize();
            BinaryStream stream;
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
            oss << RequestHeader<TransactionRequest>::headerStringImpl()
                << "{"
                << "Market:" << int(Market)
                << ", Code:" << strings::from(Code)
                << ", Start:" << Start
                << ", Count:" << Count
                << "}";
            return oss.str();
        }

    };

    struct TransactionResponse : public ResponseHeader<TransactionResponse> {
        uint16_t Count;                     // 返回的记录数
        std::vector<TickTransaction> List;  // 分笔成交数据列表
        int market_;
        const char *code_;

        TransactionResponse(int market, const char *code) : ResponseHeader<TransactionResponse>() {
            Count = 0;
            List = {};
            market_ = market;
            code_ = code;
        }

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            auto baseUnit = helpers::defaultBaseUnit(market_, code_);
            auto isIndex = exchange::AssertIndexByMarketAndCode(static_cast<exchange::ExchangeId>(market_), std::string(code_));
            i64 lastPrice = 0;
            try {
                for(int i = 0; i < Count; ++i) {
                    TickTransaction e{};
                    u16 seconds = bs.get_u16();
                    auto h = seconds / 60;
                    auto m = seconds % 60;
                    e.time = fmt::format("{:02d}:{:02d}", h, m);
                    i64 rawPrice = bs.varint_decode();
                    e.vol = bs.varint_decode();
                    e.num = bs.varint_decode();
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
            } catch(const std::out_of_range&) {
                spdlog::warn("[TransactionResponse] insufficient data for {} transactions, parsed {} successfully", Count, List.size());
                Count = List.size();
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << ResponseHeader<TransactionResponse>::headerStringImpl()
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

#endif //QUANT1X_LEVEL1_TRANSACTION_DATA_H
