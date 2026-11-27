// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! num 的 Zig 对等用例: 与 Python spec 的同一批口径逐条对齐。

const std = @import("std");
const num = @import("num.zig");

const allocator = std.testing.allocator;

fn almost(a: f64, b: f64) !void {
    try std.testing.expectApproxEqAbs(b, a, 1e-12);
}

test "reduce: sum / mean / population std" {
    const x = [_]f64{ 1, 2, 3, 4 };
    try std.testing.expectEqual(@as(f64, 10), num.sumValue(&x));
    try std.testing.expectEqual(@as(f64, 2.5), num.mean(&x));
    try almost(num.stddev(&x), @sqrt(@as(f64, 1.25)));
}

test "reduce: empty conventions" {
    const empty = [_]f64{};
    try std.testing.expectEqual(@as(f64, 0), num.sumValue(&empty));
    try std.testing.expect(std.math.isNan(num.mean(&empty)));
    try std.testing.expectEqual(@as(f64, 0), num.variance(&empty));
}

test "rolling: incomplete window is NaN" {
    const x = [_]f64{ 1, 2, 3, 4 };
    const window = [_]i64{ 2, 2, 2, 2 };
    const out = try num.rollingMean(allocator, &x, &window);
    defer allocator.free(out);
    try std.testing.expect(std.math.isNan(out[0]));
    try std.testing.expectEqual(@as(f64, 1.5), out[1]);
    try std.testing.expectEqual(@as(f64, 2.5), out[2]);
    try std.testing.expectEqual(@as(f64, 3.5), out[3]);
}

test "rolling: window containing NaN is NaN" {
    const x = [_]f64{ 1, std.math.nan(f64), 3 };
    const window = [_]i64{ 2, 2, 2 };
    const out = try num.rollingSum(allocator, &x, &window);
    defer allocator.free(out);
    try std.testing.expect(std.math.isNan(out[1]));
}

test "ewm: seed then recurrence" {
    const x = [_]f64{ 1, 2, 3 };
    const out = try num.ewmMean(allocator, &x, 0.3);
    defer allocator.free(out);
    try std.testing.expectEqual(@as(f64, 1), out[0]);
    try almost(out[1], 1.3);
    try almost(out[2], 1.81);
}

test "decimal: half away from zero" {
    try std.testing.expectEqual(@as(f64, 1), num.decimal(0.5, 0));
    try std.testing.expectEqual(@as(f64, -1), num.decimal(-0.5, 0));
    try std.testing.expectEqual(@as(f64, 0), num.decimal(std.math.nan(f64), 0));
}

test "shift: out of range is NaN" {
    const x = [_]f64{ 1, 2, 3 };
    const out = try num.shift(allocator, &x, 1);
    defer allocator.free(out);
    try std.testing.expect(std.math.isNan(out[0]));
    try std.testing.expectEqual(@as(f64, 1), out[1]);
    try std.testing.expectEqual(@as(f64, 2), out[2]);
}

test "where and logic" {
    const cond = [_]bool{ true, false };
    const t = [_]f64{ 1, 2 };
    const f = [_]f64{ 3, 4 };
    const out = try num.where(allocator, &cond, &t, &f);
    defer allocator.free(out);
    try std.testing.expectEqual(@as(f64, 1), out[0]);
    try std.testing.expectEqual(@as(f64, 4), out[1]);
    try std.testing.expect(num.allOf(&[_]bool{ true, true }));
    try std.testing.expect(!num.allOf(&[_]bool{ true, false }));
    try std.testing.expectEqual(@as(usize, 2), num.countTrue(&[_]bool{ true, false, true }));
}

test "regression and forcast" {
    const x = [_]f64{ 1, 2, 3, 4, 5 };
    const result = num.linearRegression(&x);
    try almost(result.slope, 1.0);
    try almost(result.intercept, 1.0);
    try almost(num.forcast(&x), 5.0);
}

test "align and linspace" {
    const x = [_]f64{ 1, 2 };
    const out = try num.padTo(allocator, &x, 4, std.math.nan(f64));
    defer allocator.free(out);
    try std.testing.expectEqual(@as(f64, 1), out[0]);
    try std.testing.expect(std.math.isNan(out[3]));

    const grid = try num.linspace(allocator, 0, 1, 5);
    defer allocator.free(grid);
    try almost(grid[0], 0.0);
    try almost(grid[4], 1.0);
    try almost(grid[2], 0.5);
}
