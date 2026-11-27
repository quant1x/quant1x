// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

/// NdArray 基本语义: 与 ndarray.py / ndarray.rs 的同一批口径对齐。

#include <quant1x/test/test.h>

#include <quant1x/num/ndarray.h>

#include <cmath>
#include <vector>

namespace {

using quant1x::num::NdArray;

NdArray make_array(std::initializer_list<double> values) {
    return NdArray(std::vector<double>(values));
}

bool approx(double actual, double expected) { return std::abs(actual - expected) < 1e-12; }

}  // namespace

TEST_CASE("ndarray-construct", "[num][ndarray]") {
    const NdArray a = make_array({1.0, 2.0, 3.0});
    REQUIRE(a.size() == 3);
    REQUIRE(a.ndim() == 1);
    REQUIRE(std::string(a.dtype()) == "float64");
    REQUIRE(a.to_vector() == std::vector<double>{1.0, 2.0, 3.0});

    REQUIRE(NdArray::zeros(2).to_vector() == std::vector<double>{0.0, 0.0});
    REQUIRE(NdArray::ones(2).to_vector() == std::vector<double>{1.0, 1.0});
    REQUIRE(NdArray::full(2, 5.0).to_vector() == std::vector<double>{5.0, 5.0});
    REQUIRE(NdArray::arange(0.0, 3.0, 1.0).to_vector() == std::vector<double>{0.0, 1.0, 2.0});
    REQUIRE(NdArray::linspace(0.0, 1.0, 3).to_vector() == std::vector<double>{0.0, 0.5, 1.0});
}

TEST_CASE("ndarray-arithmetic", "[num][ndarray]") {
    const NdArray a = make_array({1.0, 2.0, 3.0});
    const NdArray b = make_array({10.0, 20.0, 30.0});
    REQUIRE(a.add(b).to_vector() == std::vector<double>{11.0, 22.0, 33.0});
    REQUIRE(a.sub(b).to_vector() == std::vector<double>{-9.0, -18.0, -27.0});
    REQUIRE(a.mul(b).to_vector() == std::vector<double>{10.0, 40.0, 90.0});
    REQUIRE(b.div(a).to_vector() == std::vector<double>{10.0, 10.0, 10.0});
    REQUIRE(a.add_scalar(1.0).to_vector() == std::vector<double>{2.0, 3.0, 4.0});
    REQUIRE(make_array({4.0, 9.0}).sqrt().to_vector() == std::vector<double>{2.0, 3.0});
    REQUIRE(make_array({-1.0, 2.0}).abs().to_vector() == std::vector<double>{1.0, 2.0});
}

TEST_CASE("ndarray-compare-logic", "[num][ndarray]") {
    const NdArray a = make_array({1.0, 2.0, 3.0});
    REQUIRE(a.gt_scalar(2.0).to_bools() == std::vector<bool>{false, false, true});
    REQUIRE(a.equal_scalar(2.0).to_bools() == std::vector<bool>{false, true, false});

    const NdArray x = NdArray(std::vector<bool>{true, true, false, false});
    const NdArray y = NdArray(std::vector<bool>{true, false, true, false});
    REQUIRE(x.logic_and(y).to_bools() == std::vector<bool>{true, false, false, false});
    REQUIRE(x.logic_or(y).to_bools() == std::vector<bool>{true, true, true, false});
    REQUIRE(x.logic_not().to_bools() == std::vector<bool>{false, false, true, true});
}

TEST_CASE("ndarray-reductions", "[num][ndarray]") {
    const NdArray a = make_array({1.0, 2.0, 3.0, 4.0, 5.0});
    REQUIRE(approx(a.sum(), 15.0));
    REQUIRE(approx(a.mean(), 3.0));
    REQUIRE(approx(a.median(), 3.0));
    REQUIRE(approx(a.min_value(), 1.0));
    REQUIRE(approx(a.max_value(), 5.0));
    REQUIRE(a.argmin() == 0);
    REQUIRE(a.argmax() == 4);
    REQUIRE(approx(a.variance(), 2.0));
    REQUIRE(approx(a.stddev(), std::sqrt(2.0)));
    REQUIRE(make_array({1.0, 2.0, 3.0}).cumsum().to_vector() == std::vector<double>{1.0, 3.0, 6.0});
}

TEST_CASE("ndarray-windows", "[num][ndarray]") {
    const NdArray a = make_array({1.0, 2.0, 3.0, 4.0});
    const NdArray shifted = a.shift(1);
    REQUIRE(std::isnan(shifted.get_f64(0)));
    REQUIRE(shifted.get_f64(1) == 1.0);

    const NdArray diff = a.diff(1);
    REQUIRE(std::isnan(diff.get_f64(0)));
    REQUIRE(diff.get_f64(1) == 1.0);

    const std::vector<double> mean = a.rolling_mean(2).to_vector();
    REQUIRE(std::isnan(mean[0]));
    REQUIRE(approx(mean[1], 1.5));
    REQUIRE(approx(mean[2], 2.5));
    REQUIRE(approx(mean[3], 3.5));

    const std::vector<double> ewm = make_array({1.0, 2.0, 3.0}).ewm_mean(0.3).to_vector();
    REQUIRE(approx(ewm[0], 1.0));
    REQUIRE(approx(ewm[1], 1.3));
}

TEST_CASE("ndarray-convert", "[num][ndarray]") {
    const NdArray a = make_array({1.0, 0.0, 2.0});
    const NdArray mask = a.astype("bool");
    REQUIRE(std::string(mask.dtype()) == "bool");
    REQUIRE(mask.to_bools() == std::vector<bool>{true, false, true});
    REQUIRE(mask.astype("float64").to_vector() == std::vector<double>{1.0, 0.0, 1.0});
    REQUIRE(a.slice(0, 2).to_vector() == std::vector<double>{1.0, 0.0});
}
