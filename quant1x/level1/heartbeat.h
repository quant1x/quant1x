#pragma once
#ifndef QUANT1X_LEVEL1_HEARTBEAT_H
#define QUANT1X_LEVEL1_HEARTBEAT_H 1

#include "protocol.h"

// ==============================
// 心跳
// ==============================

namespace level1 {
#pragma pack(push, 1)  // 确保1字节对齐

    struct HeartbeatRequest : public RequestHeader<HeartbeatRequest> {

        HeartbeatRequest() : RequestHeader<HeartbeatRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x02;
            Method = StdCommand::HEARTBEAT;
        }

        std::vector<u8> serializeImpl() {
            spdlog::debug("HeartbeatRequest");
            PkgLen1 = 2 + 0;
            PkgLen2 = 2 + 0;
            auto buf = RequestHeader<HeartbeatRequest>::headerSerialize();
            return buf;
        }

        std::string toStringImpl() {
            std::ostringstream oss;
            oss << RequestHeader<HeartbeatRequest>::headerStringImpl();
            //oss << ' ' << " padding:" << util::bytesToHex(padding);
            return oss.str();
        }

    };

    struct HeartbeatResponse : public ResponseHeader<HeartbeatResponse> {
        std::string info;// 10个字节的消息, 未解

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream stream(data);
            info = stream.get_string(10);
        }

        std::string toStringImpl() {
            return fmt::format("Info: {}", info);
        }
    };

#pragma pack(pop)  // 恢复默认对齐方式
}

#endif //QUANT1X_LEVEL1_HEARTBEAT_H
