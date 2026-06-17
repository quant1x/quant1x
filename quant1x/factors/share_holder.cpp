#include <cpr/cpr.h>
#include <quant1x/encoding/csv.h>
#include <quant1x/encoding/json.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/factors/share_holder.h>
#include <quant1x/std/time.h>

#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>

using json   = nlohmann::json;
namespace fs = std::filesystem;

namespace dfcf {

    // Constants
    const std::string urlTop10ShareHolder = "https://datacenter-web.eastmoney.com/api/data/v1/get";

    // Raw stock holder data structure
    struct RawStockHolder {
        std::string version;
        struct Result {
            int pages;
            struct Data {
                std::string SECUCODE;
                std::string SECURITY_CODE;
                std::string ORG_CODE;
                std::string END_DATE;
                std::string HOLDER_NAME;
                int64_t     HOLD_NUM;
                double      FREE_HOLDNUM_RATIO;
                std::string HOLD_NUM_CHANGE;
                double      CHANGE_RATIO;
                std::string IS_HOLDORG;
                int         HOLDER_RANK;
                std::string SECURITY_NAME_ABBR;
                std::string HOLDER_CODE;
                std::string SECURITY_TYPE_CODE;
                std::string HOLDER_STATE;
                double      HOLDER_MARKET_CAP;
                double      HOLD_RATIO;
                std::string HOLD_CHANGE;
                double      HOLD_RATIO_CHANGE;
                std::string HOLDER_TYPE;
                std::string SHARES_TYPE;
                std::string UPDATE_DATE;
                std::string REPORT_DATE_NAME;
                std::string HOLDER_NEW;
                std::string FREE_RATIO_QOQ;
                std::string HOLDER_STATEE;
                std::string IS_REPORT;
                std::string HOLDER_CODE_OLD;
                std::string HOLDER_NEWTYPE;
                std::string HOLDNUM_CHANGE_NAME;
                std::string IS_MAX_REPORTDATE;
                std::string COOPERATION_HOLDER_MARK;
                std::string MXID;
                std::string LISTING_STATE;
                int64_t     XZCHANGE;
                std::string NEW_CHANGE_RATIO;
            };
            std::vector<Data> data;
            int               count;
        } result;
        bool        success{};
        std::string message;
        int         code{};
    };

