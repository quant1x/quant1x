#pragma once
#ifndef QUANT1X_LEVEL1_XDXR_INFO_H
#define QUANT1X_LEVEL1_XDXR_INFO_H 1

#include <quant1x/data/exchange/code.h>
#include <quant1x/contrib/data/tdx/level1//protocol.h>

#include <ostream>

// ==============================
// 除权除息
// ==============================

namespace level1 {

    // XDXR类型映射表
    enum XdxrCategory : int {
        EX_DIVIDEND                       = 1,   // 除权除息
        BONUS_SHARES_LISTING              = 2,   // 送股上市（无偿）
        RESTRICTED_SHARES_LISTING         = 3,   // 非流通股上市（受限股解禁）
        UNSPECIFIED_CAPITAL_ADJUSTMENT    = 4,   // 未知股本变动
        GENERAL_CAPITAL_ADJUSTMENT        = 5,   // 股本变化（保留，但慎用）
        NEW_SHARE_ISSUANCE                = 6,   // 增发新股
        SHARE_REPURCHASE                  = 7,   // 股份回购
        NEW_SHARES_LISTING                = 8,   // 增发新股上市
        TRANSFERRED_RIGHTS_SHARES_LISTING = 9,   // 转配股上市（中国特有）
        CONVERTIBLE_BOND_LISTING          = 10,  // 可转债上市
        STOCK_SPLIT_OR_REVERSE_SPLIT      = 11,  // 拆股或合股
        RESTRICTED_SHARES_CONSOLIDATION   = 12,  // 非流通股缩股
        ISSUE_CALL_WARRANTS               = 13,  // 送认购权证
        ISSUE_PUT_WARRANTS                = 14   // 送认沽权证
    };

    // 将枚举值转换为描述文本
    inline std::string to_string(XdxrCategory c) {
        switch (c) {
            case EX_DIVIDEND:                       return "除权除息";
            case BONUS_SHARES_LISTING:              return "送配股上市";
            case RESTRICTED_SHARES_LISTING:         return "非流通股上市";
            case UNSPECIFIED_CAPITAL_ADJUSTMENT:    return "未知股本变动";
            case GENERAL_CAPITAL_ADJUSTMENT:        return "股本变化";
            case NEW_SHARE_ISSUANCE:                return "增发新股";
            case SHARE_REPURCHASE:                  return "股份回购";
            case NEW_SHARES_LISTING:                return "增发新股上市";
            case TRANSFERRED_RIGHTS_SHARES_LISTING: return "转配股上市";
            case CONVERTIBLE_BOND_LISTING:          return "可转债上市";
            case STOCK_SPLIT_OR_REVERSE_SPLIT:      return "扩缩股";
            case RESTRICTED_SHARES_CONSOLIDATION:   return "非流通股缩股";
            case ISSUE_CALL_WARRANTS:               return "送认购权证";
            case ISSUE_PUT_WARRANTS:                return "送认沽权证";
            default:                                return fmt::format("Unknown({})", static_cast<int>(c));
        }
    }

    /// 网络协议
#pragma pack(push, 1)  // 确保1字节对齐

    // 除权除息请求结构
    struct XdxrInfoRequest : public RequestHeader<XdxrInfoRequest> {
        u8              Market;   // 市场代码 0:深圳 1:上海
        char            Code[6];  // 股票代码
        std::vector<u8> padding;

        XdxrInfoRequest(const std::string &securityCode) : RequestHeader<XdxrInfoRequest>() {
            ZipFlag               = ZlibFlag::Uncompressed;
            SeqID                 = SequenceId();
            PacketType            = 0x01;
            Method                = StdCommand::XDXR_INFO;
            auto [id, _, symbol]  = exchange::DetectMarket(securityCode);
            Market                = static_cast<u8>(id);
            const char *const tmp = symbol.c_str();
            std::memcpy(Code, tmp, sizeof(Code));
            padding = strings::hexToBytes("0100");
        }

        // 序列化方法
        std::vector<u8> serializeImpl() {
            PkgLen1          = 2 + 1 + 6 + 2;
            PkgLen2          = 2 + 1 + 6 + 2;
            auto         buf = RequestHeader<XdxrInfoRequest>::headerSerialize();
            BinaryStream stream;
            stream.push_byte_array(padding.data(), padding.size());
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            auto data = stream.data();
            buf.insert(buf.end(), data.begin(), data.end());
            return buf;
        }

