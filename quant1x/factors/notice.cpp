#include <cpr/cpr.h>
#include <quant1x/encoding/json.h>
#include <quant1x/market/instruments.h>
#include <quant1x/factors/notice.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

namespace dfcf {
    namespace {
        using json = nlohmann::json;

        // 常量定义
        // const std::string CacheL5KeyNotices = "cache/notices";
        const std::string urlEastmoneyNotices      = "https://np-anotice-stock.eastmoney.com/api/security/ann";
        const std::string urlEastmoneyWarning      = "https://datacenter.eastmoney.com/securities/api/data/get";
        constexpr int     EastmoneyNoticesPageSize = 100;

        // 风险检测的关键词
        const std::vector<std::string> riskKeywords = {"立案",
                                                       "处罚",
                                                       "冻结",
                                                       "诉讼",
                                                       "质押",
                                                       "仲裁",
                                                       "持股5%以上股东权益变动",
                                                       "信用减值",
                                                       "商誉减值",
                                                       "重大风险",
                                                       "退市风险"};

        // 公告原始的消息结构
        struct RawNoticePackage {
            struct Data {
                struct ListItem {
                    std::string ArtCode;
                    struct CodeInfo {
                        std::string AnnType;
                        std::string InnerCode;
                        std::string MarketCode;
                        std::string ShortName;
                        std::string StockCode;
                    };
                    std::vector<CodeInfo> Codes;
                    struct ColumnInfo {
                        std::string ColumnCode;
                        std::string ColumnName;
                    };
                    std::vector<ColumnInfo> Columns;
                    std::string             DisplayTime;
                    std::string             EiTime;
                    std::string             Language;
                    std::string             NoticeDate;
                    std::string             ProductCode;
                    std::string             SortDate;
                    std::string             SourceType;
                    std::string             Title;
                    std::string             TitleCh;
                    std::string             TitleEn;
                };
                std::vector<ListItem> List;
                int                   PageIndex{};
                int                   PageSize{};
                int                   TotalHits{};
            } Data;
            std::string Error;
            int         Success{};
        };

        // 大事提醒结构
        struct WarningDetail {
            std::string              EventType;          // 事件类型
            std::string              SpecificEventType;  // 事件类型
            std::string              NoticeDate;         // 公告日期
            std::string              Level1Content;      // 1级内容
            std::vector<std::string> Level2Content;      // 2级内容
            std::string              InfoCode;           // 信息代码
        };

        struct RawWarning {
            int                                     Code{};     // 状态码
            bool                                    Success{};  // 接口是否调用成功
            std::string                             Message;    // 状态信息
            std::vector<std::vector<WarningDetail>> Data;
            int                                     HasNext{};  // 是否有下一页
        };

        /////////////////////////////////////////////////////////////////////////////
        // 辅助函数声明
        /////////////////////////////////////////////////////////////////////////////
        int         GetPages(int pageSize, int totalHits);
        std::string EncodeParams(const std::map<std::string, std::string> &params);
    }  // anonymous namespace