    // JSON 反序列化
    void from_json(const json &j, RawStockHolder::Result::Data &d) {
        // 字符串类型(自动处理null)
        encoding::unsafe_json::get_string(j, "SECUCODE", d.SECUCODE);
        encoding::unsafe_json::get_string(j, "SECURITY_CODE", d.SECURITY_CODE);
        encoding::unsafe_json::get_string(j, "ORG_CODE", d.ORG_CODE);
        encoding::unsafe_json::get_string(j, "END_DATE", d.END_DATE);
        encoding::unsafe_json::get_string(j, "HOLDER_NAME", d.HOLDER_NAME);
        encoding::unsafe_json::get_string(j, "HOLD_NUM_CHANGE", d.HOLD_NUM_CHANGE);
        encoding::unsafe_json::get_string(j, "SECURITY_NAME_ABBR", d.SECURITY_NAME_ABBR);
        encoding::unsafe_json::get_string(j, "HOLDER_CODE", d.HOLDER_CODE);
        encoding::unsafe_json::get_string(j, "SECURITY_TYPE_CODE", d.SECURITY_TYPE_CODE);
        encoding::unsafe_json::get_string(j, "HOLDER_STATE", d.HOLDER_STATE);
        encoding::unsafe_json::get_string(j, "HOLD_CHANGE", d.HOLD_CHANGE);
        encoding::unsafe_json::get_string(j, "HOLDER_TYPE", d.HOLDER_TYPE);
        encoding::unsafe_json::get_string(j, "SHARES_TYPE", d.SHARES_TYPE);
        encoding::unsafe_json::get_string(j, "UPDATE_DATE", d.UPDATE_DATE);
        encoding::unsafe_json::get_string(j, "REPORT_DATE_NAME", d.REPORT_DATE_NAME);
        encoding::unsafe_json::get_string(j, "HOLDER_NEW", d.HOLDER_NEW);
        encoding::unsafe_json::get_string(j, "FREE_RATIO_QOQ", d.FREE_RATIO_QOQ);
        encoding::unsafe_json::get_string(j, "HOLDER_STATEE", d.HOLDER_STATEE);
        encoding::unsafe_json::get_string(j, "IS_REPORT", d.IS_REPORT);
        encoding::unsafe_json::get_string(j, "HOLDER_CODE_OLD", d.HOLDER_CODE_OLD);
        encoding::unsafe_json::get_string(j, "HOLDER_NEWTYPE", d.HOLDER_NEWTYPE);
        encoding::unsafe_json::get_string(j, "HOLDNUM_CHANGE_NAME", d.HOLDNUM_CHANGE_NAME);
        encoding::unsafe_json::get_string(j, "IS_MAX_REPORTDATE", d.IS_MAX_REPORTDATE);
        encoding::unsafe_json::get_string(j, "COOPERATION_HOLDER_MARK", d.COOPERATION_HOLDER_MARK);
        encoding::unsafe_json::get_string(j, "MXID", d.MXID);
        encoding::unsafe_json::get_string(j, "LISTING_STATE", d.LISTING_STATE);
        encoding::unsafe_json::get_string(j, "NEW_CHANGE_RATIO", d.NEW_CHANGE_RATIO);

        // 数值类型(带默认值和类型检查)
        encoding::unsafe_json::get_number(j, "HOLD_NUM", d.HOLD_NUM, int64_t(0));
        encoding::unsafe_json::get_number(j, "FREE_HOLDNUM_RATIO", d.FREE_HOLDNUM_RATIO, 0.0);
        encoding::unsafe_json::get_number(j, "CHANGE_RATIO", d.CHANGE_RATIO, 0.0);
        encoding::unsafe_json::get_number(j, "HOLDER_RANK", d.HOLDER_RANK, 0);
        encoding::unsafe_json::get_number(j, "HOLDER_MARKET_CAP", d.HOLDER_MARKET_CAP, 0.0);
        encoding::unsafe_json::get_number(j, "HOLD_RATIO", d.HOLD_RATIO, 0.0);
        encoding::unsafe_json::get_number(j, "HOLD_RATIO_CHANGE", d.HOLD_RATIO_CHANGE, 0.0);
        encoding::unsafe_json::get_number(j, "XZCHANGE", d.XZCHANGE, int64_t(0));

        // 布尔类型(特殊处理字符串/数字/布尔混合情况)
        encoding::unsafe_json::get_string(j, "IS_HOLDORG", d.IS_HOLDORG, "");
    }

    void from_json(const json &j, RawStockHolder::Result &r) {
        encoding::unsafe_json::get_number(j, "pages", r.pages, 0);
        encoding::unsafe_json::get_number(j, "count", r.count, 0);

        if (j.contains("data")) {
            if (j["data"].is_array()) {
                r.data = j["data"].get<std::vector<RawStockHolder::Result::Data>>();
            } else if (!j["data"].is_null()) {
                throw json::type_error::create(302, "[share-holder] Field 'data' must be array", &j);
            }
        }
    }

    void from_json(const json &j, RawStockHolder &r) {
        encoding::unsafe_json::get_string(j, "version", r.version);
        encoding::unsafe_json::get_string(j, "message", r.message);
        encoding::unsafe_json::get_bool(j, "success", r.success, false);
        encoding::unsafe_json::get_number(j, "code", r.code, 0);

        if (j.contains("result")) {
            if (!j["result"].is_null()) {
                r.result = j["result"].get<RawStockHolder::Result>();
            }
        }
    }

