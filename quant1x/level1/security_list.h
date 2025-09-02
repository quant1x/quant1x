#pragma once
#ifndef QUANT1X_LEVEL1_SECURITY_LIST_H
#define QUANT1X_LEVEL1_SECURITY_LIST_H 1

#include <ostream>
#include "protocol.h"
#include "encoding.h"

// ==============================
// 证券列表
// ==============================

namespace level1 {
    constexpr int security_list_max = 1000; ///< 单次最大获取多少条股票数据
    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐
    // 证券列表
    struct SecurityListRequest : RequestHeader<SecurityListRequest> {
        u16 market;
        u16 start;

        SecurityListRequest(int market, int start) : RequestHeader<SecurityListRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x01;
            Method = StdCommand::SECURITY_LIST;
            this->market = market;
            this->start = start;
        }

        std::vector<u8> serializeImpl() {
            BinaryStream tmp;
            PkgLen1 = 2 + 4;
            PkgLen2 = 2 + 4;
            tmp.push_arithmetic(market);
            tmp.push_arithmetic(start);
            auto buf = RequestHeader<SecurityListRequest>::headerSerialize();
            auto data = tmp.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        [[nodiscard]] std::string toStringImpl() const {
            return "";
        }
    };

    struct Security {
        std::string Code;
        u16 VolUnit;
        u8 Reversed1[4];
        u8 DecimalPoint;
        std::string Name;
        f64 PreClose;
        u8 Reversed2[4];

        friend std::ostream &operator<<(std::ostream &os, const Security &security) {
            os << "Code:" << security.Code << " VolUnit:" << security.VolUnit << " Reversed1:" << security.Reversed1
               << " DecimalPoint:" << security.DecimalPoint << " Name:" << security.Name << " PreClose:"
               << security.PreClose << " Reversed2:" << security.Reversed2;
            return os;
        }
    };

    struct SecurityListResponse : public ResponseHeader<SecurityListResponse> {
        u16 Count;
        std::vector<Security> List;

        void deserializeImpl(const std::vector<u8> &data) {
            BinaryStream buf(data);
            Count = buf.get_u16();
            //std::cout << Count << std::endl;
            for (int index = 0; index < Count; index++) {
                auto e = Security{};
                e.Code = buf.get_string(6);
                e.VolUnit = buf.get_u16();
                std::string Name = buf.get_string(8);
                e.Name = charsets::gbk_to_utf8(Name);
                buf.get_array(e.Reversed1);
                e.DecimalPoint = buf.get_u8();
                u32 tmp = buf.get_u32();
                e.PreClose = encoding::IntToFloat64(tmp);
                buf.get_array(e.Reversed2);
                List.push_back(e);
                //std::cout <<"offset:"<<buf.position() << " ,data="<< e << std::endl;
            }
        }

        std::string toStringImpl() const {
            return "";
        }

        friend std::ostream &operator<<(std::ostream &os, const SecurityListResponse &response) {
            os << "Count:" << response.Count;
            return os;
        }
    };
#pragma pack(pop)      // 恢复默认对齐方式
}

#endif //QUANT1X_LEVEL1_SECURITY_LIST_H