    /////////////////////////////////////////////////////////////////////////////
    // AllNotices - 安全版本
    /////////////////////////////////////////////////////////////////////////////
    [[maybe_unused]] std::tuple<std::vector<NoticeDetail>, int, Exception>
    AllNotices(EMNoticeType noticeType, const std::string &date, int pageNumber) {
        std::string beginDate = exchange::timestamp(date).only_date();
        std::string endDate   = exchange::timestamp::now().only_date();
        int         pageSize  = EastmoneyNoticesPageSize;

        std::map<std::string, std::string> params = {
            {"sr", "-1"},
            {"page_size", std::to_string(pageSize)},
            {"page_index", std::to_string(pageNumber)},
            {"ann_type", "SHA,CYB,SZA,BJA"},
            {"client_source", "web"},
            {"f_node", std::to_string(static_cast<int>(noticeType))},
            {"s_node", "0"},
            {"begin_time", beginDate},
            {"end_time", endDate},
        };

        std::string url = urlEastmoneyNotices + "?" + EncodeParams(params);
        cpr::Header headers;
        auto        response = cpr::Get(cpr::Url{url}, headers);

        std::vector<NoticeDetail> notices;
        int                       pages = 0;
        Exception                 err(0, "");

        if (response.status_code != 200) {
            spdlog::error("请求失败: {}", response.status_code);
            err = ErrNoticeBadApi;
            return {notices, pages, err};
        }

        try {
            auto             raw = json::parse(response.text);
            RawNoticePackage noticePackage;
            noticePackage.Success        = encoding::safe_json::get_number<int>(raw, "success", -1);
            noticePackage.Error          = encoding::safe_json::get_string(raw, "error", std::string());
            noticePackage.Data.PageIndex = encoding::safe_json::nested_get<int>(raw, {"data", "page_index"}, 1);
            noticePackage.Data.PageSize  = encoding::safe_json::nested_get<int>(
                raw, {"data", "page_size"}, EastmoneyNoticesPageSize);
            noticePackage.Data.TotalHits = encoding::safe_json::nested_get<int>(raw, {"data", "total_hits"}, 0);

            if (raw.contains("data") && raw["data"].is_object()) {
                const auto &dataObj = raw["data"];
                if (dataObj.contains("list") && dataObj["list"].is_array()) {
                    for (const auto &item : dataObj["list"]) {
                        RawNoticePackage::Data::ListItem noticeItem;
                        noticeItem.ArtCode = encoding::safe_json::get_string(item, "art_code", std::string());

                        if (item.contains("codes") && item["codes"].is_array()) {
                            for (const auto &code : item["codes"]) {
                                RawNoticePackage::Data::ListItem::CodeInfo codeInfo;
                                codeInfo.AnnType   = encoding::safe_json::get_string(code, "ann_type", std::string());
                                codeInfo.InnerCode = encoding::safe_json::get_string(code, "inner_code", std::string());
                                codeInfo.MarketCode = encoding::safe_json::get_string(
                                    code, "market_code", std::string());
                                codeInfo.ShortName = encoding::safe_json::get_string(code, "short_name", std::string());
                                codeInfo.StockCode = encoding::safe_json::get_string(code, "stock_code", std::string());
                                noticeItem.Codes.push_back(codeInfo);
                            }
                        }

                        if (item.contains("columns") && item["columns"].is_array()) {
                            for (const auto &column : item["columns"]) {
                                RawNoticePackage::Data::ListItem::ColumnInfo columnInfo;
                                columnInfo.ColumnCode = encoding::safe_json::get_string(
                                    column, "column_code", std::string());
                                columnInfo.ColumnName = encoding::safe_json::get_string(
                                    column, "column_name", std::string());
                                noticeItem.Columns.push_back(columnInfo);
                            }
                        }

                        noticeItem.DisplayTime = encoding::safe_json::get_string(item, "display_time", std::string());
                        noticeItem.EiTime      = encoding::safe_json::get_string(item, "eiTime", std::string());
                        noticeItem.Language    = encoding::safe_json::get_string(item, "language", std::string());
                        noticeItem.NoticeDate  = encoding::safe_json::get_string(item, "notice_date", std::string());
                        noticeItem.ProductCode = encoding::safe_json::get_string(item, "product_code", std::string());
                        noticeItem.SortDate    = encoding::safe_json::get_string(item, "sort_date", std::string());
                        noticeItem.SourceType  = encoding::safe_json::get_string(item, "source_type", std::string());
                        noticeItem.Title       = encoding::safe_json::get_string(item, "title", std::string());
                        noticeItem.TitleCh     = encoding::safe_json::get_string(item, "title_ch", std::string());
                        noticeItem.TitleEn     = encoding::safe_json::get_string(item, "title_en", std::string());

                        if (noticeItem.Codes.empty() || noticeItem.Columns.empty()) {
                            continue;
                        }

                        auto marketCode = static_cast<exchange::ExchangeId>(std::stoll(noticeItem.Codes[0].MarketCode));
                        std::string securityCode = exchange::GetSecurityCode(marketCode, noticeItem.Codes[0].StockCode);
                        std::string securityName = noticeItem.Codes[0].ShortName;

                        NoticeDetail notice;
                        notice.Code        = securityCode;
                        notice.Name        = securityName;
                        notice.DisplayTime = noticeItem.EiTime;
                        notice.NoticeDate  = noticeItem.NoticeDate;
                        notice.Title       = noticeItem.TitleCh;

                        std::vector<std::string> noticeKeywords;
                        auto                     checkRisk = [&](const std::string &content) {
                            if (content.find("减持") != std::string::npos) {
                                noticeKeywords.emplace_back("减持");
                                notice.Reduce++;
                            }
                            if (content.find("增持") != std::string::npos) {
                                noticeKeywords.emplace_back("增持");
                                notice.Increase++;
                            }
                            if (content.find("控制人变更") != std::string::npos) {
                                noticeKeywords.emplace_back("控制人变更");
                                notice.HolderChange++;
                            }
                            for (const auto &keyword : riskKeywords) {
                                if (content.find(keyword) != std::string::npos) {
                                    noticeKeywords.push_back(keyword);
                                    notice.Risk++;
                                }
                            }
                        };

                        for (const auto &column : noticeItem.Columns)
                            checkRisk(column.ColumnName);
                        checkRisk(notice.Title);

                        if (!noticeKeywords.empty()) {
                            notice.Keywords = std::accumulate(
                                noticeKeywords.begin(),
                                noticeKeywords.end(),
                                std::string(),
                                [](const std::string &a, const std::string &b) { return a.empty() ? b : a + "," + b; });
                        }

                        notices.push_back(notice);
                    }
                }
            }

            pages = GetPages(EastmoneyNoticesPageSize, noticePackage.Data.TotalHits);
            if (noticePackage.Success != 1 || notices.empty()) {
                err = ErrNoticeNotFound;
            }
        } catch (const std::exception &e) {
            spdlog::error("JSON解析错误: {}", e.what());
            err = Exception(0, "JSON解析错误: " + std::string(e.what()));
        }

        return {notices, pages, err};
    }

