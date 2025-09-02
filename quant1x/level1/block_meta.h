#pragma once
#ifndef QUANT1X_LEVEL1_BLOCK_META_H
#define QUANT1X_LEVEL1_BLOCK_META_H 1

#include "protocol.h"

// ==============================
// 板块元数据
// ==============================

namespace level1 {

    constexpr const char* const BLOCK_ZHISHU      = "block_zs.dat";  // 指数
    constexpr const char* const BLOCK_FENGGE      = "block_fg.dat";  // 风格
    constexpr const char* const BLOCK_GAINIAN     = "block_gn.dat";  // 概念
    constexpr const char* const BLOCK_DEFAULT     = "block.dat";     // 早期的板块数据文件, 与block_zs.dat
    constexpr u16         BLOCK_CHUNKS_SIZE = 0x7530;          // 板块文件默认一个请求包最大数据

/// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    // BlockMeta 响应包结构
    struct BlockMeta {
        u32 Size;           // 尺寸
        u8  C1;             // C1
        u8  HashValue[32];  // hash值
        u8  C2;             // C2

        friend std::ostream &operator<<(std::ostream &os, const BlockMeta &meta) {
            os << "Size: " << meta.Size << " C1: " << int(meta.C1) << " HashValue: " << strings::from(meta.HashValue)
               << " C2: " << int(meta.C2);
            return os;
        }
    };

    struct BlockMetaRequest : public RequestHeader<BlockMetaRequest> {
        char BlockFilename[40];

        BlockMetaRequest(const std::string &filename) : RequestHeader<BlockMetaRequest>() {
            ZipFlag    = ZlibFlag::NotZipped;
            SeqID      = SequenceId();
            PacketType = 0x01;
            Method     = StdCommand::BLOCK_META;

            memset(BlockFilename, 0x00, sizeof(BlockFilename));
            std::strncpy(BlockFilename, filename.c_str(), sizeof(BlockFilename) - 1);
        }

        std::vector<u8> serializeImpl() {
            PkgLen1  = 0x2a;
            PkgLen2  = 0x2a;
            auto buf = RequestHeader<BlockMetaRequest>::headerSerialize();
            buf.insert(buf.end(), std::begin(BlockFilename), std::end(BlockFilename));
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<BlockMetaRequest>::headerStringImpl();
            oss << "{BlockFilename:" << strings::from(BlockFilename) << "}";
            return oss.str();
        }
    };

    struct BlockMetaResponse : public ResponseHeader<BlockMetaResponse> {
        BlockMeta Meta{};

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Meta.Size = bs.get_u32();
            Meta.C1   = bs.get_u8();
            bs.get_array(Meta.HashValue);
            Meta.C2 = bs.get_u8();
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << ResponseHeader<BlockMetaResponse>::headerStringImpl() << "{" << Meta << "}";
            return oss.str();
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式

}  // namespace level1

#endif  // QUANT1X_LEVEL1_BLOCK_META_H
