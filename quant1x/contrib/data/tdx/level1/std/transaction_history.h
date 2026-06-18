#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_TRANSACTION_HISTORY_H
#define QUANT1X_CONTRB_DATA_TDX_TRANSACTION_HISTORY_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/contrib/data/tdx/level1/std/transaction_data.h>
#include <quant1x/data/meta/instrument.h>
#include <stdexcept>

// ==============================
// 历史分笔成交记录
// ==============================

namespace quant1x::contrib::data::tdx {

    // 历史分笔成交请求/响应 (对齐 Python HistoricalTransaction)
    struct HistoryTransaction : public BaseMessage<HistoryTransaction> {
        uint32_t Date;      // 日期
        uint16_t Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)
        uint16_t Start;     // 起始位置
        uint16_t ReqCount;  // 请求数量

        uint16_t Count;                     // 返回的记录数
        std::vector<TickTransaction> List;  // 分笔成交数据列表
        int market_;
        const char *code_;

        HistoryTransaction(const meta::Instrument &inst, u32 date, u16 offset, u16 size) : BaseMessage<HistoryTransaction>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x00;
            request_header.cmd_id = StdCommand::HISTORY_TRANSACTION_DATA;
            {
                Market = static_cast<u8>(helpers::exchange_to_market(inst.exchange));
                const char *const tmp = inst.marker_ticker().c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Date = date;
            Start = offset;
            ReqCount = size;
            market_ = Market;
            code_ = Code;
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
            auto baseUnit = helpers::defaultBaseUnit(market_, code_);
            auto isIndex = quant1x::data::assert_index_by_security_code(std::string(code_));
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
                spdlog::warn("[HistoryTransaction] insufficient data for {} historical transactions, parsed {} successfully", Count, List.size());
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

}

#endif //QUANT1X_CONTRB_DATA_TDX_TRANSACTION_HISTORY_H
