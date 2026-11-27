// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! ndarray 的 Zig 对等用例: 与 ndarray.py / ndarray.h / ndarray.rs 同一批口径。

const std = @import("std");
const nd = @import("ndarray.zig");

const allocator = std.testing.allocator;

fn approx(actual: f64, expected: f64) !void {
    try std.testing.expectApproxEqAbs(expected, actual, 1e-12);
}

test "construct and props" {
    const values = [_]f64{ 1, 2, 3 };
    var a = try nd.NdArray.initFloat(allocator, &values);
    defer a.deinit();
    try std.testing.expectEqual(@as(usize, 3), a.len());
    try std.testing.expect(!a.isBool());
    try std.testing.expectEqual(nd.DType.float64, a.dtype());

    var z = try nd.NdArray.zeros(allocator, 2);
    defer z.deinit();
    try std.testing.expectEqual(@as(f64, 0), z.getF64(0));

    var grid = try nd.NdArray.linspace(allocator, 0, 1, 3);
    defer grid.deinit();
    try approx(grid.getF64(0), 0);
    try approx(grid.getF64(1), 0.5);
    try approx(grid.getF64(2), 1);
}

test "arithmetic" {
    const x = [_]f64{ 1, 2, 3 };
    const y = [_]f64{ 10, 20, 30 };
    var a = try nd.NdArray.initFloat(allocator, &x);
    defer a.deinit();
    var b = try nd.NdArray.initFloat(allocator, &y);
    defer b.deinit();

    var sum = try a.add(&b);
    defer sum.deinit();
    try std.testing.expectEqual(@as(f64, 11), sum.getF64(0));
    try std.testing.expectEqual(@as(f64, 33), sum.getF64(2));

    var plus = try a.addScalar(1);
    defer plus.deinit();
    try std.testing.expectEqual(@as(f64, 2), plus.getF64(0));

    var root = try nd.NdArray.initFloat(allocator, &[_]f64{ 4, 9 });
    defer root.deinit();
    var r = try root.sqrt();
    defer r.deinit();
    try std.testing.expectEqual(@as(f64, 2), r.getF64(0));
}

test "compare and logic" {
    const x = [_]f64{ 1, 2, 3 };
    var a = try nd.NdArray.initFloat(allocator, &x);
    defer a.deinit();
    var mask = try a.gtScalar(2);
    defer mask.deinit();
    try std.testing.expect(mask.isBool());
    try std.testing.expectEqual(false, mask.getBool(0));
    try std.testing.expectEqual(true, mask.getBool(2));

    var p = try nd.NdArray.initBool(allocator, &[_]bool{ true, true, false, false });
    defer p.deinit();
    var q = try nd.NdArray.initBool(allocator, &[_]bool{ true, false, true, false });
    defer q.deinit();
    var anded = try p.logicAnd(&q);
    defer anded.deinit();
    try std.testing.expectEqual(false, anded.getBool(1));
    var noted = try p.logicNot();
    defer noted.deinit();
    try std.testing.expectEqual(false, noted.getBool(0));
}

test "reductions" {
    const x = [_]f64{ 1, 2, 3, 4, 5 };
    var a = try nd.NdArray.initFloat(allocator, &x);
    defer a.deinit();
    try approx(a.sum(), 15);
    try approx(a.mean(), 3);
    try approx(try a.median(), 3);
    try approx(a.minValue(), 1);
    try approx(a.maxValue(), 5);
    try std.testing.expectEqual(@as(i64, 0), a.argmin());
    try std.testing.expectEqual(@as(i64, 4), a.argmax());
    try approx(a.variance(), 2);
    try approx(a.stddev(), @sqrt(@as(f64, 2)));
}

test "windows" {
    const x = [_]f64{ 1, 2, 3, 4 };
    var a = try nd.NdArray.initFloat(allocator, &x);
    defer a.deinit();

    var shifted = try a.shift(1);
    defer shifted.deinit();
    try std.testing.expect(std.math.isNan(shifted.getF64(0)));
    try std.testing.expectEqual(@as(f64, 1), shifted.getF64(1));

    var mean = try a.rollingMean(2);
    defer mean.deinit();
    try std.testing.expect(std.math.isNan(mean.getF64(0)));
    try approx(mean.getF64(1), 1.5);
    try approx(mean.getF64(3), 3.5);

    var ewm = try nd.NdArray.initFloat(allocator, &[_]f64{ 1, 2, 3 });
    defer ewm.deinit();
    var e = try ewm.ewmMean(0.3);
    defer e.deinit();
    try approx(e.getF64(0), 1);
    try approx(e.getF64(1), 1.3);
}
