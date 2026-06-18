#pragma once
#ifndef QUANT1X_CONTRB_DATA_TDX_XDXR_INFO_H
#define QUANT1X_CONTRB_DATA_TDX_XDXR_INFO_H 1

#include <quant1x/data/meta/exchange.h>
#include <quant1x/contrib/data/tdx/protocol.h>
#include <quant1x/data/meta/instrument.h>

#include <ostream>

// ==============================
// 除权除息
// ==============================

namespace quant1x::contrib::data::tdx {

    // XDXR类型映射表
    enum XdxrCategory : int {
        EX_DIVIDEND                       = 1,   // 除权除息
        BONUS_SHARES_LISTING              = 2,   // 送股上市(无偿)
        RESTRICTED_SHARES_LISTING         = 3,   // 非流通股上市(受限股解禁)
        UNSPECIFIED_CAPITAL_ADJUSTMENT    = 4,   // 未知股本变动
        GENERAL_CAPITAL_ADJUSTMENT        = 5,   // 股本变化(保留, 但慎用)
        NEW_SHARE_ISSUANCE                = 6,   // 增发新股
        SHARE_REPURCHASE                  = 7,   // 股份回购
        NEW_SHARES_LISTING                = 8,   // 增发新股上市
        TRANSFERRED_RIGHTS_SHARES_LISTING = 9,   // 转配股上市(中国特有)
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