    // 前十大流通股东 https://data.eastmoney.com/gdfx/stock/600115.html
    std::vector<CirculatingShareholder>
    ShareHolder(const std::string &securityCode, const std::string &date, int diff = 0) {
        std::vector<CirculatingShareholder> list;

        auto [x1, x2, code]        = data::detect_symbol(securityCode);
        std::string quarterEndDate = meta::Timestamp(date).only_date();

        auto [y1, y2, qEnd] = api::GetQuarterByDate(date, diff);
        quarterEndDate      = meta::Timestamp(qEnd).only_date();

        cpr::Parameters params{{"sortColumns", "HOLDER_RANK"},
                               {"sortTypes", "1"},
                               {"pageSize", "10"},
                               {"pageNumber", "1"},
                               {"reportName", "RPT_F10_EH_FREEHOLDERS"},
                               {"columns", "ALL"},
                               {"source", "WEB"},
                               {"client", "WEB"},
                               {"filter", "(SECURITY_CODE=\"" + code + "\")(END_DATE='" + quarterEndDate + "')"}};

        std::string url      = urlTop10ShareHolder + "?" + params.GetContent(cpr::CurlHolder());
        auto        response = cpr::Get(cpr::Url{url});

        if (response.status_code != 200) {
            return list;
        }

        try {
            json           rawJson = json::parse(response.text);
            RawStockHolder raw     = rawJson.get<RawStockHolder>();

            if (!raw.success || raw.result.count == 0 || raw.result.data.empty()) {
                return list;
            }

            for (const auto &v : raw.result.data) {
                CirculatingShareholder shareholder{
                    v.SECUCODE,                                      // SecurityCode
                    v.SECURITY_NAME_ABBR,                            // SecurityName
                    meta::Timestamp(v.END_DATE).only_date(),     // EndDate
                    meta::Timestamp(v.UPDATE_DATE).only_date(),  // UpdateDate
                    v.HOLDER_NEWTYPE,                                // HolderType
                    v.HOLDER_NAME,                                   // HolderName
                    v.IS_HOLDORG,                                    // IsHoldOrg
                    v.HOLDER_RANK,                                   // HolderRank
                    v.HOLD_NUM,                                      // HoldNum
                    v.FREE_HOLDNUM_RATIO,                            // FreeHoldNumRatio
                    v.XZCHANGE,                                      // HoldNumChange
                    v.HOLDNUM_CHANGE_NAME,                           // HoldChangeName
                    0,                                               // HoldChangeState (set below)
                    v.CHANGE_RATIO,                                  // HoldChangeRatio
                    v.HOLD_RATIO,                                    // HoldRatio
                    v.HOLD_RATIO_CHANGE                              // HoldRatioChange
                };

                // 修订证券代码
                auto [_, mflag, mcode]   = data::detect_symbol(shareholder.SecurityCode);
                shareholder.SecurityCode = mflag + mcode;

                // HoldChangeState
                if (v.HOLDNUM_CHANGE_NAME == "新进") {
                    shareholder.HoldChangeState = HoldNumNewlyAdded;
                } else if (v.HOLDNUM_CHANGE_NAME == "增加") {
                    shareholder.HoldChangeState = HoldNumIncrease;
                } else if (v.HOLDNUM_CHANGE_NAME == "减少") {
                    shareholder.HoldChangeState = HoldNumDampened;
                } else if (v.HOLDNUM_CHANGE_NAME == "不变") {
                    shareholder.HoldChangeState = HoldNumUnChanged;
                } else {
                    shareholder.HoldChangeState = HoldNumUnknownChanges;
                    std::string warning         = v.SECURITY_NAME_ABBR + ": " + v.SECUCODE +
                                          ", 变化状态未知: " + v.HOLDNUM_CHANGE_NAME;
                    spdlog::warn("[share-holder] WARNING: {}", warning);
                }

                list.push_back(shareholder);
            }

            // Sort by HolderRank
            std::sort(list.begin(), list.end(), [](const CirculatingShareholder &a, const CirculatingShareholder &b) {
                return a.HolderRank < b.HolderRank;
            });
        } catch (const std::exception &e) {
            spdlog::error("[share-holder] Error parsing shareholder data: {}", e.what());
        }

        return list;
    }

    // cacheShareHolder 获取流动股东数据
    std::vector<CirculatingShareholder>
    cacheShareHolder(const std::string &securityCode, const std::string &date, int diff = 1) {
        std::vector<CirculatingShareholder> list;

        auto [x1, x2, last]  = api::GetQuarterByDate(date, diff);
        std::string filename = config::top10_holders_filename(securityCode, last);
        if (fs::exists(filename)) {
            list = encoding::csv::csv_to_slices<CirculatingShareholder>(filename);
            if (!list.empty()) {
                return list;
            }
        }

        auto tmpList = ShareHolder(securityCode, last);
        if (!tmpList.empty()) {
            list = tmpList;
            encoding::csv::slices_to_csv(list, filename);
        }

        return list;
    }

    // GetCacheShareHolder 获取流动股东数据
    std::vector<CirculatingShareholder>
    GetCacheShareHolder(const std::string &securityCode, const std::string &date, int diff) {
        std::vector<CirculatingShareholder> list;

        for (; diff < 4; diff++) {
            auto tmpList = cacheShareHolder(securityCode, date, diff);
            if (tmpList.empty()) {
                continue;
            }
            list = tmpList;
            break;
        }

        return list;
    }

}  // namespace dfcf