        std::string toStringImpl() const {
            std::ostringstream out;
            out << RequestHeader<XdxrInfoRequest>::headerStringImpl();
            out << '{';
            out << "Market:" << (int)Market;
            out << ", Code:" << std::string(Code, sizeof(Code));
            out << ", padding:" << strings::bytesToHex(padding);
            out << '}';
            return out.str();
        }
    };

    // 原始除权除息信息
    struct RawXdxrInfo {
        u8   Market;    // 市场代码
        char Code[6];   // 股票代码
        u8   Unknown;   // 保留字段
        u32  Date;      // 日期(YYYYMMDD格式)
        u8   Category;  // 类型编号
        u8   Data[16];  // 数据内容(根据类型不同解析方式不同)
    };

    // 除权除息响应结构
    struct XdxrInfoReply {
        u8                       Unknown[9];  // 保留字段
        u16                      Count;       // 数据条数
        std::vector<RawXdxrInfo> List;        // 原始数据列表
    };

    // 解析后的除权除息信息
    struct XdxrInfo {
        std::string Date;           // 日期 YYYY-MM-DD格式
        int         Category;       // 类型编号
        std::string Name;           // 类型名称
        f64         FenHong;        // 分红(元)
        f64         PeiGuJia;       // 配股价(元)
        f64         SongZhuanGu;    // 送转股(股)
        f64         PeiGu;          // 配股(股)
        f64         SuoGu;          // 缩股(股)
        f64         QianLiuTong;    // 除权前流通股(万股)
        f64         HouLiuTong;     // 除权后流通股(万股)
        f64         QianZongGuBen;  // 除权前总股本(万股)
        f64         HouZongGuBen;   // 除权后总股本(万股)
        f64         FenShu;         // 权证份数
        f64         XingQuanJia;    // 行权价格(元)

        // 是否进行除权除息调整
        bool is_adjust() const {
            f64 count = FenHong;   // 分红
            count += PeiGu;        // 配股
            count += SongZhuanGu;  // 送转股
            count += SuoGu;        // 缩股
            count += FenShu;       // 行权
            return count > 0.00;
        }

        /**
         * @brief 计算调整因子m和a
         *
         * 根据股票分红配股等参数计算价格调整因子，用于复权计算
         *
         * @return std::tuple<f64, f64> 返回调整因子m和a的元组
         *         - m: 价格调整乘数因子
         *         - a: 价格调整加数因子
         *
         * @note 当1+B接近0时，会返回默认值m=1.0和a=0.0
         */
        std::tuple<f64, f64> adjustFactor() const {
            f64 m = 0, a = 0;

            // 计算货币调整项和股本调整比率（通过独立函数）
            f64 A = computeMonetaryAdjustment();
            f64 B = computeShareAdjustmentRatio();

            if (std::abs(1.0 + B) > 1e-10) {
                m = 1.0 / (1.0 + B);
                a = A * m;
            } else {
                m = 1.0;
                a = 0.0;
            }

            return {m, a};
        }

        // 计算货币调整项 (monetary adjustment per 10 shares -> per-share adjust after /10)
        [[nodiscard]] f64 computeMonetaryAdjustment() const {
            // (配股数量 * 配股价 - 分红 + 权证份数 * 行权价格) / 10
            return (PeiGu * PeiGuJia - FenHong + FenShu * XingQuanJia) / 10.0;
        }

        // 计算股本调整比率 (新增股数/送转股/缩股/行权影响) / 10
        [[nodiscard]] f64 computeShareAdjustmentRatio() const {
            // (送转股 + 配股 - 缩股 + 权证份数) / 10
            return (SongZhuanGu + PeiGu - SuoGu + FenShu) / 10.0;
        }

        // 判断是否是股本变化
        // 返回: true表示是股本变化，false表示不是
        [[nodiscard]] bool IsCapitalChange() const {
            switch (Category) {
                case EX_DIVIDEND:                      // 除权除息
                case STOCK_SPLIT_OR_REVERSE_SPLIT:     // 拆股或合股
                case RESTRICTED_SHARES_CONSOLIDATION:  // 非流通股缩股
                case ISSUE_CALL_WARRANTS:              // 送认购权证
                case ISSUE_PUT_WARRANTS:               // 送认沽权证
                    return false;
                default:
                    if (HouLiuTong > 0 && HouZongGuBen > 0) {
                        return true;
                    }
            }
            return false;
        }

