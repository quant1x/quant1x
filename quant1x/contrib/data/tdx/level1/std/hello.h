#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_HELLO_H
#define QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_HELLO_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/std/util.h>

// ==============================
// 标准行情 - 协议握手
// ==============================

namespace quant1x::contrib::data::tdx {

    // login1 - 第一次协议握手 (对齐 Python StdLoginContext)
    struct StdLoginContext : public BaseFrame<StdLoginContext> {
        std::vector<u8> padding;  // 请求体填充字节
        std::string Info;         // 响应信息

        StdLoginContext() : BaseFrame<StdLoginContext>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x01;
            request_header.cmd_id = StdCommand::LOGIN1;
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

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl();
            oss << ' ' << " padding:" << strings::bytesToHex(padding);
            oss << " Info:" << Info;
            return oss.str();
        }
    };

    // login2 - 第二次协议握手 (对齐 Python UpgradeTipContext)
    struct UpgradeTipContext : public BaseFrame<UpgradeTipContext> {
        std::vector<u8> padding;  // 请求体填充字节
        std::string Info;         // 响应信息

        UpgradeTipContext() : BaseFrame<UpgradeTipContext>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x01;
            request_header.cmd_id = StdCommand::LOGIN2;
            padding = strings::hexToBytes("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002");
        }

        // 序列化请求体
        std::vector<u8> serialize_request_body_impl() {
            return padding;
        }

        // 反序列化响应体
        void deserialize_response_body_impl(const std::vector<u8> &data) {
            const int offset = 58;
            if (data.size() >= offset) {
                // 截取从offset字节开始的部分
                std::vector<u8> infoBytes(data.begin() + offset, data.end());
                auto str = std::string(infoBytes.begin(), infoBytes.end());
                Info = charsets::gbk_to_utf8(str);
            }
        }

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl();
            oss << ' ' << " padding:" << strings::bytesToHex(padding);
            oss << " Info:" << Info;
            return oss.str();
        }
    };

} // namespace quant1x::contrib::data::tdx

template <>
struct fmt::formatter<quant1x::contrib::data::tdx::StdLoginContext> {
    // 解析格式化规则(这里不需要特殊处理, 直接返回)
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }

    // 格式化逻辑
    template <typename FormatContext>
    auto format(const quant1x::contrib::data::tdx::StdLoginContext& pkg, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), pkg.to_string_impl());
    }
};

#endif // QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_HELLO_H
