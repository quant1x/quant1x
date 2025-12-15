#include <quant1x/datasets/minute.h>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>
#include <quant1x/std/filepath.h>

namespace datasets {

    cache::Kind DataMinute::Kind() const {
        return BaseMinutes;
    }

    std::string DataMinute::Owner() {
        return cache::DefaultDataProvider;
    }

    std::string DataMinute::Key() const {
        return "minutes";
    }

    std::string DataMinute::Name() const {
        return "分时数据";
    }

    std::string DataMinute::Usage() const {
        return "分时数据";
    }

    void DataMinute::Print(const std::string &code, const std::vector<exchange::timestamp> &dates) {
        (void) code;
        (void) dates;
    }

    void save_minutes(const std::string &code, const exchange::timestamp &date,
                      const std::vector<level1::MinuteTime> &values) {
        std::string securityCode = exchange::CorrectSecurityCode(code);
        exchange::timestamp factor_date = date;
        (void) factor_date;

        auto ofn = config::get_minute_filename(securityCode, date.toString(config::cache_filename_date_layout));
        filepath::check_filepath(ofn, true);
        //std::cout << "write = " << ofn << std::endl;
        io::CSVWriter writer(ofn);
        writer.write_row("Price", "Vol");
        for (const auto &v: values) {
            writer.write_row(v.Price, v.Vol);
        }
    }

    void DataMinute::Update(const std::string &code, const exchange::timestamp &date) {
        try {
            auto conn = level1::get_std_conn();
            auto [id, _, symbol] = exchange::DetectMarket(code);
            level1::HistoryMinuteTimeRequest request(code, date.yyyymmdd());
            level1::HistoryMinuteTimeResponse response(static_cast<int>(id), symbol.c_str());
            auto err = level1::process(conn->socket(), request, response);
            if (err) {
                throw std::runtime_error(fmt::format("Process error: {}", err.message()));
            }
            if (response.Count) {
                save_minutes(code, date, response.List);
            }
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[dataset::minute] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[dataset::minute] Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[dataset::minute] 获取除权除息异常");
        }
    }
}