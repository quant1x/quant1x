#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_INSTRUMENT_BARS_H
#define QUANT1X_CONTRB_DATA_TDX_INSTRUMENT_BARS_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/data/schema/bar.h>

namespace quant1x::contrib::data::tdx {

/// 扩展行情K线请求 (对应 Python level1/ext.py InstrumentBars, 命令字 0x23FF)
/// 协议格式: packet_ctrl=0x01, frame_type=0x01 (FLAG_GENERIC)
struct InstrumentBars : public BaseMessage<InstrumentBars> {
    static constexpr int PRE_REQUEST_MAX = 700;

    u8  market;
    std::string ticker;
    u16 category;
    u16 frequency = 1; // 频率为1时返回category设定的原始周期
    u32 start;
    u16 count;

    /// 响应数据 — 已经转换为 domain Bar (对应 Python bars.reply)
    std::vector<meta::schema::Bar> reply;

    InstrumentBars(u8 _market, const std::string& _ticker, u16 _category, u32 _start, u16 _count)
        : BaseMessage<InstrumentBars>()
        , market(_market)
        , ticker(_ticker)
        , category(_category)
        , start(_start)
        , count(_count) {
        request_header.frame_type    = 0x01; // FLAG_GENERIC
        request_header.seq_id      = get_sequence_id();
        request_header.packet_ctrl = 0x01; // ext frame type
        request_header.cmd_id     = 0x23FF; // EXT_INSTRUMENT_BARS
    }

    /// 序列化请求体 — 对齐 Python: struct.pack('<B9sHHIH', market, ticker, category, frequency, start, count)
    std::vector<u8> serialize_request_body_impl() {
        BinaryStream bs;
        bs.push_arithmetic(market);
        {
            // ticker 填充到 9 字节 (对齐 Python: 9s)
            std::string padded = ticker;
            padded.resize(9, '\0');
            bs.push_byte_array(reinterpret_cast<const u8*>(padded.data()), 9);
        }
        bs.push_arithmetic(category);
        bs.push_arithmetic(frequency);
        bs.push_arithmetic(start);
        bs.push_arithmetic(count);
        return bs.data();
    }

    /// 反序列化响应体 — 对齐 Python InstrumentBars.deserialize_response_body
    /// body 格式: 跳过 14 字节 → start(u32) + count(u16) → count * 28 字节/条
    void deserialize_response_body_impl(const std::vector<u8>& data) {
        if (data.size() < 20) return;

        BinaryStream bs(data);
        // 跳过前14字节 (对齐 Python: pos = 0; pos += 14)
        for (int i = 0; i < 14; ++i) (void)bs.get_u8();

        u32 resp_start = bs.get_u32();
        u16 resp_count = bs.get_u16();
        (void)resp_start;

        reply.clear();
        reply.reserve(resp_count);

        for (int i = 0; i < resp_count; ++i) {
            // 日期: u32 YYYYMMDD
            u32 zipday = bs.get_u32();
            int year  = static_cast<int>(zipday / 10000);
            int month = static_cast<int>((zipday % 10000) / 100);
            int day   = static_cast<int>(zipday % 100);

            // OHLC: 4 × f32 = 16 bytes
            f64 open_f  = static_cast<f64>(bs.get_float());
            f64 high_f  = static_cast<f64>(bs.get_float());
            f64 low_f   = static_cast<f64>(bs.get_float());
            f64 close_f = static_cast<f64>(bs.get_float());

            // position(u32) + volume(u32) + price(f32) = 12 bytes
            (void)bs.get_u32();       // skip position
            u32 volume_u32 = bs.get_u32();
            (void)bs.get_float();     // skip price

            meta::schema::Bar bar;
            bar.date      = fmt::format("{:04d}-{:02d}-{:02d}", year, month, day);
            bar.open      = open_f;
            bar.close     = close_f;
            bar.high      = high_f;
            bar.low       = low_f;
            bar.volume    = static_cast<f64>(volume_u32);
            bar.amount    = 0.0; // ext协议没有成交额字段 (对齐 Rust: amount = 0.0)
            bar.up        = 0;
            bar.down      = 0;
            bar.timestamp = fmt::format("{:04d}-{:02d}-{:02d} 15:00:00", year, month, day);
            bar.adjustment_count = 0;
            reply.push_back(std::move(bar));
        }
    }

    std::string to_string_impl() const {
        return fmt::format(
            "InstrumentBars{{market:{}, ticker:{}, category:{}, frequency:{}, start:{}, count:{}, reply_count:{}}}",
            market, ticker, category, frequency, start, count, reply.size());
    }
};

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_CONTRB_DATA_TDX_INSTRUMENT_BARS_H
