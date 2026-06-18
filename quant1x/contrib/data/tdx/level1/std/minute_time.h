#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_MINUTE_TIME_H
#define QUANT1X_CONTRB_DATA_TDX_MINUTE_TIME_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/data/meta/instrument.h>
#include <stdexcept>

// ==============================
// 分时数据(历史), 当日分时数据和历史分时数据没区别, 只是命令字不同, 且ETF数据不准确
// ==============================

namespace quant1x::contrib::data::tdx {

    struct MinuteTime {
        f32 Price;
        i64 Vol;

        friend std::ostream &operator<<(std::ostream &os, const MinuteTime &minuteTime) {
            os << "Price: " << minuteTime.Price << " Vol: " << minuteTime.Vol;
            return os;
        }
    };

    // 历史分时数据 (对齐 Python HistoryMinuteTime)
    struct HistoryMinuteTime : public BaseMessage<HistoryMinuteTime> {
        uint32_t Date;      // 日期
        uint8_t  Market;    // 市场代码
        char Code[6];       // 证券代码(固定6字节)

        uint16_t Count;                     // 返回的记录数
        std::vector<MinuteTime> List;       // 分时数据列表
        int market_;
        const char *code_;

        HistoryMinuteTime(const meta::Instrument &inst, u32 date) : BaseMessage<HistoryMinuteTime>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x00;
            request_header.cmd_id = StdCommand::HISTORY_MINUTE_DATA;
            {
                Market = static_cast<u8>(helpers::exchange_to_market(inst.exchange));
                const char *const tmp = inst.marker_ticker().c_str();
                std::memcpy(Code, tmp, sizeof(Code));
            }
            Date = date;
            market_ = Market;
            code_ = Code;
        }

        // 序列化方法
        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_arithmetic(Date);
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            return stream.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            if(data.size() < 2) {
                return;
            }
            BinaryStream bs(data);
            Count = bs.get_u16();
            List.reserve(Count);
            auto baseUnit = helpers::defaultBaseUnit(market_, code_);
            i64 lastPrice = 0;
            bs.skip(4); // 历史分笔成交记录, 跳过4个字节
            try {
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
            } catch(const std::out_of_range&) {
                spdlog::warn("[HistoryMinuteTime] insufficient data for {} minute times, parsed {} successfully", Count, List.size());
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
                << "}";
            oss << " {Count:" << Count << ", List:[";
            for(int i = 0; i < Count; i++) {
                oss << "{" << List[i] << "}";
            }
            oss << "]}";
            return oss.str();
        }
    };

}


#endif //QUANT1X_CONTRB_DATA_TDX_MINUTE_TIME_H
