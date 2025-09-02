#pragma once
#ifndef QUANT1X_LEVEL1_FINANCE_INFO_H
#define QUANT1X_LEVEL1_FINANCE_INFO_H 1

#include "protocol.h"
#include <ostream>

// ==============================
// 企业财务信息
// ==============================

namespace level1 {

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐
    struct FinanceRequest : public RequestHeader<FinanceRequest> {
        u16 Count;
        u8 Market;
        char Code[6]{};

        FinanceRequest(const std::string &securityCode) : RequestHeader<FinanceRequest>() {
            ZipFlag = ZlibFlag::NotZipped;
            SeqID = SequenceId();
            PacketType = 0x01;
            Method = StdCommand::FINANCE_INFO;
            auto [id, _, symbol] = exchange::DetectMarket(securityCode);
            Count = 1;
            Market = id;
            const char * const tmp = symbol.c_str();
            std::memcpy(Code, tmp, sizeof(Code));
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1 = 2 + 2 + 1 + 6;
            PkgLen2 = 2 + 2 + 1 + 6;
            auto buf = RequestHeader<FinanceRequest>::headerSerialize();
            BinaryStream stream;
            stream.push_arithmetic(Count);
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream oss;
            oss << RequestHeader<FinanceRequest>::headerStringImpl();
            oss << '{';
            oss << "Count:" << Count;
            oss << ", Market:" << (int)Market;
            oss << ", Code:" << std::string(Code, sizeof(Code));
            oss << '}';
            return oss.str();
        }
    };

    struct RawFinanceInfo {
        uint8_t Market;           // 市场
        char Code[6];             // 股票代码
        f32 LiuTongGuBen;         // 流通股本
        u16 Province;             // 所属省份
        u16 Industry;             // 行业
        u32 UpdatedDate;          // 更新日期
        u32 IPODate;              // 上市日期
        f32 ZongGuBen;            // 总股本
        f32 GuoJiaGu;             // 国家股
        f32 FaQiRenFaRenGu;       // 发起人法人股
        f32 FaRenGu;              // 法人股
        f32 BGu;                  // B股
        f32 HGu;                  // H股
        f32 ZhiGongGu;            // 职工股
        f32 ZongZiChan;           // 总资产
        f32 LiuDongZiChan;        // 流动资产
        f32 GuDingZiChan;         // 固定资产
        f32 WuXingZiChan;         // 无形资产
        f32 GuDongRenShu;         // 股东人数
        f32 LiuDongFuZhai;        // 流动负债
        f32 ChangQiFuZhai;        // 长期负债
        f32 ZiBenGongJiJin;       // 资本公积金
        f32 JingZiChan;           // 净资产
        f32 ZhuYingShouRu;        // 主营收入
        f32 ZhuYingLiRun;         // 主营利润
        f32 YingShouZhangKuan;    // 应收账款
        f32 YingYeLiRun;          // 营业利润
        f32 TouZiShouYu;          // 投资收益
        f32 JingYingXianJinLiu;   // 经营现金流量
        f32 ZongXianJinLiu;       // 总现金流量
        f32 CunHuo;               // 存货
        f32 LiRunZongHe;          // 利润总额
        f32 ShuiHouLiRun;         // 税后利润
        f32 JingLiRun;            // 净利润
        f32 WeiFenLiRun;          // 未分配利润
        f32 BaoLiu1;              // 保留字段1
        f32 BaoLiu2;              // 保留字段2
        // char BaoLiu3[7];         // 保留字段3（可选）

