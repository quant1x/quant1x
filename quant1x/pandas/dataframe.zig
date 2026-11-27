// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! DataFrame —— 多列容器的可移植实现（Zig 扩展）
//!
//! 对齐 quant1x/pandas/dataframe.py：列名 → Series 的有序表，只做按列名存取，
//! 数值语义在 Series 与 num。Zig 无运算符重载，df[name] / df[name] = value /
//! df[names] 分别对应 get / set / select。容器持有分配内存，调用方负责 deinit。
//!
//! `set` 成功时接管 `values` 的所有权；失败时所有权仍归调用方。

const std = @import("std");
const ser = @import("series.zig");

const Series = ser.Series;

pub const DataFrame = struct {
    allocator: std.mem.Allocator,
    columns: std.ArrayList(Column) = .empty,

    pub const Column = struct {
        name: []u8,
        values: Series,
    };

    pub const Shape = struct {
        rows: usize,
        columns: usize,
    };

    // -- 构造 / 释放 ------------------------------------------------------
    pub fn init(allocator: std.mem.Allocator) DataFrame {
        return .{ .allocator = allocator };
    }

    pub fn deinit(self: *DataFrame) void {
        for (self.columns.items) |*column| {
            self.allocator.free(column.name);
            column.values.deinit();
        }
        self.columns.deinit(self.allocator);
    }

    // -- 属性 -------------------------------------------------------------
    pub fn columnCount(self: *const DataFrame) usize {
        return self.columns.items.len;
    }

    pub fn len(self: *const DataFrame) usize {
        if (self.columns.items.len == 0) return 0;
        return self.columns.items[0].values.len();
    }

    pub fn isEmpty(self: *const DataFrame) bool {
        return self.len() == 0;
    }

    pub fn shape(self: *const DataFrame) Shape {
        return .{ .rows = self.len(), .columns = self.columnCount() };
    }

    pub fn contains(self: *const DataFrame, name: []const u8) bool {
        return self.indexOf(name) != null;
    }

    /// 第 index 列的列名, 顺序与插入序一致
    pub fn columnName(self: *const DataFrame, index: usize) ?[]const u8 {
        if (index >= self.columns.items.len) return null;
        return self.columns.items[index].name;
    }

    // -- 取列 / 子集 ------------------------------------------------------
    pub fn get(self: *const DataFrame, name: []const u8) ?*const Series {
        const index = self.indexOf(name) orelse return null;
        return &self.columns.items[index].values;
    }

    pub fn select(self: *const DataFrame, names: []const []const u8) !DataFrame {
        var frame = DataFrame.init(self.allocator);
        errdefer frame.deinit();
        for (names) |name| {
            const values = try (self.get(name) orelse return error.UnknownColumn).copy();
            errdefer {
                var owned = values;
                owned.deinit();
            }
            try frame.set(name, values);
        }
        return frame;
    }

    // -- 写列 -------------------------------------------------------------
    /// 已存在则原位替换, 否则追加; 列长必须与现有行数一致 (成功时接管 values)
    pub fn set(self: *DataFrame, name: []const u8, values: Series) !void {
        if (self.columns.items.len > 0 and values.len() != self.len()) {
            return error.LengthMismatch;
        }
        if (self.indexOf(name)) |index| {
            self.columns.items[index].values.deinit();
            self.columns.items[index].values = values;
            return;
        }
        const owned_name = try self.allocator.dupe(u8, name);
        errdefer self.allocator.free(owned_name);
        try self.columns.append(self.allocator, .{ .name = owned_name, .values = values });
    }

    // -- 复制 -------------------------------------------------------------
    pub fn copy(self: *const DataFrame) !DataFrame {
        var frame = DataFrame.init(self.allocator);
        errdefer frame.deinit();
        for (self.columns.items) |column| {
            const values = try column.values.copy();
            errdefer {
                var owned = values;
                owned.deinit();
            }
            const owned_name = try self.allocator.dupe(u8, column.name);
            errdefer self.allocator.free(owned_name);
            try frame.columns.append(self.allocator, .{ .name = owned_name, .values = values });
        }
        return frame;
    }

    // -- 调试输出 ---------------------------------------------------------
    /// Zig 0.17 的自定义格式化: 用 `{f}` 触发
    pub fn format(self: DataFrame, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        const size = self.shape();
        try writer.print("DataFrame(shape=({}, {}), columns=[", .{ size.rows, size.columns });
        for (self.columns.items, 0..) |column, i| {
            if (i > 0) try writer.writeAll(", ");
            try writer.writeAll(column.name);
        }
        try writer.writeAll("])");
    }

    fn indexOf(self: *const DataFrame, name: []const u8) ?usize {
        for (self.columns.items, 0..) |column, i| {
            if (std.mem.eql(u8, column.name, name)) return i;
        }
        return null;
    }
};
