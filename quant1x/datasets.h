#pragma once
#ifndef QUANT1X_DATASETS_CACHE_H
#define QUANT1X_DATASETS_CACHE_H 1

#include <quant1x/std/api.h>
#include <quant1x/data.h>

namespace datasets {
    // Deprecated shim: forward to `data` namespace. Use `data::init()` instead.
    inline void init() { return data::init(); }
}

#endif //QUANT1X_DATASETS_CACHE_H
