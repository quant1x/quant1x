#pragma once
#ifndef QUANT1X_FACTOR_F10_FINANCIAL_REPORT_H
#define QUANT1X_FACTOR_F10_FINANCIAL_REPORT_H 1

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <ostream>

namespace dfcf {

    struct Exception;

//    struct Exception {
//        int code_;
//        std::string msg_;
//        Exception(int code, const std::string& msg) : code_(code), msg_(msg) {}
//        [[nodiscard]] int code() const { return code_; }
//        [[nodiscard]] const std::string& message() const { return msg_; }
//    };

    struct QuarterlyReport {
        std::string SecuCode;
        std::string UpdateDate;      // 更新日期
        std::string ReportDate;      // 报告日期
        std::string NoticeDate;      // 最新公告日期
        std::string IsNew;
        std::string ORGCODE;
        std::string TRADEMARKETZJG;
        std::string QDATE;           // 季报期
        std::string DATATYPE;
        std::string DATAYEAR;
        std::string DATEMMDD;
        std::string EITIME;
        std::string SECURITYCODE;
        std::string SECURITYNAMEABBR;
        std::string TRADEMARKETCODE;
        std::string TRADEMARKET;
        std::string SECURITYTYPECODE;
        std::string SECURITYTYPE;
        double BasicEPS{};            // 每股收益
        double DeductBasicEPS{};      // 每股收益(扣除)
        double BPS{};                 // 每股净资产
        double TotalOperateIncome{};  // 营业总收入
        double ParentNetprofit{};     // 净利润
        double WeightAvgRoe{};        // 净资产收益率
        double YSTZ{};                // 营业总收入同比增长
        double SJLTZ{};               // 净利润同比增长
        double MGJYXJJE{};            // 每股经营现金流量(元)
        double XSMLL{};               // 销售毛利率(%)
        double YSHZ{};
        double SJLHZ{};
        double ASSIGNDSCRPT{};        // 废弃
        double PAYYEAR{};             // 废弃
        std::string PUBLISHNAME;
        double ZXGXL{};
        std::string SecurityCode;

        friend std::ostream &operator<<(std::ostream &os, const QuarterlyReport &report);
    };

    std::tuple<std::vector<QuarterlyReport>, int, Exception> QuarterlyReports(
        const std::string& featureDate,
        int pageNo = 1);

    [[maybe_unused]] std::tuple<std::vector<QuarterlyReport>, int, Exception> QuarterlyReportsBySecurityCode(
        const std::string& securityCode,
        const std::string& date,
        int diffQuarters,
        int pageNo = 1);

    std::optional<QuarterlyReport> GetCacheQuarterlyReportsBySecurityCode(
        const std::string& securityCode,
        const std::string& date,
        int diffQuarters = 1);

    struct QuarterlyReportSummary {
        std::string QDate;
        double BPS = 0.0;
        double BasicEPS = 0.0;
        double TotalOperateIncome = 0.0;
        double DeductBasicEPS = 0.0;

        void Assign(const dfcf::QuarterlyReport& v) {
            BPS = v.BPS;
            BasicEPS = v.BasicEPS;
            TotalOperateIncome = v.TotalOperateIncome;
            DeductBasicEPS = v.DeductBasicEPS;
            QDate = v.QDATE;
        }
    };

    void loadQuarterlyReports(const std::string &date);
    QuarterlyReportSummary getQuarterlyReportSummary(const std::string& securityCode, const std::string& date);

} // namespace dfcf

#endif //QUANT1X_FACTOR_F10_FINANCIAL_REPORT_H
