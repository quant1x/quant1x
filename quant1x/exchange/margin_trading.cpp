#include <quant1x/exchange/margin_trading.h>

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>
#include <atomic>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <tuple>
#include <algorithm>
#include <quant1x/market/instruments.h>
#include <quant1x/config/config.h>
#include <quant1x/io/file.h>
#include <quant1x/io/csv-reader.h>
#include <quant1x/encoding/json.h>
#include <quant1x/std/filesystem.h>

using json = nlohmann::json;

namespace exchange {

    // 常量定义
    const std::string marginTradingFilename = "margin-trading.csv";
    const std::string urlEastMoneyApiRZRQ = "https://datacenter-web.eastmoney.com/api/data/v1/get";
    constexpr const int rzrqPageSize = 500;

    // 融资融券数据结构
    struct SecurityMarginTrading {
        std::string DATE;            // 日期
        std::string MARKET;          // 市场
        std::string SCODE;           // 代码
        std::string SECNAME;         // 证券名称
        double RZYE = 0;             // 融资余额(元)
        double RQYL = 0;             // 融券余量(股)
        double RZRQYE = 0;           // 融资融券余额(元)
        double RQYE = 0;             // 融券余额(元)
        double RQMCL = 0;            // 融券卖出量(股)
        double RZRQYECZ = 0;         // 融资融券余额差值(元)
        double RZMRE = 0;            // 融资买入额(元)
        double SZ = 0;               // SZ
        double RZYEZB = 0;           // 融资余额占流通市值比(%)
        double RZMRE3D = 0;          // 3日融资买入额(元)
        double RZMRE5D = 0;          // 5日融资买入额(元)
        double RZMRE10D = 0;         // 10日融资买入额(元)
        double RZCHE = 0;            // 融资偿还额(元)
        double RZCHE3D = 0;          // 3日融资偿还额(元)
        double RZCHE5D = 0;          // 5日融资偿还额(元)
        double RZCHE10D = 0;         // 10日融资偿还额(元)
        double RZJME = 0;            // 融资净买额(元)
        double RZJME3D = 0;          // 3日融资净买额(元)
        double RZJME5D = 0;          // 5日融资净买额(元)
        double RZJME10D = 0;         // 10日融资净买额(元)
        double RQMCL3D = 0;          // 3日融券卖出量(股)
        double RQMCL5D = 0;          // 5日融券卖出量(股)
        double RQMCL10D = 0;         // 10日融券卖出量(股)
        double RQCHL = 0;            // 融券偿还量(股)
        double RQCHL3D = 0;          // 3日融券偿还量(股)
        double RQCHL5D = 0;          // 5日融券偿还量(股)
        double RQCHL10D = 0;         // 10日融券偿还量(股)
        double RQJMG = 0;            // 融券净卖出(股)
        double RQJMG3D = 0;          // 3日融券净卖出(股)
        double RQJMG5D = 0;          // 5日融券净卖出(股)
        double RQJMG10D = 0;         // 10日融券净卖出(股)
        double SPJ = 0;              // 收盘价
        double ZDF = 0;              // 涨跌幅
        double RChange3DCP = 0;      // 3日未识别
        double RChange5DCP = 0;      // 5日未识别
        double RChange10DCP = 0;     // 10日未识别
        int KCB = 0;                 // 科创板
        std::string TRADE_MARKET_CODE; // 二级市场代码
        std::string TRADE_MARKET;    // 二级市场
        double FIN_BALANCE_GR = 0;    // 融资余额增长率
        std::string SECUCODE;        // 证券代码
    };

    // 原始融资融券数据结构
    struct RawMarginTrading {
        std::string version;
        struct Result {
            int pages = 0;
            std::vector<SecurityMarginTrading> data;
            int count = 0;
        } result;
        bool success = false;
        std::string message;
        int code = 0;
    };

