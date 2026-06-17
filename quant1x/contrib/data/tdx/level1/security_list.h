#pragma once
#include <quant1x/std/base.h>
#ifndef QUANT1X_LEVEL1_SECURITY_LIST_H
#define QUANT1X_LEVEL1_SECURITY_LIST_H 1

#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/contrib/data/tdx/protocol.h>

#include <ostream>

// ==============================
// 证券列表
// ==============================

namespace level1 {
    constexpr int security_list_pre_request_max = 1600;  ///< 单次最大获取多少条股票数据
    // 证券列表
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

    // 证券列表请求/响应 (对齐 Python SecurityList)
    struct SecurityList : public BaseMessage<SecurityList> {
        u16 market;   // 交易市场
        u32 start;    // 起始位置
        u32 count;    // 请求数量
        u32 unknown;  // 未知字段, 通常为0x00000000

        u16                   Count;     // 响应: 返回记录数
        std::vector<Security> List;      // 响应: 证券列表

        SecurityList(int market, int start, int count) : BaseMessage<SecurityList>() {
            request_header.ZipFlag       = ZlibFlag::Uncompressed;
            request_header.SeqID         = SequenceId();
            request_header.PacketType    = 0x01;
            request_header.Method        = StdCommand::SECURITY_LIST;
            this->market  = market;
            this->start   = start;
            this->count   = count;
            this->unknown = 0;
        }

        std::vector<u8> serialize_request_body_impl() {
            BinaryStream tmp;
            tmp.push_arithmetic(market);
            tmp.push_arithmetic(start);
            tmp.push_arithmetic(count);
            tmp.push_arithmetic(unknown);
            return tmp.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
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
                e.PreClose     = helpers::integerToFloat64(tmp);
                buf.get_array(e.Reversed3);
                List.push_back(e);
            }
        }

        [[nodiscard]] std::string toStringImpl() const {
            std::ostringstream oss;
            oss << "Count:" << Count;
            return oss.str();
        }
    };
}  // namespace level1

#endif  // QUANT1X_LEVEL1_SECURITY_LIST_H
