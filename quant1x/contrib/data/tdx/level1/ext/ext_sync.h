#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_EXT_SYNC_H
#define QUANT1X_CONTRB_DATA_TDX_EXT_SYNC_H 1

#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/std/util.h>

namespace quant1x::contrib::data::tdx {

/// 扩展行情握手请求 (对应 Python level1/ext.py Synchronize, 命令字 0x2454)
/// 协议格式与标准行情不同: packet_ctrl=0x01, frame_type=0x01 (FLAG_GENERIC)
struct ExtSync : public BaseMessage<ExtSync> {
    bool success = false;

    ExtSync() : BaseMessage<ExtSync>() {
        request_header.frame_type    = 0x01; // FLAG_GENERIC
        request_header.seq_id      = get_sequence_id();
        request_header.packet_ctrl = 0x01; // ext frame type
        request_header.cmd_id     = 0x2454;
    }

    std::vector<u8> serialize_request_body_impl() {
        // 80 字节 padding (对齐 Python Synchronize.serialize_request_body)
        return strings::hexToBytes(
            "e5bb1c2fafe52594"
            "1f32c6e5d53dfb41"
            "5b734cc9cdbf0ac9"
            "2021bfdd1eb06d22"
            "d008884c1611cb13"
            "78f6abd824d899d2"
            "1f32c6e5d53dfb41"
            "1f32c6e5d53dfb41"
            "a9325ac935dc0837"
            "335a16e4ce17c1bb");
    }

    void deserialize_response_body_impl(const std::vector<u8>& data) {
        success = !data.empty() && data[0] > 0;
    }

    std::string to_string_impl() const {
        return fmt::format("ExtSync{{success={}}}", success);
    }
};

} // namespace quant1x::contrib::data::tdx

#endif // QUANT1X_CONTRB_DATA_TDX_EXT_SYNC_H
