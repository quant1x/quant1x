#pragma once
#ifndef QUANT1X_LEVEL_BLOCK_INFO_H
#define QUANT1X_LEVEL_BLOCK_INFO_H 1

#include "protocol.h"
#include "block_meta.h"

// ==============================
// 板块数据
// ==============================

namespace level1 {

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    struct BlockInfo {
        std::string BlockName{};
        u16 BlockType = 0;
        u16 StockCount = 0;
        std::vector<std::string> Codelist{};

        friend std::ostream &operator<<(std::ostream &os, const BlockInfo &info) {
            os << "BlockName: " << info.BlockName << " BlockType: " << info.BlockType << " StockCount: "
               << info.StockCount << " Codelist: [" << strings::join(info.Codelist, ',');
            os << "]";
            return os;
        }
    };

    struct BlockInfoRequest : public RequestHeader<BlockInfoRequest> {
        u32 Start;
        u32 Size;
        char BlockFilename[100];

        BlockInfoRequest(const std::string &filename, u32 offset) : RequestHeader<BlockInfoRequest>(), BlockFilename() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x01;
            Method = StdCommand::BLOCK_DATA;

            Start = offset;
            Size = BLOCK_CHUNKS_SIZE;
            memset(BlockFilename, 0x00, sizeof(BlockFilename));
            strncpy(BlockFilename, filename.c_str(), sizeof(BlockFilename) - 1);
        }

        std::vector<u8> serializeImpl() {
            PkgLen1 = 0x6e;
            PkgLen2 = 0x6e;
            auto buf = RequestHeader<BlockInfoRequest>::headerSerialize();
            BinaryStream stream;
            stream.push_arithmetic(Start);
            stream.push_arithmetic(Size);
            stream.push_array(BlockFilename);
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<BlockInfoRequest>::headerStringImpl()
                << "{Start:" << Start
                << ", Size:" << Size
                << ", BlockFilename:" << strings::from(BlockFilename)
                << "}";

            return oss.str();
        }
    };

    struct BlockInfoResponse : public ResponseHeader<BlockInfoResponse> {
        u32 Size = 0;
        std::vector<u8> Data;

        void deserializeImpl(const std::vector<u8> &body) {
            BinaryStream bs(body);
            Size = bs.get_u32();
            if(Size > 0) {
                Data.reserve(Size);
                auto remain = bs.data();
                Data.insert(Data.end(), remain.begin()+ bs.position(), remain.end());
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << ResponseHeader<BlockInfoResponse>::headerStringImpl()
                << "{Size:" << Size;
            oss << "}";
            return oss.str();
        }
    };

#pragma pack(pop)  // 恢复默认对齐方式

}

#endif //QUANT1X_LEVEL_BLOCK_INFO_H