        // 解码方法（成员方法）
        void decode(BinaryStream &bs) {

            // 1. 读取 Market
            Market = bs.get_arithmetic<uint8_t>();

            // 2. 读取 Code (固定长度数组)
            bs.get_array(Code);
            //bs.get_array(Code);
            // 3. 读取 LiuTongGuBen
            LiuTongGuBen = bs.get_arithmetic<float>();

            // 4. 读取 Province
            Province = bs.get_arithmetic<uint16_t>();

            // 5. 读取 Industry
            Industry = bs.get_arithmetic<uint16_t>();

            // 6. 读取 UpdatedDate
            UpdatedDate = bs.get_arithmetic<uint32_t>();

            // 7. 读取 IPODate
            IPODate = bs.get_arithmetic<uint32_t>();

            // 8. 读取 ZongGuBen
            ZongGuBen = bs.get_arithmetic<float>();

            // 9. 读取 GuoJiaGu
            GuoJiaGu = bs.get_arithmetic<float>();

            // 10. 读取 FaQiRenFaRenGu
            FaQiRenFaRenGu = bs.get_arithmetic<float>();

            // 11. 读取 FaRenGu
            FaRenGu = bs.get_arithmetic<float>();

            // 12. 读取 BGu
            BGu = bs.get_arithmetic<float>();

            // 13. 读取 HGu
            HGu = bs.get_arithmetic<float>();

            // 14. 读取 ZhiGongGu
            ZhiGongGu = bs.get_arithmetic<float>();

            // 15. 读取 ZongZiChan
            ZongZiChan = bs.get_arithmetic<float>();

            // 16. 读取 LiuDongZiChan
            LiuDongZiChan = bs.get_arithmetic<float>();

            // 17. 读取 GuDingZiChan
            GuDingZiChan = bs.get_arithmetic<float>();

            // 18. 读取 WuXingZiChan
            WuXingZiChan = bs.get_arithmetic<float>();

            // 19. 读取 GuDongRenShu
            GuDongRenShu = bs.get_arithmetic<float>();

            // 20. 读取 LiuDongFuZhai
            LiuDongFuZhai = bs.get_arithmetic<float>();

            // 21. 读取 ChangQiFuZhai
            ChangQiFuZhai = bs.get_arithmetic<float>();

            // 22. 读取 ZiBenGongJiJin
            ZiBenGongJiJin = bs.get_arithmetic<float>();

            // 23. 读取 JingZiChan
            JingZiChan = bs.get_arithmetic<float>();

            // 24. 读取 ZhuYingShouRu
            ZhuYingShouRu = bs.get_arithmetic<float>();

            // 25. 读取 ZhuYingLiRun
            ZhuYingLiRun = bs.get_arithmetic<float>();

            // 26. 读取 YingShouZhangKuan
            YingShouZhangKuan = bs.get_arithmetic<float>();

            // 27. 读取 YingYeLiRun
            YingYeLiRun = bs.get_arithmetic<float>();

            // 28. 读取 TouZiShouYu
            TouZiShouYu = bs.get_arithmetic<float>();

            // 29. 读取 JingYingXianJinLiu
            JingYingXianJinLiu = bs.get_arithmetic<float>();

            // 30. 读取 ZongXianJinLiu
            ZongXianJinLiu = bs.get_arithmetic<float>();

            // 31. 读取 CunHuo
            CunHuo = bs.get_arithmetic<float>();

            // 32. 读取 LiRunZongHe
            LiRunZongHe = bs.get_arithmetic<float>();

            // 33. 读取 ShuiHouLiRun
            ShuiHouLiRun = bs.get_arithmetic<float>();

            // 34. 读取 JingLiRun
            JingLiRun = bs.get_arithmetic<float>();

            // 35. 读取 WeiFenLiRun
            WeiFenLiRun = bs.get_arithmetic<float>();

            // 36. 读取 BaoLiu1
            BaoLiu1 = bs.get_arithmetic<float>();

            // 37. 读取 BaoLiu2
            BaoLiu2 = bs.get_arithmetic<float>();
        }
    };

