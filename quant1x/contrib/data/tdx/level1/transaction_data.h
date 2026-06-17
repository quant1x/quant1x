#pragma once
#ifndef QUANT1X_LEVEL1_TRANSACTION_DATA_H
#define QUANT1X_LEVEL1_TRANSACTION_DATA_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/level1/security_quote.h>
#include <quant1x/data/meta/exchange.h>
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

    // 分笔成交请求/响应 (对齐 Python Transaction)
    struct Transaction : public BaseMessage<Transaction> {
        uint16_t Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)
        uint16_t Start;     // 起始位置
        uint16_t ReqCount;  // 请求数量

        uint16_t Count;                     // 返回的记录数
        std::vector<TickTransaction> List;  // 分笔成交数据列表
        int market_;                        // 响应解析用
        const char *code_;                  // 响应解析用

        Transaction(const std::string &securityCode, u16 offset, u16 size) : BaseMessage<Transaction>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x00;
            request_header.cmd_id = StdCommand::TRANSACTION_DATA;
            {
                auto [id, _, symbol] = detect_symbol(securityCode);
                Market = static_cast<uint16_t>(id);
                const char * const tmp = symbol.c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Start = offset;
            ReqCount = size;
            market_ = Market;
            code_ = Code;
        }

        // 序列化方法
        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            stream.push_arithmetic(Start);
            stream.push_arithmetic(ReqCount);
            return stream.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            auto baseUnit = helpers::defaultBaseUnit(market_, code_);
            auto isIndex = assert_index_by_security_code(static_cast<meta::Exchange>(market_), std::string(code_));
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
                spdlog::warn("[Transaction] insufficient data for {} transactions, parsed {} successfully", Count, List.size());
                Count = List.size();
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << request_header.headerStringImpl()
                << "{"
                << "Market:" << int(Market)
                << ", Code:" << strings::from(Code)
                << ", Start:" << Start
                << ", Count:" << ReqCount
                << "}";
            oss << " {RspCount:" << Count << ", List:[";
            for(int i = 0; i < Count; i++) {
                oss << "{" << List[i] << "}";
            }
            oss << "]}";
            return oss.str();
        }
    };

}

#endif //QUANT1X_LEVEL1_TRANSACTION_DATA_H
