// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/test/test.h>

#include <quant1x/pandas/series.h>
#include <quant1x/pandas/ewm.h>

#ifdef _WIN32
#include <windows.h>
#endif

TEST_CASE("ema-basic", "[dataframe]") {
    std::vector<double> values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    quant1x::pandas::Series s1(values);
    std::cout << s1.size() << std::endl;

    int N = 7;
    quant1x::pandas::ExponentialWeighting param;
    param.span = static_cast<double>(N);
    param.adjust = false;
    const auto r1 = quant1x::pandas::ewm(s1, param);

    std::cout << "values:";
    for (auto const &v : values) {
        std::cout << v << ",";
    }
    std::cout << std::endl;
    std::cout << "r1:";
    for (auto const &v : r1.values()) {
        std::cout << v << ",";
    }
    std::cout << std::endl;
}
