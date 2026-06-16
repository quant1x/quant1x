#pragma once
#ifndef QUANT1X_LEVEL1_HELLO1_H
#define QUANT1X_LEVEL1_HELLO1_H 1

#include <quant1x/contrib/data/tdx/level1/protocol.h>
#include <quant1x/std/util.h>

// ==============================
// 第一次协议握手
// ==============================

namespace level1 {

    // login1 - 第一次协议握手 (对齐 Python StdLogin)
    struct Hello1 : public BaseMessage<Hello1> {
        std::vector<u8> padding;  // 请求体填充字节
        std::string Info;         // 响应信息

        Hello1() : BaseMessage<Hello1>() {
            request_header.ZipFlag = ZlibFlag::Uncompressed;
            request_header.SeqID = SequenceId();
            request_header.PacketType = 0x01;
            request_header.Method = StdCommand::LOGIN1;
            padding = strings::hexToBytes("01");
        }

        // 序列化请求体
        std::vector<u8> serialize_request_body_impl() {
            return padding;
        }

        // 反序列化响应体
        void deserialize_response_body_impl(const std::vector<u8> &data) {
            const int offset = 68;
            if (data.size() >= offset) {
                // 截取从68字节开始的部分
                std::vector<u8> infoBytes(data.begin() + offset, data.end());
                auto str = std::string(infoBytes.begin(), infoBytes.end());
                Info = charsets::gbk_to_utf8(str);
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << request_header.headerStringImpl();
            oss << ' ' << " padding:" << strings::bytesToHex(padding);
            oss << " Info:" << Info;
            return oss.str();
        }
    };

} // namespace level1

template <>
struct fmt::formatter<level1::Hello1> {
    // 解析格式化规则（这里不需要特殊处理，直接返回）
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    // 格式化逻辑
    template <typename FormatContext>
    auto format(const level1::Hello1& pkg, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), pkg.toStringImpl());
    }
};

#endif //QUANT1X_LEVEL1_HELLO1_H
