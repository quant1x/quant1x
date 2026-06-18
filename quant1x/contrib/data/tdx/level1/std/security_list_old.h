#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_SECURITY_LIST_OLD_H
#define QUANT1X_CONTRB_DATA_TDX_SECURITY_LIST_OLD_H 1

#include <quant1x/contrib/data/tdx/helpers.h>
#include <quant1x/contrib/data/tdx/protocol.h>

#include <ostream>

// ==============================
// 证券列表, 不支持北京交易所, 上海和深圳两市的证券列表比较全
// ==============================

namespace quant1x::contrib::data::tdx {
    constexpr int old_security_list_pre_request_max = 1000;  ///< 单次最大获取多少条股票数据
    // 证券列表
    struct OldSecurity {
        std::string Code;          // 证券代码
        u16         VolUnit;       // 每手股数
        u8          Reversed1[4];  // 保留字段1
        u8          DecimalPoint;  // 小数点位数
        std::string Name;          // 证券名称
        f64         PreClose;      // 昨收价
        u8          Reversed2[4];  // 保留字段2

        friend std::ostream &operator<<(std::ostream &os, const OldSecurity &security) {
            os << "Code:" << security.Code << " VolUnit:" << security.VolUnit << " Reversed1:" << security.Reversed1
               << " DecimalPoint:" << security.DecimalPoint << " Name:" << security.Name
               << " PreClose:" << security.PreClose << " Reversed2:" << security.Reversed2;
            return os;
        }
    };

    // 证券列表(旧版) (对齐 Python SecurityList, 老版本协议)
    struct OldSecurityList : public BaseMessage<OldSecurityList> {
        u16 market;       // 市场
        u16 start;        // 起始位置

        u16                   Count;     // 响应: 返回记录数
        std::vector<OldSecurity> List;   // 响应: 证券列表

        OldSecurityList(int market, int start) : BaseMessage<OldSecurityList>() {
            request_header.frame_type      = ZlibFlag::Uncompressed;
            request_header.seq_id        = get_sequence_id();
            request_header.packet_ctrl   = 0x01;
            request_header.cmd_id       = StdCommand::SECURITY_LIST;
            this->market = market;
            this->start  = start;
        }

        std::vector<u8> serialize_request_body_impl() {
            BinaryStream tmp;
            tmp.push_arithmetic(market);
            tmp.push_arithmetic(start);
            return tmp.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            BinaryStream buf(data);
            Count = buf.get_u16();
            for (int index = 0; index < Count; index++) {
                auto e           = OldSecurity{};
                e.Code           = buf.get_string(6);
                e.VolUnit        = buf.get_u16();
                std::string Name = buf.get_string(8);
                e.Name           = charsets::gbk_to_utf8(Name);
                buf.get_array(e.Reversed1);
                e.DecimalPoint = buf.get_u8();
                u32 tmp        = buf.get_u32();
                e.PreClose     = helpers::integerToFloat64(tmp);
                buf.get_array(e.Reversed2);
                List.push_back(e);
            }
        }

        [[nodiscard]] std::string to_string_impl() const {
            std::ostringstream oss;
            oss << "Count:" << Count;
            return oss.str();
        }
    };
}  // namespace quant1x::contrib::data::tdx

#endif  // QUANT1X_CONTRB_DATA_TDX_SECURITY_LIST_OLD_H
