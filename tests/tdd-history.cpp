#include <quant1x/test/test.h>
#include <quant1x/factors/history.h>
#include <quant1x/contrib/data/tdx/bar.h>
#include <quant1x/dataframe/dataframe.h>
#include <quant1x/formula.h>
#include <quant1x/datasets/trans.h>

// 测试获取指定日期的历史成交数据
TEST_CASE("history-trans", "[features]") {
    std::string code = "600600";
    std::string date = "2025-05-29";
    meta::Timestamp ts = meta::Timestamp::parse(date);
    auto list = datasets::CheckoutTransactionData(code, ts, true);
    auto summary = datasets::CountInflow(list, code, ts);
    std::cout << summary << std::endl;
}

TEST_CASE("history-basic-base", "[features]") {
    //using namespace formula;
    std::string code = "600600";
    std::string date = "2025-05-29";
    auto klines = tdx::checkout_klines(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
    // 直接获取列
    const auto& col_close = df["close"];
    // 使用std::get获取具体vector
    const auto& close_ = std::get<std::vector<double>>(col_close);
    auto CLOSE = xt::adapt(close_);
    auto ref1 = formula::ref(CLOSE,1);
    for (const auto& price : ref1) {
        std::cout << price << " ";
    }
    std::cout << std::endl;
    auto last = formula::at(ref1, - 1);
    std::cout << "last:" << last << std::endl;
}

TEST_CASE("history-basic-auto", "[features]") {
    //using namespace formula;
    std::string code = "600600";
    std::string date = "2025-05-29";
    auto klines = tdx::checkout_klines(code, date);
    std::cout << klines.size() << std::endl;
    DataFrame df = DataFrame::from_struct_vector(klines);
    std::cout << df.to_string() << std::endl;
    // 直接获取列
    auto const& col_open = df.get<f64>("open");
    const xt::xarray<f64>& OPEN = xt::adapt(col_open);
    auto const& col_close = df.get<f64>("close");
    const xt::xarray<f64>& CLOSE = xt::adapt(col_close);
    auto const& col_high = df.get<f64>("high");
    const xt::xarray<f64>& HIGH = xt::adapt(col_high);
    auto const& col_low = df.get<f64>("low");
    const xt::xarray<f64>& LOW = xt::adapt(col_low);

    auto const& col_vol = df.get<f64>("volume");
    const xt::xarray<f64>& VOL = xt::adapt(col_vol);
    auto const& col_amount = df.get<f64>("amount");
    const xt::xarray<f64>& AMOUNT = xt::adapt(col_amount);

    auto r1Close = formula::ref(CLOSE,1);
    for (const auto& price : r1Close) {
        std::cout << price << " ";
    }
    std::cout << std::endl;
    History info{};
    auto ma2 = formula::ma(CLOSE, 2);
    info.MA2 = formula::at(ma2, -1);

    auto ma3 = formula::ma(CLOSE, 3);
    info.MA3 = formula::at(ma3, -1);
    auto mv3 = formula::ma(VOL, 3);
    info.MV3 = formula::at(mv3, -1);

    auto ma4 = formula::ma(CLOSE, 4);
    info.MA4 = formula::at(ma4, -1);

    auto ma5 = formula::ma(CLOSE, 5);
    info.MA5 = formula::at(ma5, -1);
    auto mv5 = formula::ma(VOL, 5);
    info.MV5 = formula::at(mv5, -1);

    auto ma9 = formula::ma(CLOSE, 9);
    info.MA9 = formula::at(ma9, -1);
    auto mv9 = formula::ma(VOL, 9);
    info.MV9 = formula::at(mv9, -1);

    auto ma10 = formula::ma(CLOSE, 10);
    info.MA10 = formula::at(ma10, -1);
    auto mv10 = formula::ma(VOL, 10);
    info.MV10 = formula::at(mv10, -1);

    auto ma19 = formula::ma(CLOSE, 19);
    info.MA19 = formula::at(ma19, -1);
    auto mv19 = formula::ma(VOL, 19);
    info.MV19 = formula::at(mv19, -1);

    auto ma20 = formula::ma(CLOSE, 20);
    info.MA20 = formula::at(ma20, -1);
    auto mv20 = formula::ma(VOL, 20);
    info.MV20 = formula::at(mv20, -1);

    info.LastClose = formula::at(r1Close, -1);
    info.OPEN = formula::at(OPEN, -1);
    info.CLOSE = formula::at(CLOSE, -1);
    info.HIGH = formula::at(HIGH, -1);
    info.LOW = formula::at(LOW, -1);
    info.VOL = formula::at(VOL, -1);
    info.AMOUNT = formula::at(AMOUNT, -1);

    const xt::xarray<f64>& ap = AMOUNT / VOL;
    info.AveragePrice = formula::at(ap, -1);

    // 计算多头排列: 5日线高于10日线, 10日线高于20日线
    auto bullC = ma5>ma10 && ma10>ma20;
    auto bullN = formula::bars_last_count(bullC);
    info.BullN = formula::at(bullN, -1);

    // 最近一次向上的跳空缺口到现在的周期数
    auto grapUpWard = LOW > formula::ref(HIGH, 1);
    auto upward_n = formula::bars_last(grapUpWard);
    info.UpwardN = formula::at(upward_n, -1);

    // 收盘价,最高价和成交量连续走高
    auto isClosingPriceStrong = CLOSE>r1Close;
    auto isVolStrong = VOL>formula::ref(VOL,1);
    auto newHigh = isClosingPriceStrong && HIGH>formula::ref(HIGH,1) && isVolStrong;
    auto newHighN = formula::bars_last_count(newHigh);
    info.NewHighN = formula::at(newHighN, -1);

    // 低点走高次数
    auto noLow = isClosingPriceStrong && isVolStrong;
    auto noLowN = formula::bars_last_count(noLow);
    info.NewNoLowN = formula::at(noLowN, -1);

    // 最低价连续走低
    auto newLow = LOW < formula::ref(LOW, 1);
    auto newLowN = formula::bars_last_count(newLow);
    info.NewLowN = formula::at(newLowN, -1);

    // 成交统计概要数据
    meta::Timestamp ts = meta::Timestamp::parse(date);
    auto list = datasets::CheckoutTransactionData(code, ts, true);
    auto summary = datasets::CountInflow(list, code, ts);
    info.OpenVolume = summary.OpenVolume;

    info.UpdateTime = api::get_timestamp();
    info.State |= factors::FeatureHistory;

    std::cout << "history: " << info << std::endl;
}

TEST_CASE("history-release", "[factors]") {
    std::string code = "sh600600";
    std::string date = "2025-06-24";
    HistoryFeature adapter;
    meta::Timestamp feature_date = meta::Timestamp(date);
    auto inst = data::detect_symbol(code);
    adapter.Update(inst, feature_date);
}