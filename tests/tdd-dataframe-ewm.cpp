#include <quant1x/test/test.h>

#include <quant1x/pandas/series.h>
#include <quant1x/pandas/ewm.h>

#ifdef _WIN32
#include <windows.h>
#endif



TEST_CASE("ema-basic", "[dataframe]") {
    std::vector<f64> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    ta::Series<f64> s1(values);
    std::cout << s1.size()<< std::endl;
    int N = 7;
//    EW param = {.span = 7, .adjust = false, .callback = [&](int idx) -> double {
//        (void)idx;
//        int j = N;
//        if(j == 0) {
//            j = 1;
//        }
//        return 2.0 / (j + 1);
//    }};
    //ExponentialMovingWindow<f64> w0 = s1.ewm(span=7.0, adjust=false);
    //(void)w0;
    ExponentialMovingWindow<f64> w1 = s1.ewm(ExponentialWeighting{.span=static_cast<double>(N), .adjust=false});
    auto r1 = w1.mean();

    std::cout << "values:";
    for(auto const & v : values) {
        std::cout << v << ",";
    }
    std::cout << std::endl;
    std::cout << "r1:";
    for(auto const & v : r1.data()) {
        std::cout << v << ",";
    }
    std::cout << std::endl;
    //std::cout << "s2:" << s2.mean() << std::endl;
}