    /////////////////////////////////////////////////////////////////////////////
    // StockNotices - 安全版本
    /////////////////////////////////////////////////////////////////////////////
    std::tuple<std::vector<NoticeDetail>, int, Exception> StockNotices(const std::string &securityCode,
                                                                       const std::string &beginDate,
                                                                       const std::string &endDate,
                                                                       int                pageNumber) {
        std::string fixedBeginDate = exchange::timestamp(beginDate).only_date();
        std::string fixedEndDate   = endDate.empty() ? exchange::timestamp::now().only_date()
                                                     : exchange::timestamp(endDate).only_date();

        int             pageSize = EastmoneyNoticesPageSize;
        cpr::Parameters params   = {
            {"sr", "-1"},
            {"page_size", std::to_string(pageSize)},
            {"page_index", std::to_string(pageNumber)},
            {"ann_type", "A"},
            {"client_source", "web"},
            {"f_node", "0"},
            {"s_node", "0"},
            {"begin_time", fixedBeginDate},
            {"end_time", fixedEndDate},
        };

        auto marketInfo = exchange::DetectMarket(securityCode);
        params.Add({"stock_list", std::get<2>(marketInfo)});

        std::string url = urlEastmoneyNotices;
        cpr::Header headers;
        auto        response = cpr::Get(cpr::Url{url}, params, headers);

        std::vector<NoticeDetail> notices;
        int                       pages = 0;
        Exception                 err(0, "");

        if (response.status_code != 200) {
            spdlog::error("请求失败: {}", response.status_code);
            err = ErrNoticeBadApi;
            return {notices, pages, err};
        }

        try {
            auto             raw = json::parse(response.text);
            RawNoticePackage noticePackage;
            noticePackage.Success        = encoding::safe_json::get_number<int>(raw, "success", -1);
            noticePackage.Error          = encoding::safe_json::get_string(raw, "error", std::string());
            noticePackage.Data.PageIndex = encoding::safe_json::nested_get<int>(raw, {"data", "page_index"}, 1);
            noticePackage.Data.PageSize  = encoding::safe_json::nested_get<int>(
                raw, {"data", "page_size"}, EastmoneyNoticesPageSize);
            noticePackage.Data.TotalHits = encoding::safe_json::nested_get<int>(raw, {"data", "total_hits"}, 0);

            if (raw.contains("data") && raw["data"].is_object()) {
                const auto &dataObj = raw["data"];
                if (dataObj.contains("list") && dataObj["list"].is_array()) {
                    for (const auto &item : dataObj["list"]) {
                        RawNoticePackage::Data::ListItem noticeItem;
                        noticeItem.ArtCode = encoding::safe_json::get_string(item, "art_code", std::string());

                        if (item.contains("codes") && item["codes"].is_array()) {
                            for (const auto &code : item["codes"]) {
                                RawNoticePackage::Data::ListItem::CodeInfo codeInfo;
                                codeInfo.AnnType   = encoding::safe_json::get_string(code, "ann_type", std::string());
                                codeInfo.InnerCode = encoding::safe_json::get_string(code, "inner_code", std::string());
                                codeInfo.MarketCode = encoding::safe_json::get_string(
                                    code, "market_code", std::string());
                                codeInfo.ShortName = encoding::safe_json::get_string(code, "short_name", std::string());
                                codeInfo.StockCode = encoding::safe_json::get_string(code, "stock_code", std::string());
                                noticeItem.Codes.push_back(codeInfo);
                            }
                        }

                        if (item.contains("columns") && item["columns"].is_array()) {
                            for (const auto &column : item["columns"]) {
                                RawNoticePackage::Data::ListItem::ColumnInfo columnInfo;
                                columnInfo.ColumnCode = encoding::safe_json::get_string(
                                    column, "column_code", std::string());
                                columnInfo.ColumnName = encoding::safe_json::get_string(
                                    column, "column_name", std::string());
                                noticeItem.Columns.push_back(columnInfo);
                            }
                        }

                        noticeItem.DisplayTime = encoding::safe_json::get_string(item, "display_time", std::string());
                        noticeItem.EiTime      = encoding::safe_json::get_string(item, "eiTime", std::string());
                        noticeItem.Language    = encoding::safe_json::get_string(item, "language", std::string());
                        noticeItem.NoticeDate  = encoding::safe_json::get_string(item, "notice_date", std::string());
                        noticeItem.ProductCode = encoding::safe_json::get_string(item, "product_code", std::string());
                        noticeItem.SortDate    = encoding::safe_json::get_string(item, "sort_date", std::string());
                        noticeItem.SourceType  = encoding::safe_json::get_string(item, "source_type", std::string());
                        noticeItem.Title       = encoding::safe_json::get_string(item, "title", std::string());
                        noticeItem.TitleCh     = encoding::safe_json::get_string(item, "title_ch", std::string());
                        noticeItem.TitleEn     = encoding::safe_json::get_string(item, "title_en", std::string());

                        if (noticeItem.Codes.empty() || noticeItem.Columns.empty()) {
                            continue;
                        }

                        auto marketCode = static_cast<exchange::ExchangeId>(std::stoll(noticeItem.Codes[0].MarketCode));
                        std::string security_code = exchange::GetSecurityCode(marketCode,
                                                                              noticeItem.Codes[0].StockCode);
                        std::string security_name = noticeItem.Codes[0].ShortName;

                        NoticeDetail notice;
                        notice.Code        = security_code;
                        notice.Name        = security_name;
                        notice.DisplayTime = noticeItem.EiTime;
                        notice.NoticeDate  = noticeItem.NoticeDate;
                        notice.Title       = noticeItem.TitleCh;

                        std::vector<std::string> noticeKeywords;
                        auto                     checkRisk = [&](const std::string &content) {
                            if (content.find("减持") != std::string::npos) {
                                noticeKeywords.emplace_back("减持");
                                notice.Reduce++;
                            }
                            if (content.find("增持") != std::string::npos) {
                                noticeKeywords.emplace_back("增持");
                                notice.Increase++;
                            }
                            if (content.find("控制人变更") != std::string::npos) {
                                noticeKeywords.emplace_back("控制人变更");
                                notice.HolderChange++;
                            }
                            for (const auto &keyword : riskKeywords) {
                                if (content.find(keyword) != std::string::npos) {
                                    noticeKeywords.push_back(keyword);
                                    notice.Risk++;
                                }
                            }
                        };

                        for (const auto &column : noticeItem.Columns)
                            checkRisk(column.ColumnName);
                        checkRisk(notice.Title);

                        if (!noticeKeywords.empty()) {
                            notice.Keywords = std::accumulate(
                                noticeKeywords.begin(),
                                noticeKeywords.end(),
                                std::string(),
                                [](const std::string &a, const std::string &b) { return a.empty() ? b : a + "," + b; });
                        }

                        notices.push_back(notice);
                    }
                }
            }

            pages = GetPages(pageSize, noticePackage.Data.TotalHits);
            if (noticePackage.Success != 1 || notices.empty()) {
                err = ErrNoticeNotFound;
            }
        } catch (const std::exception &e) {
            spdlog::error("JSON解析错误: {}", e.what());
            err = Exception(0, "JSON解析错误: " + std::string(e.what()));
        }

        return {notices, pages, err};
    }

