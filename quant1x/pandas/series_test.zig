// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! Series 的 Zig 对等用例: 与 series.py / series.h / series.rs 同一批口径。

const std = @import("std");
const ser = @import("series.zig");

const allocator = std.testing.allocator;

fn approx(actual: f64, expected: f64) !void {
    try std.testing.expectApproxEqAbs(expected, actual, 1e-12);
}

test "construct and props" {
    var s = try ser.Series.initFloat(allocator, &[_]f64{ 1, 2, 3 });
    defer s.deinit();
    try std.testing.expectEqual(@as(usize, 3), s.len());
    try std.testing.expect(!s.isBool());
    try s.setName("close");
    try std.testing.expectEqualStrings("close", s.nameOf());
}

test "arithmetic and compare" {
    var a = try ser.Series.initFloat(allocator, &[_]f64{ 1, 2, 3 });
    defer a.deinit();
    var b = try ser.Series.initFloat(allocator, &[_]f64{ 10, 20, 30 });
    defer b.deinit();

    var sum = try a.add(&b);
    defer sum.deinit();
    try std.testing.expectEqual(@as(f64, 11), sum.getF64(0));
    try std.testing.expectEqual(@as(f64, 33), sum.getF64(2));

    var plus = try a.addScalar(1);
    defer plus.deinit();
    try std.testing.expectEqual(@as(f64, 2), plus.getF64(0));

    var mask = try a.gtScalar(2);
    defer mask.deinit();
    try std.testing.expectEqual(false, mask.getBool(0));
    try std.testing.expectEqual(true, mask.getBool(2));
}

test "logic" {
    var p = try ser.Series.initBool(allocator, &[_]bool{ true, true, false, false });
    defer p.deinit();
    var q = try ser.Series.initBool(allocator, &[_]bool{ true, false, true, false });
    defer q.deinit();

    var anded = try p.logicAnd(&q);
    defer anded.deinit();
    try std.testing.expectEqual(false, anded.getBool(1));

    var ored = try p.logicOr(&q);
    defer ored.deinit();
    try std.testing.expectEqual(true, ored.getBool(2));

    var noted = try p.logicNot();
    defer noted.deinit();
    try std.testing.expectEqual(false, noted.getBool(0));
}

test "reductions" {
    var s = try ser.Series.initFloat(allocator, &[_]f64{ 1, 2, 3, 4, 5 });
    defer s.deinit();
    try approx(s.sum(), 15);
    try approx(s.mean(), 3);
    try approx(try s.median(), 3);
    try approx(s.minValue(), 1);
    try approx(s.maxValue(), 5);
    try std.testing.expectEqual(@as(i64, 0), s.argmin());
    try std.testing.expectEqual(@as(i64, 4), s.argmax());
    try approx(s.variance(), 2);
}

test "windows" {
    var s = try ser.Series.initFloat(allocator, &[_]f64{ 1, 2, 3, 4 });
    defer s.deinit();

    var shifted = try s.shift(1);
    defer shifted.deinit();
    try std.testing.expect(std.math.isNan(shifted.getF64(0)));
    try std.testing.expectEqual(@as(f64, 1), shifted.getF64(1));

    var mean = try s.rolling(2).mean();
    defer mean.deinit();
    try std.testing.expect(std.math.isNan(mean.getF64(0)));
    try approx(mean.getF64(1), 1.5);
    try approx(mean.getF64(3), 3.5);

    var ew = try s.ewm(0.5).mean();
    defer ew.deinit();
    try approx(ew.getF64(0), 1);
    try approx(ew.getF64(1), 1.5);
}

test "fill and slice" {
    var s = try ser.Series.initFloat(allocator, &[_]f64{ 1, std.math.nan(f64), 3 });
    defer s.deinit();
    var filled = try s.fillNa(0);
    defer filled.deinit();
    try std.testing.expectEqual(@as(f64, 0), filled.getF64(1));

    var part = try s.slice(0, 1);
    defer part.deinit();
    try std.testing.expectEqual(@as(usize, 1), part.len());
    try std.testing.expectEqual(@as(f64, 1), part.getF64(0));
}
