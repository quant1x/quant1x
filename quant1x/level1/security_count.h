#pragma once
#ifndef QUANT1X_LEVEL1_SECURITY_COUNT_H
#define QUANT1X_LEVEL1_SECURITY_COUNT_H 1

// ==============================
// 证券统计
// ==============================

#include "protocol.h"

namespace level1 {

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    struct SecurityCountReqeust : public RequestHeader<SecurityCountReqeust> {
        u16 Market;
        std::vector<u8> padding={};

        SecurityCountReqeust() : RequestHeader<SecurityCountReqeust>(){
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x01;
            Method = StdCommand::SECURITY_COUNT;

            Market = 0;
            padding = strings::hexToBytes("75c73301");
        }

        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + 2 + 4;
            PkgLen2 = 2 + 2 + 4;
            auto buf = RequestHeader<SecurityCountReqeust>::headerSerialize();
            BinaryStream stream;
            stream.push_arithmetic(Market);
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            buf.insert(buf.end(), padding.begin(), padding.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<SecurityCountReqeust>::headerStringImpl()
                << '{'
                << "Market:"<< int(Market)
                << ", padding:" << strings::bytesToHex(padding)
                << '}';
            return oss.str();
        }

    };

    struct SecurityCountResponse : public ResponseHeader<SecurityCountResponse> {
        u16 Count;

        SecurityCountResponse() : ResponseHeader<SecurityCountResponse>() {
            Count = 0;
        }

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Count = bs.get_u16();
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << ResponseHeader<SecurityCountResponse>::headerStringImpl()
                << "{Count:" << Count
                << "}";
            return oss.str();
        }
    };

#pragma pack(pop)  // 恢复默认对齐方式

}
#endif //QUANT1X_LEVEL1_SECURITY_COUNT_H