    /////////////////////////////////////////////////////////////////////////////
    // StockWarning - 安全版本
    /////////////////////////////////////////////////////////////////////////////
    std::pair<RawWarning, Exception> StockWarning(const std::string &securityCode, int pageNumber) {
        auto        marketInfo = exchange::DetectMarket(securityCode);
        std::string flag       = std::get<1>(marketInfo);
        flag                   = strings::to_upper(flag);

        std::map<std::string, std::string> params = {
            {"type", "RTP_F10_DETAIL"},
            {"params", std::get<2>(marketInfo) + "." + flag + ",02"},
            {"p", std::to_string(pageNumber)},
            {"ann_type", "A"},
            {"source", "HSF10"},
            {"client", "PC"},
        };

        std::string url = urlEastmoneyWarning + "?" + EncodeParams(params);
        cpr::Header headers;
        auto        response = cpr::Get(cpr::Url{url}, headers);

        RawWarning warning;
        Exception  err(0, "");

        if (response.status_code != 200) {
            spdlog::error("请求失败: {}", response.status_code);
            err = ErrNoticeBadApi;
            return {warning, err};
        }

        try {
            auto raw        = json::parse(response.text);
            warning.Code    = encoding::safe_json::get_number<int>(raw, "code", 0);
            warning.Success = encoding::safe_json::get_bool(raw, "success", false);
            warning.Message = encoding::safe_json::get_string(raw, "message", std::string());
            warning.HasNext = encoding::safe_json::get_number<int>(raw, "hasNext", 0);

            if (raw.contains("data") && raw["data"].is_array()) {
                for (const auto &dataItem : raw["data"]) {
                    std::vector<WarningDetail> details;
                    for (const auto &detail : dataItem) {
                        WarningDetail warningDetail;
                        warningDetail.EventType = encoding::safe_json::get_string(detail, "EVENT_TYPE", std::string());
                        warningDetail.SpecificEventType = encoding::safe_json::get_string(
                            detail, "SPECIFIC_EVENTTYPE", std::string());
                        warningDetail.NoticeDate = encoding::safe_json::get_string(
                            detail, "NOTICE_DATE", std::string());
                        warningDetail.Level1Content = encoding::safe_json::get_string(
                            detail, "LEVEL1_CONTENT", std::string());
                        warningDetail.InfoCode = encoding::safe_json::get_string(detail, "INFO_CODE", std::string());

                        if (detail.contains("LEVEL2_CONTENT") && detail["LEVEL2_CONTENT"].is_array()) {
                            for (const auto &content : detail["LEVEL2_CONTENT"]) {
                                if (content.is_string()) {
                                    warningDetail.Level2Content.push_back(content.get<std::string>());
                                } else {
                                    // 可选：记录日志或跳过非字符串元素
                                    spdlog::warn("LEVEL2_CONTENT 中包含非字符串元素");
                                }
                            }
                        }

                        details.push_back(warningDetail);
                    }
                    warning.Data.push_back(details);
                }
            }

            if (!warning.Success || warning.Data.empty()) {
                err = ErrNoticeNotFound;
            }
        } catch (const std::exception &e) {
            spdlog::error("JSON解析错误: {}", e.what());
            err = Exception(0, "JSON解析错误: " + std::string(e.what()));
        }

        return {warning, err};
    }

