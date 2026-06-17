#pragma once
#ifndef QUANT1X_LEVEL1_HELLO2_H
#define QUANT1X_LEVEL1_HELLO2_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/std/util.h>

// ==============================
// 第二次协议握手
// ==============================

namespace level1 {

    // login2 - 第二次协议握手 (对齐 Python UpgradeTip)
    struct Hello2 : public BaseMessage<Hello2> {
        std::vector<u8> padding;  // 请求体填充字节
        std::string Info;         // 响应信息

        Hello2() : BaseMessage<Hello2>() {
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

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << request_header.headerStringImpl();
            oss << ' ' << " padding:" << strings::bytesToHex(padding);
            oss << " Info:" << Info;
            return oss.str();
        }
    };

}

#endif //QUANT1X_LEVEL1_HELLO2_H
