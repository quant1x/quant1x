#include <quant1x/test/test.h>

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <quant1x/exchange.h>
#include <quant1x/factors/financial_report.h>
#include <quant1x/factors/notice.h>
#include <quant1x/encoding/csv.h>

TEST_CASE("notice-basic", "[f10]") {
    std::string code = "600600";
    std::string date = "20250501";
    auto [notices, pages, success] = dfcf::StockNotices(code, "20250101");
    std::cout << "Got " << notices.size() << " notices, total pages: " << pages << std::endl;
    auto [y, q] = dfcf::NoticeDateForReport(code, date);
    std::cout << y << ", " << q << std::endl;
}

TEST_CASE("notice-reports", "[f10]") {
    runtime::global_init();
    runtime::logger_set(true, true);
    auto x = dfcf::GetCacheQuarterlyReportsBySecurityCode("sh600600", "20250720");
    std::cout << x.value() << std::endl;
}

TEST_CASE("reports-csv", "[f10]") {
    dfcf::QuarterlyReport qr{};
    std::vector<dfcf::QuarterlyReport> tmp;
    qr.SecuCode = "sh000001";
    tmp.emplace_back(qr);
    encoding::csv::slices_to_csv( tmp, "1.csv");
    auto tmp2 = encoding::csv::csv_to_slices<dfcf::QuarterlyReport>("1.csv");
    std::cout<< tmp2.size() << std::endl;
}