    /////////////////////////////////////////////////////////////////////////////
    // getAnnualReportDate 获取年报披露日期
    /////////////////////////////////////////////////////////////////////////////
    std::pair<std::string, std::string> getAnnualReportDate(const std::string                &year,
                                                            const std::vector<WarningDetail> &events) {
        std::string annualReportDate, quarterlyReportDate;
        for (const auto &v : events) {
            std::string date    = exchange::timestamp(v.NoticeDate).only_date();
            std::string tmpYear = date.substr(0, 4);
            if (v.EventType != "报表披露")
                continue;

            if (annualReportDate.empty() &&
                (v.SpecificEventType == "年报披露" || v.SpecificEventType == "年报预披露") && tmpYear >= year) {
                annualReportDate = date;
            } else if (quarterlyReportDate.empty() && (v.SpecificEventType.find("季报披露") != std::string::npos ||
                                                       v.SpecificEventType.find("季报预披露") != std::string::npos)) {
                quarterlyReportDate = date;
            }

            if (!annualReportDate.empty() && !quarterlyReportDate.empty())
                break;
            if (tmpYear < year)
                break;
        }
        return {annualReportDate, quarterlyReportDate};
    }

    /////////////////////////////////////////////////////////////////////////////
    // NoticeDateForReport 年报季报披露日期
    /////////////////////////////////////////////////////////////////////////////
    std::tuple<std::string, std::string> NoticeDateForReport(const std::string &code, const std::string &date) {
        const std::string fixedDate = exchange::timestamp(date).only_date();
        const std::string year      = fixedDate.substr(0, 4);
        int               pageNo    = 1;
        std::string       annualReportDate, quarterlyReportDate;

        while (true) {
            auto [warning, err] = StockWarning(code, pageNo);
            if (err.code() != 0)
                break;

            for (const auto &events : warning.Data) {
                auto [tmpYearReportDate, tmpQuarterlyReportDate] = getAnnualReportDate(year, events);
                if (annualReportDate.empty() && !tmpYearReportDate.empty())
                    annualReportDate = tmpYearReportDate;
                if (quarterlyReportDate.empty() && !tmpQuarterlyReportDate.empty())
                    quarterlyReportDate = tmpQuarterlyReportDate;
                if (!annualReportDate.empty() && !quarterlyReportDate.empty())
                    break;
            }

            if (!annualReportDate.empty() && !quarterlyReportDate.empty())
                break;
            if (warning.HasNext > 0)
                pageNo++;
            else
                break;
        }

        return {annualReportDate, quarterlyReportDate};
    }

