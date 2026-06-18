#include <quant1x/encoding/csv.h>
#include <quant1x/data/meta/timestamp.h>
#include <quant1x/contrib/data/tdx/client.h>
#include <quant1x/data/xdxr.h>
#include <quant1x/factors/f10.h>
#include <quant1x/factors/base_compat.h>
#include <quant1x/contrib/data/tdx/kline.h>
#include <quant1x/factors/notice.h>
#include <quant1x/factors/financial_report.h>
#include <quant1x/factors/share_holder.h>
#include <quant1x/factors/safety_score.h>

static std::string get_ipo_date(const std::string &security_code, const std::string &feature_date) {
    auto kls = tdx::checkout_klines(security_code, feature_date);
    if(kls.empty()) {
        return "";
    }
    return kls[0].date;
}

// 获取财务数据
static std::tuple<f64, f64, std::string, std::string> get_finance_info(const std::string &security_code,
                                                                       const std::string &feature_date) {
    f64 capital = 0, totalCapital = 0;
    std::string ipo_date, update_date;
    u32 base_date = data::market_first_date.yyyymmdd();
    try {
        tdx::FinanceRequest request(security_code);
        tdx::FinanceResponse response{};
        auto conn = tdx::get_std_conn();
        tdx::process_message(conn->socket(), request, response);
        if(response.Count>0) {
            auto const &info = response.Info;
            if(info.LiuTongGuBen>0 && info.ZongGuBen > 0) {
                capital = info.LiuTongGuBen;
                totalCapital = info.ZongGuBen;
            }
            if(info.IPODate>=base_date) {
                meta::Timestamp ts(std::to_string(info.IPODate));
                ipo_date = ts.only_date();
            } else {
                ipo_date = get_ipo_date(security_code, feature_date);
            }
            if(info.UpdatedDate>=base_date) {
                meta::Timestamp ts(std::to_string(info.UpdatedDate));
                update_date = ts.only_date();
            }
        }
    } catch (...) {
        // 忽略异常
    }
    return {capital, totalCapital, ipo_date, update_date};
}

static const tdx::XdxrInfo *checkoutCapital(const std::vector<tdx::XdxrInfo> &list,
                                        const std::string& date) {
    for (const auto& v : list) {  // 直接遍历已排序的vector
        if (v.IsCapitalChange() && date >= v.Date) {
            return &v;  // 找到第一个满足条件的立即返回
        }
    }
    return nullptr;  // 没找到返回空指针
}

struct f10SecurityInfo {
    f64 TotalCapital;
    f64 Capital;
    int VolUnit;
    int    DecimalPoint;
    std::string Name_;
    std::string IpoDate;
    bool    SubNew;
    std::string UpdateDate;
};

static f10SecurityInfo checkoutSecurityBasicInfo(const std::string &security_code, const std::string &feature_date) {
    f10SecurityInfo info{};
    auto list = data::load_xdxr(security_code);
    std::sort(list.begin(), list.end(), [](const tdx::XdxrInfo &a, const tdx::XdxrInfo& b){
        return a.Date > b.Date;
    });

    // 直接转换Go版本的checkoutCapital
    const auto* xdxr = checkoutCapital(list, feature_date);
    if (xdxr != nullptr) {
        info.TotalCapital = xdxr->HouZongGuBen * config::TenThousand;
        info.Capital = xdxr->HouLiuTong * config::TenThousand;
    } else {
        auto [Capital, TotalCapital, IpoDate, UpdateDate] = get_finance_info(security_code, feature_date);
        info.Capital = Capital;
        info.TotalCapital = TotalCapital;
        info.IpoDate = IpoDate;
        info.UpdateDate = UpdateDate;
    }
    if (info.IpoDate.empty()) {
        info.IpoDate = get_ipo_date(security_code, feature_date);
    }
    if (info.UpdateDate.empty() || info.UpdateDate > feature_date) {
        info.UpdateDate = feature_date;
    }
    if (!info.IpoDate.empty()) {
        // 计算是否次新股, 线性判断1年内的个股为次新股
        meta::Timestamp ipo_date(info.IpoDate);
        auto [y,m,d] = ipo_date.extract();
        ipo_date = meta::Timestamp(y+1, m, d);
        meta::Timestamp current(feature_date);

        info.SubNew = current < ipo_date;
    }
    auto securityInfo = instruments::get_instrument_info(security_code);
    if(securityInfo.has_value()) {
        info.VolUnit = securityInfo->lotSize;
        info.DecimalPoint = securityInfo->pricePrecision;
        info.Name_ = securityInfo->name;
    } else {
        info.VolUnit = 100;
        info.DecimalPoint = 2;
        info.Name_ = "Unknown";
    }

    return info;
}

