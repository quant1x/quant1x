#include <cpr/cpr.h>
#include <quant1x/instruments/markets.h>
#include <quant1x/factors/safety_score.h>

//#include <iostream>
#include <map>
//#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>

using json = nlohmann::json;

const std::string urlRiskAssessment            = "http://page3.tdx.com.cn:7615/site/pcwebcall_static/bxb/json/";
const int         defaultSafetyScore           = 100;
const int         defaultSafetyScoreOfNotFound = 100;
const int         defaultSafetyScoreOfIgnore   = 0;

namespace risks {

    // ========================
    // 工具函数：安全获取字段（保留默认值）
    // ========================

    template <typename T>
    void safe_get(const json &j, const std::string &key, T &value) {
        if (j.contains(key)) {
            try {
                value = j.at(key).get<T>();
            } catch (...) {
            }
        }
    }

    // 特化：int 类型
    template <>
    void safe_get<int>(const json &j, const std::string &key, int &value) {
        if (j.contains(key) && j[key].is_number_integer()) {
            try {
                value = j.at(key).get<int>();
            } catch (...) {
            }
        }
    }

    // 特化：std::string 类型
    template <>
    void safe_get<std::string>(const json &j, const std::string &key, std::string &value) {
        if (j.contains(key) && j[key].is_string()) {
            try {
                value = j.at(key).get<std::string>();
            } catch (...) {
            }
        }
    }

    struct CommonLxId {
        int         fs    = 0;  // 默认值：0
        int         level = 0;  // 默认值：0
        int         trig  = 0;  // 默认值：0
        int         pos   = 0;  // 默认值：0
        int         id    = 0;  // 默认值：0
        std::string lx;         // 默认值：空字符串
        std::string trigyy;     // 默认值：空字符串

        friend void from_json(const json &j, CommonLxId &item) {
            safe_get(j, "fs", item.fs);
            safe_get(j, "level", item.level);
            safe_get(j, "trig", item.trig);
            safe_get(j, "pos", item.pos);
            safe_get(j, "id", item.id);
            safe_get(j, "lx", item.lx);
            safe_get(j, "trigyy", item.trigyy);
        }
    };

    struct SafetyItem {
        int         fs = 0;    // 分数
        std::string trigyy;    // 触发原因
        int         trig = 0;  // 是否触发风险, 0-否, 1-是
        int         id   = 0;  // 风险id
        std::string lx;        // 风险项名称

        std::vector<CommonLxId> details;

        friend void from_json(const json &j, SafetyItem &item) {
            safe_get(j, "fs", item.fs);
            safe_get(j, "trig", item.trig);
            safe_get(j, "id", item.id);
            safe_get(j, "lx", item.lx);
            safe_get(j, "trigyy", item.trigyy);

            if (j.contains("commonlxid") && j["commonlxid"].is_array()) {
                for (const auto &elem : j["commonlxid"]) {
                    CommonLxId clid;
                    elem.get_to(clid);
                    item.details.push_back(clid);
                }
            }
        }
    };

    struct RiskCategory {
        std::string             name;
        std::vector<SafetyItem> rows;

        friend void from_json(const json &j, RiskCategory &category) {
            safe_get(j, "name", category.name);

            if (j.contains("rows") && j["rows"].is_array()) {
                for (const auto &elem : j["rows"]) {
                    SafetyItem item;
                    elem.get_to(item);
                    category.rows.push_back(item);
                }
            }
        }
    };

    struct SafetyReport {
        int                       total = 0;  // 检查项总数
        std::string               name;       // 证券名称
        int                       num = 0;    // 风险项数
        std::vector<RiskCategory> data;       // 检查项分类数据

        friend void from_json(const json &j, SafetyReport &item) {
            safe_get(j, "total", item.total);
            safe_get(j, "name", item.name);
            safe_get(j, "num", item.num);

            if (j.contains("data") && j["data"].is_array()) {
                for (const auto &elem : j["data"]) {
                    RiskCategory category;
                    elem.get_to(category);
                    item.data.push_back(category);
                }
            }
        }
    };

    static inline std::map<std::string, int> mapSafetyScore;
    static std::mutex                        mapMutex;  // 用于线程安全访问mapSafetyScore

    // 获取个股安全分
    std::tuple<int, std::string> GetSafetyScore(const std::string &securityCode) {
        if (!exchange::AssertStockBySecurityCode(securityCode)) {
            return {defaultSafetyScore, ""};
        }

        if (instruments::IsNeedIgnore(securityCode)) {
            return {defaultSafetyScoreOfIgnore, ""};
        }

        int         score = defaultSafetyScore;
        std::string detail;
        auto [marketId, marketCode, pureCode] = exchange::DetectMarket(securityCode);

        if (pureCode.length() == 6) {
            std::string url = urlRiskAssessment + pureCode + ".json";

            // 使用cpr发送HTTP GET请求
            cpr::Response response = cpr::Get(cpr::Url{url});

            if (response.status_code != 200) {
                score = defaultSafetyScoreOfNotFound;
            } else {
                try {
                    json obj = json::parse(response.text);
                    try {
                        SafetyReport report;
                        obj.get_to(report);
                        int                           tmpScore = 100;
                        std::vector<std::string_view> risk_categories;
                        for (auto const &data : report.data) {
                            const std::string category = data.name;
                            for (auto const &v : data.rows) {
                                std::vector<std::string_view> details;
                                if (v.trig == 1) {
                                    tmpScore -= v.fs;
                                    for (auto const &common : v.details) {
                                        if (common.trig == 1) {
                                            details.emplace_back(common.trigyy);
                                        }
                                    }
                                }
                                if (!details.empty()) {
                                    std::string riskItem = category;
                                    riskItem.append(":");
                                    riskItem.append(v.lx);
                                    riskItem.append("(");
                                    riskItem.append(std::to_string(details.size()));
                                    riskItem.append("):");
                                    riskItem.append(strings::join(details, "|||"));
                                    risk_categories.emplace_back(riskItem);
                                }
                            }
                        }
                        score = tmpScore;
                        if (!risk_categories.empty()) {
                            detail.append("[");
                            detail.append(strings::join(risk_categories, ';'));
                            detail.append("]");
                        }
                        // 线程安全地更新map
                        std::lock_guard<std::mutex> lock(mapMutex);
                        mapSafetyScore[securityCode] = score;
                    } catch (const json::type_error &e) {
                        spdlog::error("[safety-score] 类型转换错误: {}", e.what());
                    }
                } catch (const json::parse_error &e) {
                    spdlog::error("[safety-score] JSON parse error: {}", e.what());
                    // 线程安全地读取map
                    std::lock_guard<std::mutex> lock(mapMutex);
                    auto                        it = mapSafetyScore.find(securityCode);
                    if (it != mapSafetyScore.end()) {
                        score = it->second;
                    } else {
                        score = defaultSafetyScore;
                    }
                }
            }
        }

        return {score, detail};
    }
}  // namespace risks