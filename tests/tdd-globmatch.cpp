// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/base/globmatch.hpp>
#include <quant1x/test/test.h>

using globmatch::matches;
using globmatch::matches_any;

// 与 Python / Rust / Go / Zig 同一批样本逐条对齐 (quant1x/base/globmatch_test.py)

TEST_CASE("globmatch globstar zero or more segments", "[globmatch]") {
    CHECK(matches("stb.h", "**/*.h"));
    CHECK(matches("a/b/c.h", "**/*.h"));
    CHECK(matches("a/c.h", "a/**/c.h"));
    CHECK(matches("a/x/y/c.h", "a/**/c.h"));
    CHECK(matches("third_party/stb.h", "third_party/**"));
    CHECK(matches("third_party/a/b.h", "third_party/**"));
}

TEST_CASE("globmatch anchored", "[globmatch]") {
    CHECK(matches("libs/stb.h", "libs/*.h"));
    CHECK_FALSE(matches("libs/sub/stb.h", "libs/*.h"));
    CHECK_FALSE(matches("x/libs/stb.h", "libs/*.h"));
    CHECK_FALSE(matches("x/third_party/a.h", "third_party/**"));
    CHECK_FALSE(matches("third_partyish/x.h", "third_party/**"));
    CHECK_FALSE(matches("b/a/c.h", "a/**/c.h"));
}

TEST_CASE("globmatch single segment wildcards", "[globmatch]") {
    CHECK(matches("a.h", "*.h"));
    CHECK_FALSE(matches("a.c", "*.h"));
    CHECK(matches("a1.h", "a?.h"));
    CHECK(matches("a1.h", "a[0-9].h"));
    CHECK_FALSE(matches("a.h", "a[0-9].h"));
    CHECK_FALSE(matches("a5.h", "a[!0-9].h"));
    CHECK(matches("ax.h", "a[!0-9].h"));
    CHECK(matches("a-.h", "a[a-].h"));
    CHECK_FALSE(matches("az.h", "a[a-].h"));
    CHECK(matches("a[b.h", "a[b.h"));
}

TEST_CASE("globmatch root absolute form", "[globmatch]") {
    CHECK(matches("/third_party/a.h", "third_party/**"));
    CHECK(matches("third_party/a.h", "/third_party/**"));
    CHECK(matches("/third_party/a.h", "/third_party/**"));
    CHECK_FALSE(matches("/x/third_party/a.h", "/third_party/**"));
    CHECK(matches("/a/b.h", "/**/*.h"));
}

TEST_CASE("globmatch dotfile and case", "[globmatch]") {
    CHECK(matches(".hidden.h", "*.h"));
    CHECK_FALSE(matches("A.H", "*.h"));
}

TEST_CASE("globmatch trailing globstar needs one segment", "[globmatch]") {
    CHECK_FALSE(matches("a", "a/**"));
    CHECK(matches("a/b", "a/**"));
    CHECK(matches("a/b/c", "a/**"));
    CHECK(matches("a", "**"));
}

TEST_CASE("globmatch empty inputs", "[globmatch]") {
    CHECK_FALSE(matches("", "**"));
    CHECK_FALSE(matches("a.h", ""));
    CHECK_FALSE(matches_any("a.h", {}));
}

TEST_CASE("globmatch matches any", "[globmatch]") {
    const std::vector<std::string_view> hit{"third_party/**", "libs/*.h"};
    const std::vector<std::string_view> miss{"third_party/**", "**.c"};
    CHECK(matches_any("libs/a.h", hit));
    CHECK_FALSE(matches_any("libs/a.h", miss));
}
