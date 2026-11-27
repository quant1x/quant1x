// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

// 多进度条冒烟：quant1x/mpb 的 MultiProgress + ProgressBar 基本用法。

#include <quant1x/base/api.h>

#include <mpb.h>

#include <fmt/format.h>

int main() {
    auto multi = mpb::MultiProgress::default_instance();
    auto style = mpb::progress_style_with_template(
        "[{elapsed_precise}] {bar:40.cyan/blue} {pos:>7}/{len:7} {msg}");

    const int64_t count = 10000;
    auto bar = multi->add(mpb::ProgressBar::make(count));
    bar->set_style(style);
    bar->set_message("multibar");

    for (int64_t i = 1; i <= count; ++i) {
        bar->set_message(fmt::format("[{}/{}]", i, count));
        bar->inc();
    }
    bar->finish();
    return 0;
}