static std::tuple<double, double, double, double, double>
ComputeFreeCapital(const std::vector<dfcf::CirculatingShareholder>& holderList, double capital) {
    double top10Capital = 0.0;
    double freeCapital = capital; // 初始自由流通股本等于总股本
    double capitalChanged = 0.0;
    double increaseRatio = 0.0;
    double reductionRatio = 0.0;

    int64_t increase = 0;
    int64_t reduce = 0;

    for (size_t k = 0; k < holderList.size(); ++k) {
        const auto& holder = holderList[k];

        top10Capital += static_cast<double>(holder.HoldNum);
        capitalChanged += static_cast<double>(holder.HoldNumChange);

        if (holder.HoldNumChange >= 0) {
            increase += holder.HoldNumChange;
        } else {
            reduce += holder.HoldNumChange;
        }

        if (k >= 10) {
            continue;
        }

        if (holder.FreeHoldNumRatio >= 1.00 && holder.IsHoldOrg == "1") {
            freeCapital -= static_cast<double>(holder.HoldNum);
        }
    }

    if (top10Capital > 0.0) {
        increaseRatio = 100.0 * (static_cast<double>(increase) / top10Capital);
        reductionRatio = 100.0 * (static_cast<double>(reduce) / top10Capital);
    }

    return {top10Capital, freeCapital, capitalChanged, increaseRatio, reductionRatio};
}

struct Top10ShareHolder {
    std::string Code;
    double FreeCapital{};
    double Top10Capital{};
    double Top10Change{};
    double ChangeCapital{};
    double IncreaseRatio{};
    double ReductionRatio{};
};

// 主函数实现
static std::unique_ptr<Top10ShareHolder> checkoutShareHolder(const std::string &securityCode,
                                                             const std::string &featureDate) {
    // 获取除权除息列表并排序
    auto xdxrs = data::load_xdxr(securityCode);
    std::sort(xdxrs.begin(), xdxrs.end(), [](const tdx::XdxrInfo& a, const tdx::XdxrInfo& b) {
        return a.Date > b.Date;
    });

    // 检查资本信息
    auto xdxrInfo = checkoutCapital(xdxrs, featureDate);
    if (xdxrInfo && data::assert_stock_by_security_code(securityCode)) {
        // 获取股东列表
        auto list = dfcf::GetCacheShareHolder(securityCode, featureDate);

        // 计算资本
        double capital = xdxrInfo->HouLiuTong * 10000;
        double totalCapital = xdxrInfo->HouZongGuBen * 10000;

        // 计算自由流通股本等指标
        auto [top10Capital, freeCapital, capitalChanged, increaseRatio, reductionRatio] =
            ComputeFreeCapital(list, capital);

        // 如果自由流通股本为负, 使用总股本重新计算
        if (freeCapital < 0) {
            std::tie(top10Capital, freeCapital, capitalChanged, increaseRatio, reductionRatio) =
                ComputeFreeCapital(list, totalCapital);
        }

        // 获取前期数据
        const auto frontList = dfcf::GetCacheShareHolder(securityCode, featureDate, 2);
        auto [frontTop10Capital, x1, x2, x3, x4] = ComputeFreeCapital(frontList, totalCapital);

        // 构建返回结果
        auto shareHolder = std::make_unique<Top10ShareHolder>();
        shareHolder->Code = securityCode;
        shareHolder->FreeCapital = freeCapital;
        shareHolder->Top10Capital = top10Capital;
        shareHolder->Top10Change = top10Capital - frontTop10Capital;
        shareHolder->ChangeCapital = capitalChanged;
        shareHolder->IncreaseRatio = increaseRatio;
        shareHolder->ReductionRatio = reductionRatio;

        return shareHolder;
    }

    return nullptr;
}

data::Kind F10Feature::Kind() const {
    return factors::FeatureF10;
}

std::string F10Feature::Owner() {
    return data::DefaultDataProvider;
}

std::string F10Feature::Key() const {
    return "f10";
}

std::string F10Feature::Name() const {
    return "F10";
}

std::string F10Feature::Usage() const {
    return "F10";
}

void F10Feature::Print(const meta::Instrument &inst, const std::vector<meta::Timestamp> &dates) {
    (void)inst;
    (void)dates;
}

