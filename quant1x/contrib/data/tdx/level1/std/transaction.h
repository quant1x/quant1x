#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_TRANSACTION_DATA_H
#define QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_TRANSACTION_DATA_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/contrib/data/tdx/level1/std/security_quote.h>
#include <quant1x/data/meta/instrument.h>
#include <ostream>
#include <stdexcept>

namespace quant1x::contrib::data::tdx {

    // ==============================
    // 分笔成交记录(Tick-by-Tick Transaction)
    // ==============================


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

    // 分笔成交请求/响应 (对齐 Python TransactionContext)
    struct TransactionContext : public BaseFrame<TransactionContext> {
        uint16_t market;    // 市场代码
        char     ticker[6]; // 证券代码(固定6字节)
        uint16_t start;     // 起始位置
        uint16_t count;     // 请求数量
        bool     is_index;  // 是否为指数(响应解析用)

        int         market_;                        // 响应解析用
        const char *code_;                  // 响应解析用
        uint16_t    Count;                     // 返回的记录数
        std::vector<TickTransaction> List;  // 分笔成交数据列表
        
        TransactionContext(const meta::Instrument &inst, u16 offset, u16 size) : BaseFrame<TransactionContext>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x00;
            request_header.cmd_id = StdCommand::TRANSACTION_DATA;
            {
                market = static_cast<u8>(helpers::exchange_to_market(inst.exchange));
                const char *const tmp = inst.market_ticker().c_str();
                std::memcpy(ticker, tmp, sizeof(ticker));
            }
            start = offset;
            count = size;
            market_ = market;
            is_index = instype_is_index(inst.type);
            code_ = ticker;
        }

        // 序列化方法
        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_arithmetic(market);
            stream.push_array(ticker);
            stream.push_arithmetic(start);
            stream.push_arithmetic(count);
            return stream.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            auto baseUnit = helpers::default_base_unit(market_, code_);
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
                    if(is_index) {
                        auto amount = (e.vol * 100);
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
                spdlog::warn("[TransactionContext] insufficient data for {} transactions, parsed {} successfully", Count, List.size());
                Count = List.size();
            }
        }

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl()
                << "{"
                << "market:" << int(market)
                << ", ticker:" << strings::from(ticker)
                << ", start:" << start
                << ", Count:" << count
                << "}";
            oss << " {RspCount:" << Count << ", List:[";
            for(int i = 0; i < Count; i++) {
                oss << "{" << List[i] << "}";
            }
            oss << "]}";
            return oss.str();
        }
    };


    // ==============================
    // 历史分笔成交记录
    // ==============================


    // 历史分笔成交请求/响应 (对齐 Python HistoricalTransactionContext)
    struct HistoricalTransactionContext : public BaseFrame<HistoricalTransactionContext> {
        uint32_t Date;      // 日期
        uint16_t Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)
        uint16_t Start;     // 起始位置
        uint16_t ReqCount;  // 请求数量
        bool     is_index;  // 是否为指数(响应解析用)

        uint16_t Count;                     // 返回的记录数
        std::vector<TickTransaction> List;  // 分笔成交数据列表
        int market_;
        const char *code_;

        HistoricalTransactionContext(const meta::Instrument &inst, u32 date, u16 offset, u16 size) : BaseFrame<HistoricalTransactionContext>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x00;
            request_header.cmd_id = StdCommand::HISTORY_TRANSACTION_DATA;
            {
                Market = static_cast<u8>(helpers::exchange_to_market(inst.exchange));
                const char *const tmp = inst.market_ticker().c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Date = date;
            Start = offset;
            ReqCount = size;
            market_ = Market;
            code_    = Code;
            is_index = instype_is_index(inst.type);
        }

        // 序列化方法
        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_arithmetic(Date);
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
            auto baseUnit = helpers::default_base_unit(market_, code_);
            i64 lastPrice = 0;
            bs.skip(4); // 历史分笔成交记录, 跳过4个字节
            try {
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
                    if(is_index) {
                        auto amount = (e.vol * 100);
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
                spdlog::warn("[HistoricalTransactionContext] insufficient data for {} historical transactions, parsed {} successfully", Count, List.size());
                Count = List.size();
            }
        }

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl()
                << "{"
                << "Date:" << Date
                << ", Market:" << int(Market)
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

}  // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_TRANSACTION_DATA_H
