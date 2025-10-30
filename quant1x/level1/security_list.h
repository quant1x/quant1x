#pragma once
#include <quant1x/std/base.h>
#ifndef QUANT1X_LEVEL1_SECURITY_LIST_H
#define QUANT1X_LEVEL1_SECURITY_LIST_H 1

#include <quant1x/level1/helpers.h>
#include <quant1x/level1/protocol.h>

#include <ostream>

// ==============================
// 证券列表
// ==============================

namespace level1 {
    constexpr int security_list_pre_request_max = 1600;  ///< 单次最大获取多少条股票数据
    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐
    // 证券列表
    struct SecurityListRequest : RequestHeader<SecurityListRequest> {
        u16 market;   // 交易市场
        u32 start;    // 起始位置
        u32 count;    // 请求数量
        u32 unknown;  // 未知字段，通常为0x00000000

        SecurityListRequest(int market, int start, int count) : RequestHeader<SecurityListRequest>() {
            ZipFlag       = ZlibFlag::NotZipped;
            SeqID         = SequenceId();
            PacketType    = 0x01;
            Method        = StdCommand::SECURITY_LIST;
            this->market  = market;
            this->start   = start;
            this->count   = count;
            this->unknown = 0;
        }

        std::vector<u8> serializeImpl() {
            BinaryStream tmp;
            PkgLen1 = 2 + 2 + 4 + 4 + 4;
            PkgLen2 = 2 + 2 + 4 + 4 + 4;
            tmp.push_arithmetic(market);
            tmp.push_arithmetic(start);
            tmp.push_arithmetic(count);
            tmp.push_arithmetic(unknown);
            auto buf  = RequestHeader<SecurityListRequest>::headerSerialize();
            auto data = tmp.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        [[nodiscard]] std::string toStringImpl() const { return ""; }
    };

    struct Security {
        std::string Code;          // 证券代码
        u16         VolUnit;       // 每手股数
        std::string Name;          // 证券名称
        //u8          Reversed1[8];  // 保留字段1
        u8          Reversed2[4];  // 保留字段2
        u8          DecimalPoint;  // 小数点位数
        f64         PreClose;      // 昨收价
        u8          Reversed3[4];  // 保留字段3

        friend std::ostream &operator<<(std::ostream &os, const Security &security) {
            os << "Code:" << security.Code
               << " VolUnit:" << security.VolUnit
               << " Name:" << security.Name
               //<< " Reversed1:" << security.Reversed1
               << " Reversed2:" << security.Reversed2
               << " DecimalPoint:" << security.DecimalPoint
               << " PreClose:" << security.PreClose
               << " Reversed3:" << security.Reversed3;
            return os;
        }
    };

    struct SecurityListResponse : public ResponseHeader<SecurityListResponse> {
        u16                   Count;
        std::vector<Security> List;

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream buf(data);
            Count = buf.get_u16();
            for (int index = 0; index < Count; index++) {
                auto e           = Security{};
                e.Code           = buf.get_string(6);
                e.VolUnit        = buf.get_u16();
                std::string Name = buf.get_string(8+8);
                e.Name           = charsets::gbk_to_utf8(Name);
                //buf.get_array(e.Reversed1);
                buf.get_array(e.Reversed2);
                e.DecimalPoint = buf.get_u8();
                u32 tmp        = buf.get_u32();
                e.PreClose     = helpers::IntToFloat64(tmp);
                buf.get_array(e.Reversed3);
                List.push_back(e);
            }
        }

        std::string toStringImpl() const { return ""; }

        friend std::ostream &operator<<(std::ostream &os, const SecurityListResponse &response) {
            os << "Count:" << response.Count;
            return os;
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式
}  // namespace level1

#endif  // QUANT1X_LEVEL1_SECURITY_LIST_H
