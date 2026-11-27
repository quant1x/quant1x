// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! ndarray —— 对标 numpy.ndarray 的一维可移植容器（Zig 扩展实现）
//!
//! 方法表与 ndarray.py / ndarray.h / ndarray.rs 一致；算子全部委托 num.zig。
//! 容器持有分配的内存，调用方负责 deinit。

const std = @import("std");
const num = @import("num.zig");

pub const DType = enum { float64, bool };

pub const NdArray = struct {
    allocator: std.mem.Allocator,
    data: Data,

    pub const Data = union(enum) {
        floats: []f64,
        bools: []bool,
    };

    // -- 构造 / 释放 ------------------------------------------------------
    pub fn fromFloats(allocator: std.mem.Allocator, values: []f64) NdArray {
        return .{ .allocator = allocator, .data = .{ .floats = values } };
    }

    pub fn fromBools(allocator: std.mem.Allocator, values: []bool) NdArray {
        return .{ .allocator = allocator, .data = .{ .bools = values } };
    }

    pub fn initFloat(allocator: std.mem.Allocator, values: []const f64) !NdArray {
        return fromFloats(allocator, try allocator.dupe(f64, values));
    }

    pub fn initBool(allocator: std.mem.Allocator, values: []const bool) !NdArray {
        return fromBools(allocator, try allocator.dupe(bool, values));
    }

    pub fn deinit(self: *NdArray) void {
        switch (self.data) {
            .floats => |values| self.allocator.free(values),
            .bools => |values| self.allocator.free(values),
        }
    }

    // -- 属性 -------------------------------------------------------------
    pub fn isBool(self: *const NdArray) bool {
        return switch (self.data) {
            .bools => true,
            else => false,
        };
    }

    pub fn dtype(self: *const NdArray) DType {
        return if (self.isBool()) .bool else .float64;
    }

    pub fn len(self: *const NdArray) usize {
        return switch (self.data) {
            .floats => |values| values.len,
            .bools => |values| values.len,
        };
    }

    pub fn isEmpty(self: *const NdArray) bool {
        return self.len() == 0;
    }

    fn floats(self: *const NdArray) []const f64 {
        return switch (self.data) {
            .floats => |values| values,
            .bools => @panic("需要 float64 数组"),
        };
    }

    fn bools(self: *const NdArray) []const bool {
        return switch (self.data) {
            .bools => |values| values,
            .floats => @panic("需要 bool 数组"),
        };
    }

    pub fn toF64(self: *const NdArray, allocator: std.mem.Allocator) ![]f64 {
        switch (self.data) {
            .floats => |values| return allocator.dupe(f64, values),
            .bools => |values| {
                const out = try allocator.alloc(f64, values.len);
                for (values, 0..) |value, i| out[i] = if (value) 1 else 0;
                return out;
            },
        }
    }

    pub fn toBools(self: *const NdArray, allocator: std.mem.Allocator) ![]bool {
        switch (self.data) {
            .bools => |values| return allocator.dupe(bool, values),
            .floats => |values| {
                const out = try allocator.alloc(bool, values.len);
                for (values, 0..) |value, i| out[i] = num.isTrue(value);
                return out;
            },
        }
    }

    pub fn getF64(self: *const NdArray, index: usize) f64 {
        return switch (self.data) {
            .floats => |values| values[index],
            .bools => |values| if (values[index]) 1 else 0,
        };
    }

    pub fn getBool(self: *const NdArray, index: usize) bool {
        return switch (self.data) {
            .bools => |values| values[index],
            .floats => |values| num.isTrue(values[index]),
        };
    }

    pub fn setF64(self: *NdArray, index: usize, value: f64) void {
        switch (self.data) {
            .floats => |*values| values[index] = value,
            .bools => {},
        }
    }

    pub fn setBool(self: *NdArray, index: usize, value: bool) void {
        switch (self.data) {
            .bools => |*values| values[index] = value,
            .floats => {},
        }
    }

    pub fn copy(self: *const NdArray, allocator: std.mem.Allocator) !NdArray {
        return switch (self.data) {
            .floats => |values| fromFloats(allocator, try allocator.dupe(f64, values)),
            .bools => |values| fromBools(allocator, try allocator.dupe(bool, values)),
        };
    }

    pub fn slice(self: *const NdArray, allocator: std.mem.Allocator, start: usize, end: usize) !NdArray {
        const length = self.len();
        const stop = @min(end, length);
        const begin = @min(start, stop);
        return switch (self.data) {
            .floats => |values| fromFloats(allocator, try allocator.dupe(f64, values[begin..stop])),
            .bools => |values| fromBools(allocator, try allocator.dupe(bool, values[begin..stop])),
        };
    }

    pub fn astype(self: *const NdArray, allocator: std.mem.Allocator, target: DType) !NdArray {
        switch (target) {
            .float64 => return fromFloats(allocator, try self.toF64(allocator)),
            .bool => return fromBools(allocator, try self.toBools(allocator)),
        }
    }

    // -- 静态构造 ---------------------------------------------------------
    pub fn zeros(allocator: std.mem.Allocator, length: usize) !NdArray {
        return fromFloats(allocator, try num.zeros(allocator, length));
    }

    pub fn ones(allocator: std.mem.Allocator, length: usize) !NdArray {
        return fromFloats(allocator, try num.ones(allocator, length));
    }

    pub fn full(allocator: std.mem.Allocator, length: usize, value: f64) !NdArray {
        const out = try allocator.alloc(f64, length);
        @memset(out, value);
        return fromFloats(allocator, out);
    }

    pub fn arange(allocator: std.mem.Allocator, start: f64, stop: f64, step: f64) !NdArray {
        return fromFloats(allocator, try num.arange(allocator, start, stop, step));
    }

    pub fn linspace(allocator: std.mem.Allocator, start: f64, stop: f64, count: usize) !NdArray {
        return fromFloats(allocator, try num.linspace(allocator, start, stop, count));
    }

    // -- 四则 -------------------------------------------------------------
    pub fn add(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.add(self.allocator, self.floats(), other.floats()));
    }
    pub fn sub(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.sub(self.allocator, self.floats(), other.floats()));
    }
    pub fn mul(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.mul(self.allocator, self.floats(), other.floats()));
    }
    pub fn div(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.div(self.allocator, self.floats(), other.floats()));
    }
    pub fn addScalar(self: *const NdArray, value: f64) !NdArray {
        return fromFloats(self.allocator, try num.addScalar(self.allocator, self.floats(), value));
    }
    pub fn subScalar(self: *const NdArray, value: f64) !NdArray {
        return fromFloats(self.allocator, try num.subScalar(self.allocator, self.floats(), value));
    }
    pub fn mulScalar(self: *const NdArray, value: f64) !NdArray {
        return fromFloats(self.allocator, try num.mulScalar(self.allocator, self.floats(), value));
    }
    pub fn divScalar(self: *const NdArray, value: f64) !NdArray {
        return fromFloats(self.allocator, try num.divScalar(self.allocator, self.floats(), value));
    }
    pub fn maximum(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.maximum(self.allocator, self.floats(), other.floats()));
    }
    pub fn minimum(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.minimum(self.allocator, self.floats(), other.floats()));
    }

    // -- 一元 -------------------------------------------------------------
    pub fn abs(self: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.abs(self.allocator, self.floats()));
    }
    pub fn sqrt(self: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.sqrt(self.allocator, self.floats()));
    }
    pub fn log(self: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.log(self.allocator, self.floats()));
    }
    pub fn pow(self: *const NdArray, exponent: f64) !NdArray {
        return fromFloats(self.allocator, try num.pow(self.allocator, self.floats(), exponent));
    }

    // -- 比较 -------------------------------------------------------------
    pub fn gt(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.gt(self.allocator, self.floats(), other.floats()));
    }
    pub fn gte(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.gte(self.allocator, self.floats(), other.floats()));
    }
    pub fn lt(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.lt(self.allocator, self.floats(), other.floats()));
    }
    pub fn lte(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.lte(self.allocator, self.floats(), other.floats()));
    }
    pub fn equal(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.equal(self.allocator, self.floats(), other.floats()));
    }
    pub fn notEqual(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.notEqual(self.allocator, self.floats(), other.floats()));
    }

    fn filledFloat(self: *const NdArray, value: f64) ![]f64 {
        const out = try self.allocator.alloc(f64, self.len());
        @memset(out, value);
        return out;
    }

    pub fn gtScalar(self: *const NdArray, value: f64) !NdArray {
        const right = try self.filledFloat(value);
        defer self.allocator.free(right);
        return fromBools(self.allocator, try num.gt(self.allocator, self.floats(), right));
    }
    pub fn ltScalar(self: *const NdArray, value: f64) !NdArray {
        const right = try self.filledFloat(value);
        defer self.allocator.free(right);
        return fromBools(self.allocator, try num.lt(self.allocator, self.floats(), right));
    }
    pub fn equalScalar(self: *const NdArray, value: f64) !NdArray {
        const right = try self.filledFloat(value);
        defer self.allocator.free(right);
        return fromBools(self.allocator, try num.equal(self.allocator, self.floats(), right));
    }

    // -- 逻辑 -------------------------------------------------------------
    pub fn logicAnd(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.logicAnd(self.allocator, self.bools(), other.bools()));
    }
    pub fn logicOr(self: *const NdArray, other: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.logicOr(self.allocator, self.bools(), other.bools()));
    }
    pub fn logicNot(self: *const NdArray) !NdArray {
        return fromBools(self.allocator, try num.logicNot(self.allocator, self.bools()));
    }

    // -- 归约 -------------------------------------------------------------
    pub fn sum(self: *const NdArray) f64 {
        return num.sumValue(self.floats());
    }
    pub fn mean(self: *const NdArray) f64 {
        return num.mean(self.floats());
    }
    pub fn median(self: *const NdArray) !f64 {
        return num.median(self.allocator, self.floats());
    }
    pub fn minValue(self: *const NdArray) f64 {
        return num.minValue(self.floats());
    }
    pub fn maxValue(self: *const NdArray) f64 {
        return num.maxValue(self.floats());
    }
    pub fn argmin(self: *const NdArray) i64 {
        return num.argmin(self.floats());
    }
    pub fn argmax(self: *const NdArray) i64 {
        return num.argmax(self.floats());
    }
    pub fn variance(self: *const NdArray) f64 {
        return num.variance(self.floats());
    }
    pub fn stddev(self: *const NdArray) f64 {
        return num.stddev(self.floats());
    }
    pub fn cumsum(self: *const NdArray) !NdArray {
        return fromFloats(self.allocator, try num.cumsum(self.allocator, self.floats()));
    }

    // -- 移位 / 差分 / 填充 ----------------------------------------------
    pub fn shift(self: *const NdArray, periods: i64) !NdArray {
        return fromFloats(self.allocator, try num.shift(self.allocator, self.floats(), periods));
    }
    pub fn diff(self: *const NdArray, periods: i64) !NdArray {
        return fromFloats(self.allocator, try num.diff(self.allocator, self.floats(), periods));
    }
    pub fn fillNa(self: *const NdArray, value: f64) !NdArray {
        return fromFloats(self.allocator, try num.fill(self.allocator, self.floats(), value));
    }

    // -- 滚动 / EWM -------------------------------------------------------
    fn constWindow(self: *const NdArray, value: i64) ![]i64 {
        return num.constantWindow(self.allocator, value, self.len());
    }

    pub fn rollingSum(self: *const NdArray, window: i64) !NdArray {
        const w = try self.constWindow(window);
        defer self.allocator.free(w);
        return fromFloats(self.allocator, try num.rollingSum(self.allocator, self.floats(), w));
    }
    pub fn rollingMean(self: *const NdArray, window: i64) !NdArray {
        const w = try self.constWindow(window);
        defer self.allocator.free(w);
        return fromFloats(self.allocator, try num.rollingMean(self.allocator, self.floats(), w));
    }
    pub fn rollingStd(self: *const NdArray, window: i64) !NdArray {
        const w = try self.constWindow(window);
        defer self.allocator.free(w);
        return fromFloats(self.allocator, try num.rollingStd(self.allocator, self.floats(), w));
    }
    pub fn rollingMin(self: *const NdArray, window: i64) !NdArray {
        const w = try self.constWindow(window);
        defer self.allocator.free(w);
        return fromFloats(self.allocator, try num.rollingMin(self.allocator, self.floats(), w));
    }
    pub fn rollingMax(self: *const NdArray, window: i64) !NdArray {
        const w = try self.constWindow(window);
        defer self.allocator.free(w);
        return fromFloats(self.allocator, try num.rollingMax(self.allocator, self.floats(), w));
    }
    pub fn rollingAvedev(self: *const NdArray, window: i64) !NdArray {
        const w = try self.constWindow(window);
        defer self.allocator.free(w);
        return fromFloats(self.allocator, try num.rollingAvedev(self.allocator, self.floats(), w));
    }
    pub fn ewmMean(self: *const NdArray, alpha: f64) !NdArray {
        return fromFloats(self.allocator, try num.ewmMean(self.allocator, self.floats(), alpha));
    }
};
