#pragma once
#ifndef QUANT1X_FACTOR_F10_NOTICE_H
#define QUANT1X_FACTOR_F10_NOTICE_H 1

#include <quant1x/base/api.h>

namespace dfcf {

    const int errorBaseNotice = 91000;

    // 错误定义
    class Exception {
    public:
        Exception(int code, const std::string &message) : code_(code), message_(message) {}

        int                code() const { return code_; }
        const std::string &message() const { return message_; }

    private:
        int         code_;
        std::string message_;
    };

    inline Exception ErrNoticeBadApi(errorBaseNotice, "接口异常");
    inline Exception ErrNoticeNotFound(errorBaseNotice + 1, "没有数据");

    // 公告详情
    struct NoticeDetail {
        std::string Code;          // 证券代码
        std::string Name;          // 证券名称
        std::string DisplayTime;   // 显示时间
        std::string NoticeDate;    // 公告时间
        std::string Title;         // 公告标题
        std::string Keywords;      // 公告关键词
        int         Increase;      // 增持
        int         Reduce;        // 减持
        int         HolderChange;  // 实际控制人变更
        int         Risk;          // 风险数
    };

    // 公告类型枚举
    enum class EMNoticeType {
        NoticeAll,          // 全部
        NoticeUnused1,      // 财务报告
        NoticeUnused2,      // 融资公告
        NoticeUnused3,      // 风险提示
        NoticeUnused4,      // 信息变更
        NoticeWarning,      // 重大事项
        NoticeUnused6,      // 资产重组
        NoticeHolderChange  // 持股变动
    };

    // 获取公告类型名称
    api_inline std::string GetNoticeType(EMNoticeType noticeType) {
        switch (noticeType) {
            case EMNoticeType::NoticeAll:
                return "全部";
            case EMNoticeType::NoticeUnused1:
                return "财务报告";
            case EMNoticeType::NoticeUnused2:
                return "融资公告";
            case EMNoticeType::NoticeUnused3:
                return "风险提示";
            case EMNoticeType::NoticeUnused4:
                return "信息变更";
            case EMNoticeType::NoticeWarning:
                return "重大事项";
            case EMNoticeType::NoticeUnused6:
                return "资产重组";
            case EMNoticeType::NoticeHolderChange:
                return "持股变动";
            default:
                return "其它";
        }
    }

    // 个股公告
    std::tuple<std::vector<NoticeDetail>, int, Exception> StockNotices(const std::string &securityCode,
                                                                       const std::string &beginDate,
                                                                       const std::string &endDate    = "",
                                                                       int                pageNumber = 1);

    // 年报季报披露日期
    std::tuple<std::string, std::string> NoticeDateForReport(const std::string &code, const std::string &date);

    struct CompanyNotice {
        int         Increase;      // 增持
        int         Reduce;        // 减持
        int         Risk;          // 风险数
        std::string RiskKeywords;  // 风险关键词
    };

    // 获取单个股票的公告信息
    CompanyNotice getOneNotice(const std::string &securityCode, const std::string &currentDate);

}  // namespace dfcf

#endif  // QUANT1X_FACTOR_F10_NOTICE_H
