#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_BLOCK_META_H
#define QUANT1X_CONTRB_DATA_TDX_BLOCK_META_H 1

#include <quant1x/contrib/data/tdx/protocol.h>

// ==============================
// 板块元数据
// ==============================

namespace quant1x::contrib::data::tdx {

    constexpr const char* const BLOCK_ZHISHU      = "block_zs.dat";  // 指数
    constexpr const char* const BLOCK_FENGGE      = "block_fg.dat";  // 风格
    constexpr const char* const BLOCK_GAINIAN     = "block_gn.dat";  // 概念
    constexpr const char* const BLOCK_DEFAULT     = "block.dat";     // 早期的板块数据文件, 与block_zs.dat
    constexpr u16         BLOCK_CHUNKS_SIZE = 0x7530;          // 板块文件默认一个请求包最大数据

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

    // 板块元数据请求/响应 (对齐 Python BlockMetaRequest)
    struct BlockMetaMsg : public BaseMessage<BlockMetaMsg> {
        char BlockFilename[40];       // 请求: 板块文件名
        BlockMeta Meta{};             // 响应: 板块元数据

        BlockMetaMsg(const std::string &filename) : BaseMessage<BlockMetaMsg>() {
            request_header.frame_type    = ZlibFlag::Uncompressed;
            request_header.seq_id      = get_sequence_id();
            request_header.packet_ctrl = 0x01;
            request_header.cmd_id     = StdCommand::BLOCK_META;

            memset(BlockFilename, 0x00, sizeof(BlockFilename));
            std::strncpy(BlockFilename, filename.c_str(), sizeof(BlockFilename) - 1);
        }

        std::vector<u8> serialize_request_body_impl() {
            std::vector<u8> buf;
            buf.insert(buf.end(), std::begin(BlockFilename), std::end(BlockFilename));
            return buf;
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream bs(data);
            Meta.Size = bs.get_u32();
            Meta.C1   = bs.get_u8();
            bs.get_array(Meta.HashValue);
            Meta.C2 = bs.get_u8();
        }

        std::string to_string_impl() const {
            std::ostringstream oss;
            oss << request_header.header_string_impl();
            oss << "{BlockFilename:" << strings::from(BlockFilename) << "}"
                << "{" << Meta << "}";
            return oss.str();
        }
    };

}  // namespace quant1x::contrib::data::tdx

#endif  // QUANT1X_CONTRB_DATA_TDX_BLOCK_META_H
