#pragma once
#ifndef QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_HEARTBEAT_H
#define QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_HEARTBEAT_H 1

#include <quant1x/contrib/data/tdx/protocol.h>

// ==============================
// 心跳
// ==============================

namespace quant1x::contrib::data::tdx {

    // 心跳 (对齐 Python HeartbeatContext)
    struct HeartbeatContext : public BaseFrame<HeartbeatContext> {
        std::string info;// 10个字节的消息, 未解

        HeartbeatContext() : BaseFrame<HeartbeatContext>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x02;
            request_header.cmd_id = StdCommand::HEARTBEAT;
        }

        std::vector<u8> serialize_request_body_impl() {
            return {};
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream stream(data);
            info = stream.get_string(10);
        }

        std::string to_string_impl() {
            return fmt::format("Info: {}", info);
        }
    };

}

#endif // QUANT1X_CONTRIB_DATA_TDX_LEVEL1_STD_HEARTBEAT_H
