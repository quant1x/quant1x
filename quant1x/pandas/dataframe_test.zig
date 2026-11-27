// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! DataFrame 契约用例: 列序、取列、子集、写入与列长校验。

const std = @import("std");
const df = @import("dataframe.zig");
const ser = @import("series.zig");

const DataFrame = df.DataFrame;
const Series = ser.Series;

fn approx(actual: f64, expected: f64) !void {
    try std.testing.expect(@abs(actual - expected) <= 1e-12);
}

fn sample(allocator: std.mem.Allocator) !DataFrame {
    var frame = DataFrame.init(allocator);
    errdefer frame.deinit();
    try frame.set("open", try Series.initFloat(allocator, &.{ 1, 2, 3 }));
    try frame.set("close", try Series.initFloat(allocator, &.{ 1.5, 2.5, 3.5 }));
    return frame;
}

test "DataFrame 列序与形状" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    try std.testing.expectEqual(@as(usize, 2), frame.columnCount());
    try std.testing.expectEqualStrings("open", frame.columnName(0).?);
    try std.testing.expectEqualStrings("close", frame.columnName(1).?);
    try std.testing.expect(frame.columnName(2) == null);

    const size = frame.shape();
    try std.testing.expectEqual(@as(usize, 3), size.rows);
    try std.testing.expectEqual(@as(usize, 2), size.columns);
    try std.testing.expectEqual(@as(usize, 3), frame.len());
    try std.testing.expect(!frame.isEmpty());

    var empty = DataFrame.init(allocator);
    defer empty.deinit();
    try std.testing.expect(empty.isEmpty());
}

test "DataFrame 取列与子集" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    const close = frame.get("close").?;
    try approx(close.getF64(0), 1.5);
    try approx(close.getF64(2), 3.5);
    try std.testing.expect(frame.get("volume") == null);
    try std.testing.expect(frame.contains("open"));

    var subset = try frame.select(&.{"close"});
    defer subset.deinit();
    try std.testing.expectEqual(@as(usize, 1), subset.columnCount());
    try std.testing.expectEqualStrings("close", subset.columnName(0).?);
    try std.testing.expectEqual(@as(usize, 3), subset.len());

    try std.testing.expectError(error.UnknownColumn, frame.select(&.{"volume"}));
}

test "DataFrame 原位替换" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    try frame.set("open", try Series.initFloat(allocator, &.{ 10, 20, 30 }));
    try std.testing.expectEqual(@as(usize, 2), frame.columnCount());
    try std.testing.expectEqualStrings("open", frame.columnName(0).?);
    try approx(frame.get("open").?.getF64(0), 10);
    try approx(frame.get("open").?.getF64(2), 30);
}

test "DataFrame 追加列" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    try frame.set("volume", try Series.initFloat(allocator, &.{ 100, 200, 300 }));
    try std.testing.expectEqual(@as(usize, 3), frame.columnCount());
    try std.testing.expectEqualStrings("volume", frame.columnName(2).?);
    const size = frame.shape();
    try std.testing.expectEqual(@as(usize, 3), size.columns);
}

test "DataFrame 列长校验" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    var short = try Series.initFloat(allocator, &.{1});
    defer short.deinit();
    try std.testing.expectError(error.LengthMismatch, frame.set("volume", short));
    try std.testing.expectEqual(@as(usize, 2), frame.columnCount());
}

test "DataFrame 深拷贝" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    var copied = try frame.copy();
    defer copied.deinit();

    try copied.set("open", try Series.initFloat(allocator, &.{ 0, 0, 0 }));
    try approx(frame.get("open").?.getF64(0), 1);
    try approx(copied.get("open").?.getF64(0), 0);
}

test "DataFrame 文本表示" {
    const allocator = std.testing.allocator;
    var frame = try sample(allocator);
    defer frame.deinit();

    var out: std.Io.Writer.Allocating = .init(allocator);
    defer out.deinit();
    try out.writer.print("{f}", .{frame});
    try std.testing.expectEqualStrings(
        "DataFrame(shape=(3, 2), columns=[open, close])",
        out.written(),
    );
}