        // 生成复权计算函数
        // 返回: 计算复权价格的函数对象
        [[nodiscard]] std::function<f64(f64)> Adjust() const {
            f64 songZhuangu = SongZhuanGu;
            f64 peiGu       = PeiGu;
            f64 suoGu       = SuoGu;
            f64 xdxrGuShu   = (songZhuangu + peiGu - suoGu) / 10;
            f64 fenHong     = FenHong;
            f64 peiGuJia    = PeiGuJia;
            f64 xdxrFenHong = (peiGuJia * peiGu - fenHong) / 10;

            return [xdxrFenHong, xdxrGuShu](f64 p) { return (p + xdxrFenHong) / (1 + xdxrGuShu); };
        }

        friend std::ostream &operator<<(std::ostream &os, const XdxrInfo &info) {
            os << "Date: " << info.Date << " Category: " << info.Category << " Name: " << info.Name
               << " FenHong: " << info.FenHong << " PeiGuJia: " << info.PeiGuJia << " SongZhuanGu: " << info.SongZhuanGu
               << " PeiGu: " << info.PeiGu << " SuoGu: " << info.SuoGu << " QianLiuTong: " << info.QianLiuTong
               << " HouLiuTong: " << info.HouLiuTong << " QianZongGuBen: " << info.QianZongGuBen
               << " HouZongGuBen: " << info.HouZongGuBen << " FenShu: " << info.FenShu
               << " XingQuanJia: " << info.XingQuanJia;
            return os;
        }
    };

    struct XdxrInfoResponse : public ResponseHeader<XdxrInfoResponse> {
        u16                   Count;
        std::vector<XdxrInfo> List;

        void deserializeImpl(const std::vector<u8> &body) {
            BinaryStream bs(body);
            bs.skip(9);
            Count = bs.get_u16();
            List.reserve(Count);
            for (int i = 0; i < Count; i++) {
                XdxrInfo e{};
                bs.get_u8();                          // 市场代码
                std::string code = bs.get_string(6);  // 股票代码
                bs.get_u8();                          // 未知
                u32 date     = bs.get_u32();          // 日期
                u8  category = bs.get_u8();           // 类型
                u8  data[16] = {0};                   // 数据
                bs.get_array(data);
                auto [year, month, day, hour, minute] = helpers::getDatetimeFromUint32(9, date, 0);
                e.Category                            = category;
                e.Date                                = fmt::format("{:04d}-{:02d}-{:02d}", year, month, day);
                e.Name                                = to_string(static_cast<XdxrCategory>(e.Category));
                BinaryStream tmp(data);
                switch (e.Category) {
                    case 1:  // 除权除息
                    {
                        f32 f         = 0;
                        f             = tmp.get_float();
                        e.FenHong     = f;
                        f             = tmp.get_float();
                        e.PeiGuJia    = f;
                        f             = tmp.get_float();
                        e.SongZhuanGu = f;
                        f             = tmp.get_float();
                        e.PeiGu       = f;
                        break;
                    }
                    case 11:
                    case 12: {
                        f32 f = 0;
                        tmp.skip(8);
                        f       = tmp.get_float();
                        e.SuoGu = f;
                        break;
                    }
                    case 13:
                    case 14: {
                        f32 f         = 0;
                        f             = tmp.get_float();
                        e.XingQuanJia = f;
                        tmp.skip(8);
                        f        = tmp.get_float();
                        e.FenShu = f;
                        break;
                    }
                    default: {
                        u32 v           = 0;
                        v               = tmp.get_u32();
                        e.QianLiuTong   = _get_v(v);
                        v               = tmp.get_u32();
                        e.QianZongGuBen = _get_v(v);
                        v               = tmp.get_u32();
                        e.HouLiuTong    = _get_v(v);
                        v               = tmp.get_u32();
                        e.HouZongGuBen  = _get_v(v);
                        break;
                    }
                }
                List.emplace_back(e);
            }
        }

        std::string toStringImpl() const { return fmt::format("Count: {}", Count); }

    private:
        static f64 _get_v(u32 v) {
            if (v == 0) {
                return 0;
            }
            return helpers::integerToFloat64(v);
        }
    };

#pragma pack(pop)  // 恢复默认对齐方式
}  // namespace level1

#endif  // QUANT1X_LEVEL1_XDXR_INFO_H
