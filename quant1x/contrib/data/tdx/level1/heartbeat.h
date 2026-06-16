#pragma once
#ifndef QUANT1X_LEVEL1_HEARTBEAT_H
#define QUANT1X_LEVEL1_HEARTBEAT_H 1

#include <quant1x/contrib/data/tdx/protocol.h>

// ==============================
// 心跳
// ==============================

namespace level1 {

    // 心跳 (对齐 Python Heartbeat)
    struct Heartbeat : public BaseMessage<Heartbeat> {
        std::string info;// 10个字节的消息, 未解

        Heartbeat() : BaseMessage<Heartbeat>() {
            request_header.ZipFlag = ZlibFlag::Uncompressed;
            request_header.SeqID = SequenceId();
            request_header.PacketType = 0x02;
            request_header.Method = StdCommand::HEARTBEAT;
        }

        std::vector<u8> serialize_request_body_impl() {
            return {};
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream stream(data);
            info = stream.get_string(10);
        }

        std::string toStringImpl() {
            return fmt::format("Info: {}", info);
        }
    };

}

#endif //QUANT1X_LEVEL1_HEARTBEAT_H
