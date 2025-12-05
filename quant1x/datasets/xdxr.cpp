#include <quant1x/level1/client.h>
#include <quant1x/datasets/xdxr.h>
#include <quant1x/config/base.h>
#include <quant1x/config/cache.h>

namespace datasets {

    std::vector<level1::XdxrInfo> load_xdxr(const std::string& code) {
        std::vector<level1::XdxrInfo> result;
        try {
            auto cache_filename = config::get_xdxr_filename(code);
            io::CSVReader<14> in(cache_filename);
            in.read_header(io::ignore_extra_column, "Date", "Category", "Name", "FenHong", "PeiGuJia", "SongZhuanGu",
                           "PeiGu", "SuoGu", "QianLiuTong", "HouLiuTong", "QianZongGuBen", "HouZongGuBen", "FenShu",
                           "XingQuanJia");
            level1::XdxrInfo row = {};
            while (in.read_row(row.Date, row.Category, row.Name, row.FenHong, row.PeiGuJia, row.SongZhuanGu, row.PeiGu, row.SuoGu,
                               row.QianLiuTong, row.HouLiuTong, row.QianZongGuBen, row.HouZongGuBen, row.FenShu, row.XingQuanJia)) {
                result.emplace_back(row);

            }
        } catch(...) {
            //
        }
        return result;
    }

    void save_xdxr(const std::string &code, const exchange::timestamp &date, const std::vector<level1::XdxrInfo>& values) {
        std::string securityCode = exchange::CorrectSecurityCode(code);
        exchange::timestamp factor_date = date;
        (void)factor_date;

        auto ofn = config::get_xdxr_filename(securityCode);
        util::check_filepath(ofn, true);
        io::CSVWriter writer(ofn);
        writer.write_row("Date","Category","Name","FenHong","PeiGuJia","SongZhuanGu","PeiGu","SuoGu","QianLiuTong","HouLiuTong","QianZongGuBen","HouZongGuBen","FenShu","XingQuanJia");
        for (const auto &v: values) {
            writer.write_row(v.Date, v.Category, v.Name, v.FenHong,v.PeiGuJia,v.SongZhuanGu,v.PeiGu,v.SuoGu,v.QianLiuTong,v.HouLiuTong,v.QianZongGuBen,v.HouZongGuBen,v.FenShu,v.XingQuanJia);
        }
    }

    cache::Kind DataXdxr::Kind() const  {
        return BaseXdxr;
    }

    std::string DataXdxr::Owner()  {
        return cache::DefaultDataProvider;
    }

    std::string DataXdxr::Key() const  {
        return "xdxr";
    }

    std::string DataXdxr::Name() const  {
        return "除权除息";
    }

    std::string DataXdxr::Usage() const  {
        return std::string();
    }

    void DataXdxr::Print(const std::string &code, const std::vector<exchange::timestamp> &dates)  {
        (void)code;
        (void)dates;
    }

    void DataXdxr::Update(const std::string &code, const exchange::timestamp &date)  {
//        if(date != exchange::last_trading_day()) {
//            return;
//        }
        try {
            auto conn = level1::get_std_conn();
            level1::XdxrInfoRequest request(code);
            level1::XdxrInfoResponse response;
            auto err = level1::process(conn->socket(), request, response);
            if (err) {
                throw std::runtime_error(fmt::format("Process error: {}", err.message()));
            }
            if (response.Count) {
                save_xdxr(code, date, response.List);
            }
        } catch (const std::exception &e) {  // 其他标准异常
            spdlog::error("[dataset::xdxr] - 标准异常: {} (type: {})", e.what(), typeid(e).name());
            // 对于system_error可以记录更多信息
            if (auto se = dynamic_cast<const std::system_error *>(&e)) {
                spdlog::error("[dataset::xdxr] Error code: {}, category: {}", se->code().value(), se->code().category().name());
            }
        } catch (...) {
            spdlog::error("[dataset::xdxr] 获取除权除息异常");
        }
    }

}