void F10Feature::Update(const meta::Instrument &inst, const meta::Timestamp &date) {
    auto code = inst.symbol();
    f10 = F10{};
    std::string feature_date = date.only_date();
    f10.Date =  meta::next_trading_day(date).only_date();;
    std::string securityCode = data::correct_security_code(code);
    spdlog::debug("update f10, code={}", securityCode);
    // 1. 基本信息
    f10SecurityInfo securityInfo = checkoutSecurityBasicInfo(securityCode, feature_date);
    {
        f10.Code = securityCode;
        f10.TotalCapital= securityInfo.TotalCapital;
        f10.Capital = securityInfo.Capital;
        f10.VolUnit = securityInfo.VolUnit;
        f10.DecimalPoint = securityInfo.DecimalPoint;
        f10.SecurityName = securityInfo.Name_;
        f10.IpoDate = securityInfo.IpoDate;
        f10.SubNew = securityInfo.SubNew;
        f10.UpdateDate = securityInfo.UpdateDate;
        f10.MarginTradingTarget = data::is_margin_trading_target(securityCode);
    }

    // 2. 前十大流通股股东
    auto shareHolder = checkoutShareHolder(securityCode, feature_date);
    if(shareHolder) {
        securityCode = shareHolder->Code;
        f10.FreeCapital = shareHolder->FreeCapital;
        f10.Top10Capital = shareHolder->Top10Capital;
        f10.Top10Change = shareHolder->Top10Change;
        f10.ChangeCapital = shareHolder->ChangeCapital;
        f10.IncreaseRatio = shareHolder->IncreaseRatio;
        f10.ReductionRatio = shareHolder->ReductionRatio;
    }
    // 如果经过前面的处理, 自由流通股本为0, 则视为流通股本等同于自由流通股本
    if (f10.FreeCapital == 0) {
        f10.FreeCapital = f10.Capital;
    }

    // 3. 上市公司公告
    auto notice = dfcf::getOneNotice(securityCode, feature_date);
    f10.Increases = notice.Increase;
    f10.Reduces = notice.Reduce;
    f10.Risk = notice.Risk;
    f10.RiskKeywords = notice.RiskKeywords;

    // 4. 季报
    auto [q, x1, x2] = api::GetQuarterByDate(feature_date, 1);
    f10.QuarterlyYearQuarter = q;
    {
        auto report = dfcf::getQuarterlyReportSummary(securityCode, feature_date);
        f10.QDate = report.QDate;
        f10.BPS = report.BPS;
        f10.BasicEPS = report.BasicEPS;
        f10.TotalOperateIncome = report.TotalOperateIncome;
        f10.DeductBasicEPS = report.DeductBasicEPS;
    }

    // 5. 安全分
    auto [SafetyScore, reason] = risks::GetSafetyScore(securityCode);
    f10.SafetyScore = SafetyScore;

    // 6. 年报季报披露日期
    auto [annualReportDate, quarterlyReportDate] = dfcf::NoticeDateForReport(securityCode, feature_date);
    f10.AnnualReportDate = annualReportDate;
    f10.QuarterlyReportDate = quarterlyReportDate;

    // 特征通用状态
    f10.UpdateTime = api::get_timestamp();
    f10.State |= Kind();
    spdlog::debug("update f10, code={}, OK", securityCode);
}

std::unique_ptr<data::FeatureAdapter> F10Feature::clone() const {
    return std::make_unique<F10Feature>(*this);
}

std::vector<std::string> F10Feature::headers() const {
    //return "Date,Code,SecurityName,SubNew,MarginTradingTarget,VolUnit,DecimalPoint,IpoDate,UpdateDate,TotalCapital,Capital,FreeCapital,Top10Capital,Top10Change,ChangeCapital,IncreaseRatio,ReductionRatio,QuarterlyYearQuarter,QDate,AnnualReportDate,QuarterlyReportDate,TotalOperateIncome,BPS,BasicEPS,DeductBasicEPS,SafetyScore,Increases,Reduces,Risk,RiskKeywords,UpdateTime,State";
    std::vector<std::string> header;
    boost::pfr::for_each_field(F10{}, [&](auto& field, auto idx) {
        (void)field;
        constexpr auto field_name = boost::pfr::get_name<idx, F10>();
        header.emplace_back(field_name);
    });
    return header;
}

std::vector<std::string> F10Feature::values() const {
    std::vector<std::string> row;
    boost::pfr::for_each_field(f10, [&](auto& field, auto /*idx*/) {
        row.emplace_back(encoding::csv::detail::to_csv_string(field));
    });
    return row;
}

void F10Feature::init(const meta::Timestamp &timestamp) {
    const std::string feature_date = timestamp.only_date();
    dfcf::loadQuarterlyReports(feature_date);
}

namespace factors {

    namespace {
        inline std::mutex g_factor_f10_mutex{};
        inline tsl::robin_map<std::string, F10> g_factor_f10_map{};
        inline meta::Timestamp              g_factor_f10_date{};
    }

    /// 获取指定日期的F10数据
    std::optional<F10> get_f10(const std::string& code, const meta::Timestamp& timestamp) {
        {
            std::lock_guard<std::mutex> lock{g_factor_f10_mutex};
            meta::Timestamp algin_date = timestamp.pre_market_time();
            if(g_factor_f10_map.empty() || g_factor_f10_date != algin_date) {
                g_factor_f10_date = algin_date;
                auto adapter = F10Feature();
                auto cache_filename = adapter.Filename(g_factor_f10_date);
                if(!std::filesystem::exists(cache_filename)) {
                    spdlog::error("[f10] cache file[{}], not found", cache_filename);
                    return std::nullopt;
                }
                std::vector<F10> list = encoding::csv::csv_to_slices<F10>(cache_filename);
                for(auto const &v : list) {
                    g_factor_f10_map.insert_or_assign(v.Code, v);
                }
            }
        }
        auto it = g_factor_f10_map.find(code);
        if(it != g_factor_f10_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

} // namespace factors