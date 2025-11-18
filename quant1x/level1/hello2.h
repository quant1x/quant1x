#pragma once
#ifndef QUANT1X_LEVEL1_HELLO2_H
#define QUANT1X_LEVEL1_HELLO2_H 1

#include <quant1x/level1/protocol.h>
#include <quant1x/std/util.h>

// ==============================
// 第二次协议握手
// ==============================

namespace level1 {
    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    // login2 - request
    struct Hello2Request : RequestHeader<Hello2Request> {
        std::vector<u8> padding;

        Hello2Request() : RequestHeader<Hello2Request>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x01;
            Method = StdCommand::LOGIN2;
            padding = strings::hexToBytes("d5d0c9ccd6a4a8af0000008fc22540130000d500c9ccbdf0d7ea00000002");
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + padding.size();
            PkgLen2 = 2 + padding.size();
            auto buf = RequestHeader<Hello2Request>::headerSerialize();
            buf.insert(buf.end(), padding.begin(), padding.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<Hello2Request>::headerStringImpl();
            oss << ' ' << " padding:" << strings::bytesToHex(padding);
            return oss.str();
        }
    };

    // login2 - response
    struct Hello2Response : public ResponseHeader<Hello2Response> {
        std::string Info;

        void deserializeImpl(const std::vector<u8> &data) {
            const int offset = 58;
            if (data.size() >= offset) {
                // 截取从offset字节开始的部分
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
}

#endif //QUANT1X_LEVEL1_HELLO2_H
