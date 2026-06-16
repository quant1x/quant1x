#pragma once
#ifndef QUANT1X_LEVEL_BLOCK_INFO_H
#define QUANT1X_LEVEL_BLOCK_INFO_H 1

#include <quant1x/contrib/data/tdx/level1/protocol.h>
#include <quant1x/contrib/data/tdx/level1//block_meta.h>

// ==============================
// 板块数据
// ==============================

namespace level1 {

    /// 板块数据结构
    struct BlockEntry {
        std::string BlockName{};
        u16 BlockType = 0;
        u16 StockCount = 0;
        std::vector<std::string> Codelist{};

        friend std::ostream &operator<<(std::ostream &os, const BlockEntry &info) {
            os << "BlockName: " << info.BlockName << " BlockType: " << info.BlockType << " StockCount: "
               << info.StockCount << " Codelist: [" << strings::join(info.Codelist, ',');
            os << "]";
            return os;
        }
    };

    // 板块数据请求/响应 (对齐 Python BlockInfo)
    struct BlockInfoMsg : public BaseMessage<BlockInfoMsg> {
        u32 Start;                       // 请求: 起始偏移
        u32 Size;                        // 请求: 数据块大小
        char BlockFilename[100];         // 请求: 板块文件名
        u32 DataSize = 0;                // 响应: 数据大小
        std::vector<u8> Data;            // 响应: 板块数据

        BlockInfoMsg(const std::string &filename, u32 offset) : BaseMessage<BlockInfoMsg>(), BlockFilename() {
            request_header.ZipFlag = ZlibFlag::Uncompressed;
            request_header.SeqID = SequenceId();
            request_header.PacketType = 0x01;
            request_header.Method = StdCommand::BLOCK_DATA;

            Start = offset;
            Size = BLOCK_CHUNKS_SIZE;
            memset(BlockFilename, 0x00, sizeof(BlockFilename));
            strncpy(BlockFilename, filename.c_str(), sizeof(BlockFilename) - 1);
        }

        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_arithmetic(Start);
            stream.push_arithmetic(Size);
            stream.push_array(BlockFilename);
            return stream.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &body) {
            BinaryStream bs(body);
            DataSize = bs.get_u32();
            if(DataSize > 0) {
                Data.reserve(DataSize);
                auto remain = bs.data();
                Data.insert(Data.end(), remain.begin()+ bs.position(), remain.end());
            }
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << request_header.headerStringImpl()
                << "{Start:" << Start
                << ", Size:" << Size
                << ", BlockFilename:" << strings::from(BlockFilename)
                << "}"
                << " DataSize:" << DataSize;
            return oss.str();
        }
    };

}

#endif //QUANT1X_LEVEL_BLOCK_INFO_H