    /// 财务数据
    struct FinanceInfo {
        std::string Code;             // 股票代码
        f64 LiuTongGuBen;             // 流通股本
        u16 Province;                 // 所属省份
        u16 Industry;                 // 行业
        u32 UpdatedDate;              // 更新日期
        u32 IPODate;                  // 上市日期
        f64 ZongGuBen;                // 总股本
        f64 GuoJiaGu;                 // 国家股
        f64 FaQiRenFaRenGu;           // 发起人法人股
        f64 FaRenGu;                  // 法人股
        f64 BGu;                      // B股
        f64 HGu;                      // H股
        f64 ZhiGongGu;                // 职工股
        f64 ZongZiChan;               // 总资产
        f64 LiuDongZiChan;            // 流动资产
        f64 GuDingZiChan;             // 固定资产
        f64 WuXingZiChan;             // 无形资产
        f64 GuDongRenShu;             // 股东人数
        f64 LiuDongFuZhai;            // 流动负债
        f64 ChangQiFuZhai;            // 长期负债
        f64 ZiBenGongJiJin;           // 资本公积金
        f64 JingZiChan;               // 净资产
        f64 ZhuYingShouRu;            // 主营收入
        f64 ZhuYingLiRun;             // 主营利润
        f64 YingShouZhangKuan;        // 应收账款
        f64 YingYeLiRun;              // 营业利润
        f64 TouZiShouYu;              // 投资收益
        f64 JingYingXianJinLiu;       // 经营现金流量
        f64 ZongXianJinLiu;           // 总现金流量
        f64 CunHuo;                   // 存货
        f64 LiRunZongHe;              // 利润总额
        f64 ShuiHouLiRun;             // 税后利润
        f64 JingLiRun;                // 净利润
        f64 WeiFenLiRun;              // 未分配利润
        f64 MeiGuJingZiChan;          // 每股净资产
        f64 BaoLiu2;                  // 保留字段2

        // 是否退市
        bool isDelisting() const {
            return IPODate == 0 && ZongGuBen == 0 && LiuTongGuBen == 0;
        }

        friend std::ostream &operator<<(std::ostream &os, const FinanceInfo &info) {
            os << "Code: " << info.Code << " LiuTongGuBen: " << info.LiuTongGuBen << " Province: " << info.Province
               << " Industry: " << info.Industry << " UpdatedDate: " << info.UpdatedDate << " IPODate: " << info.IPODate
               << " ZongGuBen: " << info.ZongGuBen << " GuoJiaGu: " << info.GuoJiaGu << " FaQiRenFaRenGu: "
               << info.FaQiRenFaRenGu << " FaRenGu: " << info.FaRenGu << " BGu: " << info.BGu << " HGu: " << info.HGu
               << " ZhiGongGu: " << info.ZhiGongGu << " ZongZiChan: " << info.ZongZiChan << " LiuDongZiChan: "
               << info.LiuDongZiChan << " GuDingZiChan: " << info.GuDingZiChan << " WuXingZiChan: " << info.WuXingZiChan
               << " GuDongRenShu: " << info.GuDongRenShu << " LiuDongFuZhai: " << info.LiuDongFuZhai
               << " ChangQiFuZhai: " << info.ChangQiFuZhai << " ZiBenGongJiJin: " << info.ZiBenGongJiJin
               << " JingZiChan: " << info.JingZiChan << " ZhuYingShouRu: " << info.ZhuYingShouRu << " ZhuYingLiRun: "
               << info.ZhuYingLiRun << " YingShouZhangKuan: " << info.YingShouZhangKuan << " YingYeLiRun: "
               << info.YingYeLiRun << " TouZiShouYu: " << info.TouZiShouYu << " JingYingXianJinLiu: "
               << info.JingYingXianJinLiu << " ZongXianJinLiu: " << info.ZongXianJinLiu << " CunHuo: " << info.CunHuo
               << " LiRunZongHe: " << info.LiRunZongHe << " ShuiHouLiRun: " << info.ShuiHouLiRun << " JingLiRun: "
               << info.JingLiRun << " WeiFenLiRun: " << info.WeiFenLiRun << " MeiGuJingZiChan: " << info.MeiGuJingZiChan
               << " BaoLiu2: " << info.BaoLiu2;
            return os;
        }
    };

    struct FinanceResponse : public ResponseHeader<FinanceResponse> {
        u16 Count; //  总数
        FinanceInfo Info;

        explicit FinanceResponse() : Count(0),Info({}) {}

