#pragma once
#ifndef QUANT1X_LEVEL1_SECURITY_COUNT_H
#define QUANT1X_LEVEL1_SECURITY_COUNT_H 1

// ==============================
// 证券统计
// ==============================

#include <quant1x/contrib/data/tdx/protocol.h>

namespace level1 {

    // 证券统计 (对齐 Python SecurityCount)
    struct SecurityCount : public BaseMessage<SecurityCount> {
        u16 Market;                    // 市场
        std::vector<u8> padding={};    // 填充
        u16 Count;                     // 返回数量

        SecurityCount() : BaseMessage<SecurityCount>() {
            request_header.ZipFlag = ZlibFlag::Uncompressed;
            request_header.SeqID = SequenceId();
            request_header.PacketType = 0x01;
            request_header.Method = StdCommand::SECURITY_COUNT;

            Market = 0;
            padding = strings::hexToBytes("75c73301");
        }

        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_arithmetic(Market);
            auto data = stream.data();
            data.insert(data.end(), padding.begin(), padding.end());
            return data;
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << request_header.headerStringImpl()
                << '{'
                << "Market:"<< int(Market)
                << ", padding:" << strings::bytesToHex(padding)
                << "}";
            return oss.str();
        }
    };

}
#endif //QUANT1X_LEVEL1_SECURITY_COUNT_H