    // HTTP请求回调函数
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    // 执行HTTP GET请求
    std::string httpGet(const std::string& url) {
        CURL* curl = curl_easy_init();
        std::string response;

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                throw std::runtime_error("HTTP request failed: " + std::string(curl_easy_strerror(res)));
            }

            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code != 200) {
                curl_easy_cleanup(curl);
                throw std::runtime_error("HTTP request failed with code: " + std::to_string(http_code));
            }

            curl_easy_cleanup(curl);
        } else {
            throw std::runtime_error("Failed to initialize CURL");
        }

        return response;
    }

    // URL编码辅助函数
    std::string urlEncode(const std::string &value) {
        CURL *curl = curl_easy_init();
        if(curl) {
            char *output = curl_easy_escape(curl, value.c_str(), int(value.length()));
            if(output) {
                std::string result(output);
                curl_free(output);
                curl_easy_cleanup(curl);
                return result;
            }
            curl_easy_cleanup(curl);
        }
        return value;
    }

    // 获取原始融资融券列表
    std::tuple<std::vector<SecurityMarginTrading>, int, std::string> rawMarginTradingList(const std::string& date, int pageNumber) {
        exchange::timestamp ts = date;
        std::string tradeDate = ts.only_date();

        std::stringstream urlStream;
        urlStream << urlEastMoneyApiRZRQ << "?"
                  << "reportName=RPTA_WEB_RZRQ_GGMX"
                  << "&columns=ALL"
                  << "&source=WEB"
                  << "&sortColumns=scode"
                  << "&sortTypes=1"
                  << "&pageSize=" << rzrqPageSize
                  << "&pageNumber=" << pageNumber
                  << "&client=WEB"
                  << "&filter=(DATE='" << tradeDate << "')";

        std::string url = urlStream.str();
        std::string response;

        try {
            response = httpGet(url);
        } catch (const std::exception& e) {
            return {{}, 0, e.what()};
        }

        try {
            auto j = json::parse(response);
            RawMarginTrading raw;

            // 解析JSON数据
            raw.version = j.value("version", "");
            raw.result.pages = j["result"].value("pages", 0);
            raw.result.count = j["result"].value("count", 0);
            raw.success = j.value("success", false);
            raw.message = j.value("message", "");
            raw.code = j.value("code", 0);

            // 解析数据数组
            for (auto& item : j["result"]["data"]) {
                SecurityMarginTrading smt;
                smt.DATE = encoding::safe_json::get_string<std::string>(item, "DATE", "");
                smt.MARKET = encoding::safe_json::get_string<std::string>(item, "MARKET", "");
                smt.SCODE = encoding::safe_json::get_string<std::string>(item, "SCODE", "");
                smt.SECNAME = encoding::safe_json::get_string<std::string>(item, "SECNAME", "");
                smt.RZYE = encoding::safe_json::get_number<double>(item, "RZYE", 0.0);
                smt.RQYL = encoding::safe_json::get_number<double>(item, "RQYL", 0.0);
                smt.RZRQYE = encoding::safe_json::get_number<double>(item, "RZRQYE", 0.0);
                smt.RQYE = encoding::safe_json::get_number<double>(item, "RQYE", 0.0);
                smt.RQMCL = encoding::safe_json::get_number<double>(item, "RQMCL", 0.0);
                smt.RZRQYECZ = encoding::safe_json::get_number<double>(item, "RZRQYECZ", 0.0);
                smt.RZMRE = encoding::safe_json::get_number<double>(item, "RZMRE", 0.0);
                smt.SZ = encoding::safe_json::get_number<double>(item, "SZ", 0.0);
                smt.RZYEZB = encoding::safe_json::get_number<double>(item, "RZYEZB", 0.0);
                smt.RZMRE3D = encoding::safe_json::get_number<double>(item, "RZMRE3D", 0.0);
                smt.RZMRE5D = encoding::safe_json::get_number<double>(item, "RZMRE5D", 0.0);
                smt.RZMRE10D = encoding::safe_json::get_number<double>(item, "RZMRE10D", 0.0);
                smt.RZCHE = encoding::safe_json::get_number<double>(item, "RZCHE", 0.0);
                smt.RZCHE3D = encoding::safe_json::get_number<double>(item, "RZCHE3D", 0.0);
                smt.RZCHE5D = encoding::safe_json::get_number<double>(item, "RZCHE5D", 0.0);
                smt.RZCHE10D = encoding::safe_json::get_number<double>(item, "RZCHE10D", 0.0);
                smt.RZJME = encoding::safe_json::get_number<double>(item, "RZJME", 0.0);
                smt.RZJME3D = encoding::safe_json::get_number<double>(item, "RZJME3D", 0.0);
                smt.RZJME5D = encoding::safe_json::get_number<double>(item, "RZJME5D", 0.0);
                smt.RZJME10D = encoding::safe_json::get_number<double>(item, "RZJME10D", 0.0);
                smt.RQMCL3D = encoding::safe_json::get_number<double>(item, "RQMCL3D", 0.0);
                smt.RQMCL5D = encoding::safe_json::get_number<double>(item, "RQMCL5D", 0.0);
                smt.RQMCL10D = encoding::safe_json::get_number<double>(item, "RQMCL10D", 0.0);
                smt.RQCHL = encoding::safe_json::get_number<double>(item, "RQCHL", 0.0);
                smt.RQCHL3D = encoding::safe_json::get_number<double>(item, "RQCHL3D", 0.0);
                smt.RQCHL5D = encoding::safe_json::get_number<double>(item, "RQCHL5D", 0.0);
                smt.RQCHL10D = encoding::safe_json::get_number<double>(item, "RQCHL10D", 0.0);
                smt.RQJMG = encoding::safe_json::get_number<double>(item, "RQJMG", 0.0);
                smt.RQJMG3D = encoding::safe_json::get_number<double>(item, "RQJMG3D", 0.0);
                smt.RQJMG5D = encoding::safe_json::get_number<double>(item, "RQJMG5D", 0.0);
                smt.RQJMG10D = encoding::safe_json::get_number<double>(item, "RQJMG10D", 0.0);
                smt.SPJ = encoding::safe_json::get_number<double>(item, "SPJ", 0.0);
                smt.ZDF = encoding::safe_json::get_number<double>(item, "ZDF", 0.0);
                smt.RChange3DCP = encoding::safe_json::get_number<double>(item, "RCHANGE3DCP", 0.0);
                smt.RChange5DCP = encoding::safe_json::get_number<double>(item, "RCHANGE5DCP", 0.0);
                smt.RChange10DCP = encoding::safe_json::get_number<double>(item, "RCHANGE10DCP", 0.0);
                smt.KCB = encoding::safe_json::get_number<int>(item, "KCB", 0);
                smt.TRADE_MARKET_CODE = encoding::safe_json::get_string<std::string>(item, "TRADE_MARKET_CODE", "");
                smt.TRADE_MARKET = encoding::safe_json::get_string<std::string>(item, "TRADE_MARKET", "");
                smt.FIN_BALANCE_GR = encoding::safe_json::get_number<double>(item, "FIN_BALANCE_GR", 0.0);
                smt.SECUCODE = encoding::safe_json::get_string<std::string>(item, "SECUCODE", "");

                raw.result.data.push_back(smt);
            }

            return {raw.result.data, raw.result.pages, ""};
        } catch (const std::exception& e) {
            return {{}, 0, e.what()};
        }
    }

    // 获取融资融券日期
    std::string getMarginTradingDate() {
        return exchange::prev_trading_day().only_date();
    }

    // 获取融资融券列表
    std::vector<SecurityMarginTrading> GetMarginTradingList() {
        std::string date = getMarginTradingDate();
        std::vector<SecurityMarginTrading> list;
        int pages = 1;

        for (int i = 0; i < pages; i++) {
            auto [tmpList, tmpPages, err] = rawMarginTradingList(date, i + 1);
            if (!err.empty()) {
                break;
            }

            list.insert(list.end(), tmpList.begin(), tmpList.end());

            if (tmpList.size() < rzrqPageSize) {
                break;
            }

            if (pages == 1) {
                pages = tmpPages;
            }
        }

        return list;
    }

    // 全局变量
    static inline auto _margin_trading_once = RollingOnce::create("exchange-margin-trading", cron_expr_daily_9am);
    static inline std::vector<std::string> _margin_trading_cache_list;
    static inline std::unordered_map<std::string, bool> _margin_trading_cache_map;
    static inline std::mutex _margin_trading_cache_mutex;

    // 懒加载融资融券数据
    void lazyLoadMarginTrading() {
        spdlog::info("初始化融资融券...");
        std::lock_guard<std::mutex> lock(_margin_trading_cache_mutex);

        std::string cache_filename = config::get_meta_path() + "/" + marginTradingFilename;
        filesystem::check_filepath(cache_filename, true);
        // 1. 获取缓存文件状态
        i64 lastModified = 0;
        if (std::filesystem::exists(cache_filename)) {
            lastModified = io::last_modified_time(cache_filename);
        }

        // 2. 临时两融列表
        std::vector<SecurityMarginTrading> tempList;

        // 3. 比较缓存日期和最新交易日期
        exchange::timestamp cache_timestamp(lastModified);

        if (cache_timestamp.only_date() < exchange::last_trading_day().only_date()) {
            // 缓存过期，下载最新数据
            auto list = GetMarginTradingList();
            for (auto& v : list) {
                std::string securityCode = exchange::CorrectSecurityCode(v.SECUCODE);
                v.SECUCODE = securityCode;
                // 这里只缓存了证券代码
                tempList.push_back(v);
            }

            // 刷新本地缓存文件
            if (!tempList.empty()) {
                io::CSVWriter writer(cache_filename);
                writer.write_row("DATE", "MARKET", "SCODE", "SECNAME",
                                 "RZYE", "RQYL", "RZRQYE", "RQYE", "RQMCL", "RZRQYECZ",
                                 "RZMRE", "SZ", "RZYEZB", "RZMRE3D", "RZMRE5D", "RZMRE10D",
                                 "RZCHE", "RZCHE3D", "RZCHE5D", "RZCHE10D",
                                 "RZJME", "RZJME3D", "RZJME5D", "RZJME10D",
                                 "RQMCL3D", "RQMCL5D", "RQMCL10D",
                                 "RQCHL", "RQCHL3D", "RQCHL5D", "RQCHL10D",
                                 "RQJMG", "RQJMG3D", "RQJMG5D", "RQJMG10D",
                                 "SPJ", "ZDF", "RCHANGE3DCP", "RCHANGE5DCP", "RCHANGE10DCP",
                                 "KCB", "TRADE_MARKET_CODE", "TRADE_MARKET", "FIN_BALANCE_GR", "SECUCODE");
                for (auto const &record: list) {
                    writer.write_row(record.DATE, record.MARKET, record.SCODE, record.SECNAME,
                                     record.RZYE, record.RQYL, record.RZRQYE, record.RQYE, record.RQMCL, record.RZRQYECZ,
                                     record.RZMRE, record.SZ, record.RZYEZB, record.RZMRE3D, record.RZMRE5D, record.RZMRE10D,
                                     record.RZCHE, record.RZCHE3D, record.RZCHE5D, record.RZCHE10D,
                                     record.RZJME, record.RZJME3D, record.RZJME5D, record.RZJME10D,
                                     record.RQMCL3D, record.RQMCL5D, record.RQMCL10D,
                                     record.RQCHL, record.RQCHL3D, record.RQCHL5D, record.RQCHL10D,
                                     record.RQJMG, record.RQJMG3D, record.RQJMG5D, record.RQJMG10D,
                                     record.SPJ, record.ZDF, record.RChange3DCP, record.RChange5DCP, record.RChange10DCP,
                                     record.KCB, record.TRADE_MARKET_CODE, record.TRADE_MARKET, record.FIN_BALANCE_GR, record.SECUCODE);
                }
            }
        }

//        // 4. 如果文件不存在，从内置资源文件导出
//        if (tempList.empty() && !std::filesystem::exists(target)) {
//            std::string filename = ResourcesPath + "/" + marginTradingFilename;
//            try {
//                std::ifstream srcFile(filename, std::ios::binary);
//                std::ofstream dstFile(target, std::ios::binary);
//                dstFile << srcFile.rdbuf();
//            } catch (...) {
//                // 忽略导出错误
//            }
//        }

        // 5. 如果没有更新，从缓存加载
        if (tempList.empty() && std::filesystem::exists(cache_filename)) {
            try {
                // 配置CSV解析器（45个字段对应所有数据列）
                io::CSVReader<45, io::trim_chars<' ', '\t'>,
                    io::double_quote_escape<',', '"'>> in(cache_filename);

                // 读取CSV头部
                in.read_header(io::ignore_missing_column | io::ignore_extra_column,
                               "DATE", "MARKET", "SCODE", "SECNAME",
                               "RZYE", "RQYL", "RZRQYE", "RQYE", "RQMCL", "RZRQYECZ",
                               "RZMRE", "SZ", "RZYEZB", "RZMRE3D", "RZMRE5D", "RZMRE10D",
                               "RZCHE", "RZCHE3D", "RZCHE5D", "RZCHE10D",
                               "RZJME", "RZJME3D", "RZJME5D", "RZJME10D",
                               "RQMCL3D", "RQMCL5D", "RQMCL10D",
                               "RQCHL", "RQCHL3D", "RQCHL5D", "RQCHL10D",
                               "RQJMG", "RQJMG3D", "RQJMG5D", "RQJMG10D",
                               "SPJ", "ZDF", "RCHANGE3DCP", "RCHANGE5DCP", "RCHANGE10DCP",
                               "KCB", "TRADE_MARKET_CODE", "TRADE_MARKET", "FIN_BALANCE_GR", "SECUCODE");

                // 逐行解析数据
                SecurityMarginTrading record;
                while (in.read_row(
                    record.DATE, record.MARKET, record.SCODE, record.SECNAME,
                    record.RZYE, record.RQYL, record.RZRQYE, record.RQYE, record.RQMCL, record.RZRQYECZ,
                    record.RZMRE, record.SZ, record.RZYEZB, record.RZMRE3D, record.RZMRE5D, record.RZMRE10D,
                    record.RZCHE, record.RZCHE3D, record.RZCHE5D, record.RZCHE10D,
                    record.RZJME, record.RZJME3D, record.RZJME5D, record.RZJME10D,
                    record.RQMCL3D, record.RQMCL5D, record.RQMCL10D,
                    record.RQCHL, record.RQCHL3D, record.RQCHL5D, record.RQCHL10D,
                    record.RQJMG, record.RQJMG3D, record.RQJMG5D, record.RQJMG10D,
                    record.SPJ, record.ZDF, record.RChange3DCP, record.RChange5DCP, record.RChange10DCP,
                    record.KCB, record.TRADE_MARKET_CODE, record.TRADE_MARKET, record.FIN_BALANCE_GR, record.SECUCODE)) {

                    // 修正证券代码格式（与Go版本完全一致）
                    record.SECUCODE = exchange::CorrectSecurityCode(record.SECUCODE);
                    tempList.emplace_back(record);
                }

            } catch (const io::error::can_not_open_file& e) {
                spdlog::error("[ERROR] 无法打开两融数据文件: {} ({})",cache_filename, e.what());
            } catch (const io::error::base& e) {
                spdlog::error( "[ERROR] CSV解析错误: {}", e.what());
            } catch (const std::exception& e) {
                spdlog::error("[ERROR] 未知错误: {}", e.what());
            }
        }

        // 6. 准备加载两融标的代码列表到内存
        std::vector<std::string> codes;
        for (const auto& v : tempList) {
            std::string securityCode = exchange::CorrectSecurityCode(v.SECUCODE);
            codes.push_back(securityCode);
        }

        if (!codes.empty()) {
            // 去重
            std::sort(codes.begin(), codes.end());
            codes.erase(std::unique(codes.begin(), codes.end()), codes.end());

            // 更新缓存
            _margin_trading_cache_list = codes;
            _margin_trading_cache_map.clear();
            for (const auto& code : _margin_trading_cache_list) {
                _margin_trading_cache_map[code] = true;
            }
        }
        spdlog::info("初始化融资融券...OK");
    }

    // 获取两融标的列表
    std::vector<std::string> MarginTradingList() {
        _margin_trading_once->Do(lazyLoadMarginTrading);
        return _margin_trading_cache_list;
    }

    // 判断是否两融标的
    bool IsMarginTradingTarget(const std::string& code) {
        _margin_trading_once->Do(lazyLoadMarginTrading);
        std::string securityCode = exchange::CorrectSecurityCode(code);
        return _margin_trading_cache_map.find(securityCode) != _margin_trading_cache_map.end();
    }

} // namespace exchange