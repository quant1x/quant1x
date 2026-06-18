#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_SECURITY_COUNT_H
#define QUANT1X_CONTRB_DATA_TDX_SECURITY_COUNT_H 1

// ==============================
// 证券统计
// ==============================

#include <quant1x/contrib/data/tdx/protocol.h>

namespace quant1x::contrib::data::tdx {

    // 证券统计 (对齐 Python SecurityCount)
    struct SecurityCount : public BaseMessage<SecurityCount> {
        u16 Market;                    // 市场
        std::vector<u8> padding={};    // 填充
        u16 Count;                     // 返回数量

        SecurityCount() : BaseMessage<SecurityCount>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x01;
            request_header.cmd_id = StdCommand::SECURITY_COUNT;

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

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl()
                << '{'
                << "Market:"<< int(Market)
                << ", padding:" << strings::bytesToHex(padding)
                << "}";
            return oss.str();
        }
    };

}
#endif //QUANT1X_CONTRB_DATA_TDX_SECURITY_COUNT_H
