// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/test/test.h>
#include <quant1x/base/filesystem.h>

TEST_CASE("homedir", "[io]") {
    auto homedir = filesystem::homedir();
    std::cout << homedir << std::endl;
}