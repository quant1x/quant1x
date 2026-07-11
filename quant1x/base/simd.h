#pragma once
#ifndef QUANT1X_BASE_SIMD_H
#define QUANT1X_BASE_SIMD_H 1

#define XTENSOR_USE_XSIMD
#include <xtensor/containers/xadapt.hpp>
#include <xtensor/containers/xarray.hpp>
#include <xtensor/core/xeval.hpp>
#include <xtensor/core/xoperation.hpp> // 需包含 xtensor 运算支持
#include <xtensor/generators/xrandom.hpp>
#include <xtensor/core/xmath.hpp>
#include <xtensor/misc/xpad.hpp>
#include <xtensor/misc/xsort.hpp>
#include <xtensor/io/xio.hpp>
#include <xtensor/views/xview.hpp>
#include <quant1x/ta/type_default.h>
#include <vector>
#include <cmath>
#include <span>
#include <type_traits> // 用于类型检查

namespace simd {
    template<typename T>
    using array = xt::xarray<T>;
} // namespace simd

#endif //QUANT1X_BASE_SIMD_H