        void deserializeImpl(const std::vector<u8> &body) {
            BinaryStream bs(body);
            Count = bs.get_u16(); // 总数
            if(Count == 0) {
                return;
            }
            RawFinanceInfo raw{};
            raw.decode(bs);
            const static int baseUnit = 10000;
            auto symbol = strings::from(raw.Code);
            Info.Code = exchange::GetSecurityCode(static_cast<exchange::MarketType>(raw.Market), std::string(raw.Code, sizeof(raw.Code)));
            Info.LiuTongGuBen = encoding::NumberToFloat64(raw.LiuTongGuBen) * baseUnit;
            Info.Province = raw.Province;
            Info.Industry = raw.Industry;
            Info.UpdatedDate = raw.UpdatedDate;
            Info.IPODate = raw.IPODate;
            Info.ZongGuBen = encoding::NumberToFloat64(raw.ZongGuBen) * baseUnit;
            Info.GuoJiaGu = encoding::NumberToFloat64(raw.GuoJiaGu) * baseUnit;
            Info.FaQiRenFaRenGu = encoding::NumberToFloat64(raw.FaQiRenFaRenGu) * baseUnit;
            Info.FaRenGu = encoding::NumberToFloat64(raw.FaRenGu) * baseUnit;
            Info.BGu = encoding::NumberToFloat64(raw.BGu) * baseUnit;
            Info.HGu = encoding::NumberToFloat64(raw.HGu) * baseUnit;
            Info.ZhiGongGu = encoding::NumberToFloat64(raw.ZhiGongGu) * baseUnit;
            Info.ZongZiChan = encoding::NumberToFloat64(raw.ZongZiChan) * baseUnit;
            Info.LiuDongZiChan = encoding::NumberToFloat64(raw.LiuDongZiChan) * baseUnit;
            Info.GuDingZiChan = encoding::NumberToFloat64(raw.GuDingZiChan) * baseUnit;
            Info.WuXingZiChan = encoding::NumberToFloat64(raw.WuXingZiChan) * baseUnit;
            Info.GuDongRenShu = encoding::NumberToFloat64(raw.GuDongRenShu);
            Info.LiuDongFuZhai = encoding::NumberToFloat64(raw.LiuDongFuZhai) * baseUnit;
            Info.ChangQiFuZhai = encoding::NumberToFloat64(raw.ChangQiFuZhai) * baseUnit;
            Info.ZiBenGongJiJin = encoding::NumberToFloat64(raw.ZiBenGongJiJin) * baseUnit;
            Info.JingZiChan = encoding::NumberToFloat64(raw.JingZiChan) * baseUnit;
            Info.ZhuYingShouRu = encoding::NumberToFloat64(raw.ZhuYingShouRu) * baseUnit;
            Info.ZhuYingLiRun = encoding::NumberToFloat64(raw.ZhuYingLiRun) * baseUnit;
            Info.YingShouZhangKuan = encoding::NumberToFloat64(raw.YingShouZhangKuan) * baseUnit;
            Info.YingYeLiRun = encoding::NumberToFloat64(raw.YingYeLiRun) * baseUnit;
            Info.TouZiShouYu = encoding::NumberToFloat64(raw.TouZiShouYu) * baseUnit;
            Info.JingYingXianJinLiu = encoding::NumberToFloat64(raw.JingYingXianJinLiu) * baseUnit;
            Info.ZongXianJinLiu = encoding::NumberToFloat64(raw.ZongXianJinLiu) * baseUnit;
            Info.CunHuo = encoding::NumberToFloat64(raw.CunHuo) * baseUnit;
            Info.LiRunZongHe = encoding::NumberToFloat64(raw.LiRunZongHe) * baseUnit;
            Info.ShuiHouLiRun = encoding::NumberToFloat64(raw.ShuiHouLiRun) * baseUnit;
            Info.JingLiRun = encoding::NumberToFloat64(raw.JingLiRun) * baseUnit;
            Info.WeiFenLiRun = encoding::NumberToFloat64(raw.WeiFenLiRun) * baseUnit;
            Info.MeiGuJingZiChan = encoding::NumberToFloat64(raw.BaoLiu1) * baseUnit;
            Info.BaoLiu2 = encoding::NumberToFloat64(raw.BaoLiu2);
        }

        std::string toStringImpl() const {
            std::ostringstream out;
            out << ResponseHeader<FinanceResponse>::headerStringImpl()
            << "{Count:" << Count
            << ", Info:"<< Info
            << "}";
            return out.str();
        }
    };
#pragma pack(pop)  // 恢复默认对齐方式

}

#endif //QUANT1X_LEVEL1_FINANCE_INFO_H