    /////////////////////////////////////////////////////////////////////////////
    // getOneNotice 获取最近公告
    /////////////////////////////////////////////////////////////////////////////
    CompanyNotice getOneNotice(const std::string &securityCode, const std::string &currentDate) {
        CompanyNotice notice{};
        if (!exchange::AssertStockBySecurityCode(securityCode))
            return notice;

        exchange::timestamp timestamp(currentDate);
        timestamp                                = timestamp.offset(-24 * 30);
        const std::string             beginDate  = timestamp.only_date();
        const std::string            &endDate    = currentDate;
        int                           pagesCount = 1;
        std::unique_ptr<NoticeDetail> tmpNotice;

        for (int pageNo = 1; pageNo <= pagesCount; ++pageNo) {
            auto [list, pages, err] = dfcf::StockNotices(securityCode, beginDate, endDate, pageNo);
            if (err.code() != 0 || pages < 1)
                break;

            if (pagesCount < pages)
                pagesCount = pages;
            if (list.empty())
                break;

            for (const auto &v : list) {
                if (tmpNotice) {
                    tmpNotice->Name = v.Name;
                    if (tmpNotice->NoticeDate < v.NoticeDate) {
                        tmpNotice->DisplayTime = v.DisplayTime;
                        tmpNotice->NoticeDate  = v.NoticeDate;
                    }
                    tmpNotice->Title = v.Title;

                    std::string keywords = tmpNotice->Keywords;
                    if (!v.Keywords.empty()) {
                        if (keywords.empty())
                            keywords = v.Keywords;
                        else
                            keywords += "," + v.Keywords;
                    }

                    std::vector<std::string> tmpArr;
                    size_t                   start = 0, end = keywords.find(',');
                    while (end != std::string::npos) {
                        tmpArr.push_back(keywords.substr(start, end - start));
                        start = end + 1;
                        end   = keywords.find(',', start);
                    }
                    tmpArr.push_back(keywords.substr(start));
                    tmpArr = strings::unique(tmpArr);

                    tmpNotice->Keywords = "";
                    for (size_t i = 0; i < tmpArr.size(); ++i) {
                        if (i != 0)
                            tmpNotice->Keywords += ",";
                        tmpNotice->Keywords += tmpArr[i];
                    }

                    tmpNotice->Increase += v.Increase;
                    tmpNotice->Reduce += v.Reduce;
                    tmpNotice->HolderChange += v.HolderChange;
                    tmpNotice->Risk += v.Risk;
                } else {
                    tmpNotice = std::make_unique<NoticeDetail>(v);
                }
            }

            if (list.size() < dfcf::EastmoneyNoticesPageSize)
                break;
        }

        if (tmpNotice) {
            notice.Increase     = tmpNotice->Increase;
            notice.Reduce       = tmpNotice->Reduce;
            notice.Risk         = tmpNotice->Risk;
            notice.RiskKeywords = tmpNotice->Keywords;
        }

        return notice;
    }

    /////////////////////////////////////////////////////////////////////////////
    // 工具函数实现
    /////////////////////////////////////////////////////////////////////////////
    namespace {
        int GetPages(const int pageSize, const int totalHits) {
            return (totalHits + pageSize - 1) / pageSize;
        }

        std::string UrlEncode(const std::string &value) {
            std::ostringstream escaped;
            escaped.fill('0');
            escaped << std::hex << std::uppercase;
            for (char c : value) {
                if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                    escaped << c;
                } else {
                    escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
                }
            }
            return escaped.str();
        }

        std::string EncodeParams(const std::map<std::string, std::string> &params) {
            std::string encoded;
            for (const auto &param : params) {
                if (!encoded.empty())
                    encoded += "&";
                encoded += UrlEncode(param.first) + "=" + UrlEncode(param.second);
            }
            return encoded;
        }
    }  // anonymous namespace

}  // namespace dfcf