    // ==========================================================
    // 解析后的除权除息信息 (定义在前, 被 Xdxr 使用)
    // ==========================================================
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
         * 根据股票分红配股等参数计算价格调整因子, 用于复权计算
         *
         * @return std::tuple<f64, f64> 返回调整因子m和a的元组
         *         - m: 价格调整乘数因子
         *         - a: 价格调整加数因子
         *
         * @note 当1+B接近0时, 会返回默认值m=1.0和a=0.0
         */
        std::tuple<f64, f64> adjustFactor() const {
            f64 m = 0, a = 0;

            // 计算货币调整项和股本调整比率(通过独立函数)
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
        // 返回: true表示是股本变化, false表示不是
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

    // 除权除息请求/响应 (对齐 Python Xdxr)
    struct Xdxr : public BaseMessage<Xdxr> {
        u8              Market;   // 市场代码 0:深圳 1:上海
        char            Code[6];  // 股票代码
        std::vector<u8> padding;

        u16                   Count;       // 响应: 数据条数
        std::vector<XdxrInfo> List;        // 响应: 解析后的除权除息列表

        Xdxr(const meta::Instrument &inst) : BaseMessage<Xdxr>() {
            request_header.frame_type  = ZlibFlag::Uncompressed;
            request_header.seq_id      = get_sequence_id();
            request_header.packet_ctrl = 0x01;
            request_header.cmd_id      = StdCommand::XDXR_INFO;
            
            Market                = static_cast<u8>(helpers::exchange_to_market(inst.exchange));
            const char *const tmp = inst.marker_ticker().c_str();
            std::memcpy(Code, tmp, sizeof(Code));
            padding = strings::hexToBytes("0100");
        }

        // 序列化方法
        std::vector<u8> serialize_request_body_impl() {
            BinaryStream stream;
            stream.push_byte_array(padding.data(), padding.size());
            stream.push_arithmetic(Market);
            stream.push_array(Code);
            return stream.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &body) {
            BinaryStream bs(body);
            bs.skip(9);
            Count = bs.get_u16();
            List.reserve(Count);
            for (int i = 0; i < Count; i++) {
                XdxrInfo info{};
                bs.get_u8();                          // 市场代码
                std::string code = bs.get_string(6);  // 股票代码
                bs.get_u8();                          // 未知
                u32 date     = bs.get_u32();          // 日期
                u8  category = bs.get_u8();           // 类型
                u8  data[16] = {0};                   // 数据
                bs.get_array(data);
                auto [year, month, day, hour, minute] = helpers::getDatetimeFromUint32(9, date, 0);
                info.Category                            = category;
                info.Date                                = fmt::format("{:04d}-{:02d}-{:02d}", year, month, day);
                info.Name                                = quant1x::contrib::data::tdx::to_string(static_cast<XdxrCategory>(info.Category));
                BinaryStream tmp(data);
                switch (info.Category) {
                    case 1:  // 除权除息
                    {
                        f32 f         = 0;
                        f             = tmp.get_float();
                        info.FenHong     = f;
                        f             = tmp.get_float();
                        info.PeiGuJia    = f;
                        f             = tmp.get_float();
                        info.SongZhuanGu = f;
                        f             = tmp.get_float();
                        info.PeiGu       = f;
                        break;
                    }
                    case 11:
                    case 12: {
                        f32 f = 0;
                        tmp.skip(8);
                        f       = tmp.get_float();
                        info.SuoGu = f;
                        break;
                    }
                    case 13:
                    case 14: {
                        f32 f         = 0;
                        f             = tmp.get_float();
                        info.XingQuanJia = f;
                        tmp.skip(8);
                        f        = tmp.get_float();
                        info.FenShu = f;
                        break;
                    }
                    default: {
                        u32 v           = 0;
                        v               = tmp.get_u32();
                        info.QianLiuTong   = _get_v(v);
                        v               = tmp.get_u32();
                        info.QianZongGuBen = _get_v(v);
                        v               = tmp.get_u32();
                        info.HouLiuTong    = _get_v(v);
                        v               = tmp.get_u32();
                        info.HouZongGuBen  = _get_v(v);
                        break;
                    }
                }
                List.emplace_back(info);
            }
        }

        std::string to_string_impl() const {
            std::ostringstream out;
            out << request_header.header_string_impl();
            out << '{';
            out << "Market:" << (int)Market;
            out << ", Code:" << std::string(Code, sizeof(Code));
            out << ", padding:" << strings::bytesToHex(padding);
            out << '}';
            out << " {Count:" << Count << "}";
            return out.str();
        }

    private:
        static f64 _get_v(u32 v) {
            if (v == 0) {
                return 0;
            }
            return helpers::integerToFloat64(v);
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

    // 除权除息批量请求 (对齐 Python XdxrBatch, Python有, C++原无)
    struct XdxrBatch : public BaseMessage<XdxrBatch> {
        struct StockEntry {
            u8 market;
            char code[6];
        };
        std::vector<StockEntry> stocks;

        u16 Count;
        struct BatchEntry {
            u8 market;
            char code[6];
            u16 xdxr_count;
            std::vector<XdxrInfo> list;
        };
        std::vector<BatchEntry> entries;

        XdxrBatch(const std::vector<meta::Instrument> &insts) : BaseMessage<XdxrBatch>() {
            request_header.frame_type = ZlibFlag::Uncompressed;
            request_header.seq_id = get_sequence_id();
            request_header.packet_ctrl = 0x01;
            request_header.cmd_id = StdCommand::XDXR_INFO;
            for (auto const &inst : insts) {
                StockEntry entry{};
                entry.market = helpers::exchange_to_market(inst.exchange);
                const char *tmp = inst.marker_ticker().c_str();
                std::memcpy(entry.code, tmp, sizeof(entry.code));
                stocks.push_back(entry);
            }
        }

        std::vector<u8> serialize_request_body_impl() {
            u16 cnt = u16(stocks.size());
            BinaryStream stream;
            stream.push_arithmetic(cnt);
            for (auto const &s : stocks) {
                stream.push_arithmetic(s.market);
                stream.push_array(s.code);
            }
            return stream.data();
        }

        void deserialize_response_body_impl(const std::vector<u8> &data) {
            entries.clear();
            if (data.size() < 2) return;
            BinaryStream bs(data);
            Count = bs.get_u16();
            for (int i = 0; i < Count; i++) {
                BatchEntry be{};
                be.market = bs.get_u8();
                bs.get_array(be.code);
                be.xdxr_count = bs.get_u16();
                for (int j = 0; j < be.xdxr_count; j++) {
                    bs.get_u8();  // market
                    bs.get_string(6);  // code
                    bs.get_u8();  // unknown
                    u32 date = bs.get_u32();
                    u8 cat = bs.get_u8();
                    u8 d[16] = {0};
                    bs.get_array(d);
                    auto [year, month, day, h, m] = helpers::getDatetimeFromUint32(9, date, 0);
                    XdxrInfo info{};
                    info.Category = cat;
                    info.Date = fmt::format("{:04d}-{:02d}-{:02d}", year, month, day);
                    info.Name = quant1x::contrib::data::tdx::to_string(static_cast<XdxrCategory>(info.Category));
                    BinaryStream tmp(d);
                    switch (info.Category) {
                        case 1:
                            info.FenHong = tmp.get_float();
                            info.PeiGuJia = tmp.get_float();
                            info.SongZhuanGu = tmp.get_float();
                            info.PeiGu = tmp.get_float();
                            break;
                        case 11: case 12:
                            tmp.skip(8);
                            info.SuoGu = tmp.get_float();
                            break;
                        case 13: case 14:
                            info.XingQuanJia = tmp.get_float();
                            tmp.skip(8);
                            info.FenShu = tmp.get_float();
                            break;
                        default:
                            info.QianLiuTong = _get_v(tmp.get_u32());
                            info.QianZongGuBen = _get_v(tmp.get_u32());
                            info.HouLiuTong = _get_v(tmp.get_u32());
                            info.HouZongGuBen = _get_v(tmp.get_u32());
                            break;
                    }
                    be.list.push_back(info);
                }
                entries.push_back(be);
            }
        }

        std::string to_string_impl() const {
            return fmt::format("XdxrBatch{{Count:{}}}", Count);
        }

    private:
        static f64 _get_v(u32 v) {
            if (v == 0) return 0;
            return helpers::integerToFloat64(v);
        }
    };

}  // namespace quant1x::contrib::data::tdx

#endif  // QUANT1X_CONTRB_DATA_TDX_XDXR_INFO_H
