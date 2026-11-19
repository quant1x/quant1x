// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! globmatch 的对等用例: 与 Python / C++ / Rust / Go 同一批样本逐条对齐。

const std = @import("std");
const gm = @import("globmatch.zig");

fn expectMatch(path: []const u8, pattern: []const u8, want: bool) !void {
    try std.testing.expectEqual(want, gm.matches(path, pattern));
}

test "globstar zero or more segments" {
    try expectMatch("stb.h", "**/*.h", true);
    try expectMatch("a/b/c.h", "**/*.h", true);
    try expectMatch("a/c.h", "a/**/c.h", true);
    try expectMatch("a/x/y/c.h", "a/**/c.h", true);
    try expectMatch("third_party/stb.h", "third_party/**", true);
    try expectMatch("third_party/a/b.h", "third_party/**", true);
}

test "anchored" {
    try expectMatch("libs/stb.h", "libs/*.h", true);
    try expectMatch("libs/sub/stb.h", "libs/*.h", false);
    try expectMatch("x/libs/stb.h", "libs/*.h", false);
    try expectMatch("x/third_party/a.h", "third_party/**", false);
    try expectMatch("third_partyish/x.h", "third_party/**", false);
    try expectMatch("b/a/c.h", "a/**/c.h", false);
}

test "single segment wildcards" {
    try expectMatch("a.h", "*.h", true);
    try expectMatch("a.c", "*.h", false);
    try expectMatch("a1.h", "a?.h", true);
    try expectMatch("a1.h", "a[0-9].h", true);
    try expectMatch("a.h", "a[0-9].h", false);
    try expectMatch("a5.h", "a[!0-9].h", false);
    try expectMatch("ax.h", "a[!0-9].h", true);
    try expectMatch("a-.h", "a[a-].h", true);
    try expectMatch("az.h", "a[a-].h", false);
    try expectMatch("a[b.h", "a[b.h", true);
}

test "root absolute form" {
    try expectMatch("/third_party/a.h", "third_party/**", true);
    try expectMatch("third_party/a.h", "/third_party/**", true);
    try expectMatch("/third_party/a.h", "/third_party/**", true);
    try expectMatch("/x/third_party/a.h", "/third_party/**", false);
    try expectMatch("/a/b.h", "/**/*.h", true);
}

test "dotfile and case" {
    try expectMatch(".hidden.h", "*.h", true);
    try expectMatch("A.H", "*.h", false);
}

test "trailing globstar needs one segment" {
    try expectMatch("a", "a/**", false);
    try expectMatch("a/b", "a/**", true);
    try expectMatch("a/b/c", "a/**", true);
    try expectMatch("a", "**", true);
}

test "empty inputs" {
    try expectMatch("", "**", false);
    try expectMatch("a.h", "", false);
    try std.testing.expect(!gm.matchesAny("a.h", &.{}));
}

test "matches any" {
    try std.testing.expect(gm.matchesAny("libs/a.h", &.{ "third_party/**", "libs/*.h" }));
    try std.testing.expect(!gm.matchesAny("libs/a.h", &.{ "third_party/**", "**.c" }));
}
