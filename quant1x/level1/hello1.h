#pragma once
#ifndef QUANT1X_LEVEL1_HELLO1_H
#define QUANT1X_LEVEL1_HELLO1_H 1

#include <quant1x/level1/protocol.h>
#include <quant1x/std/util.h>

// ==============================
// 第一次协议握手
// ==============================

namespace level1 {
#pragma pack(push, 1)  // 确保1字节对齐

    // login1 - request
    struct Hello1Request : public RequestHeader<Hello1Request> {
        std::vector<u8> padding;

        Hello1Request() : RequestHeader<Hello1Request>(){
                ZipFlag = ZlibFlag::Uncompressed;
                SeqID = SequenceId();
                PacketType = 0x01;
                Method = StdCommand::LOGIN1;
                padding = strings::hexToBytes("01");
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + padding.size();
            PkgLen2 = 2 + padding.size();
            auto buf = RequestHeader<Hello1Request>::headerSerialize();
            buf.insert(buf.end(), padding.begin(), padding.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<Hello1Request>::headerStringImpl();
            oss << ' ' << " padding:" << strings::bytesToHex(padding);
            return oss.str();
        }
    };

    // login1 - response
    struct Hello1Response : public ResponseHeader<Hello1Response> {
        std::string Info;

        void deserializeImpl(const std::vector<u8> &data) {
            const int offset = 68;
            if (data.size() >= offset) {
                // 截取从68字节开始的部分
                std::vector<u8> infoBytes(data.begin() + offset, data.end());
                auto str = std::string(infoBytes.begin(), infoBytes.end());
                Info = charsets::gbk_to_utf8(str);
            }
        }

        std::string toStringImpl() const {
            return fmt::format("Info: {}", Info);
        }

    };
#pragma pack(pop)  // 恢复默认对齐方式

} // namespace level1

template <>
struct fmt::formatter<level1::Hello1Request> {
    // 解析格式化规则（这里不需要特殊处理，直接返回）
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    // 格式化逻辑
    template <typename FormatContext>
    auto format(const level1::Hello1Request& pkg, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), pkg.toStringImpl());
    }
};

template<>
struct fmt::formatter<level1::Hello1Response> {
    // 解析格式化规则（这里不需要特殊处理，直接返回）
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    // 格式化逻辑
    template <typename FormatContext>
    auto format(const level1::Hello1Response& pkg, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), pkg.toStringImpl());
    }
};

#endif //QUANT1X_LEVEL1_HELLO1_H
