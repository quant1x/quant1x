#pragma once
#ifndef QUANT1X_LEVEL1_SECURITY_LIST_OLD_H
#define QUANT1X_LEVEL1_SECURITY_LIST_OLD_H 1

#include <quant1x/level1/helpers.h>
#include <quant1x/level1/protocol.h>

#include <ostream>

// ==============================
// 证券列表, 不支持北京交易所, 上海和深圳两市的证券列表比较全
// ==============================

namespace level1 {
    constexpr int old_security_list_pre_request_max = 1000;  ///< 单次最大获取多少条股票数据
    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐
    // 证券列表
    struct OldSecurityListRequest : RequestHeader<OldSecurityListRequest> {
        u16 market;
        u16 start;

        OldSecurityListRequest(int market, int start) : RequestHeader<OldSecurityListRequest>() {
            ZipFlag      = ZlibFlag::Uncompressed;
            SeqID        = SequenceId();
            PacketType   = 0x01;
            Method       = StdCommand::SECURITY_LIST;
            this->market = market;
            this->start  = start;
        }

        std::vector<u8> serializeImpl() {
            BinaryStream tmp;
            PkgLen1 = 2 + 4;
            PkgLen2 = 2 + 4;
            tmp.push_arithmetic(market);
            tmp.push_arithmetic(start);
            auto buf  = RequestHeader<OldSecurityListRequest>::headerSerialize();
            auto data = tmp.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        [[nodiscard]] std::string toStringImpl() const { return ""; }
    };

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

    struct OldSecurityListResponse : public ResponseHeader<OldSecurityListResponse> {
        u16                   Count;
        std::vector<OldSecurity> List;

        void deserializeImpl(const std::vector<u8> &data) {
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

        std::string toStringImpl() const { return ""; }

        friend std::ostream &operator<<(std::ostream &os, const OldSecurityListResponse &response) {
            os << "Count:" << response.Count;
            return os;
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式
}  // namespace level1

#endif  // QUANT1X_LEVEL1_SECURITY_LIST_OLD_H
