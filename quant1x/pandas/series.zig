// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! Series —— 一列同构数据的可移植容器（Zig 扩展实现）
//!
//! 对齐 quant1x/pandas/series.py：Python 用 pandas 承载是 spec，本文件以
//! quant1x/num/ndarray.zig 的 NdArray 为底座（float64 / bool），算子委托 NdArray。
//! Zig 无运算符重载，契约以命名方法为准。容器持有分配内存，调用方负责 deinit。

const std = @import("std");
const nd = @import("num_ndarray");

pub const Series = struct {
    allocator: std.mem.Allocator,
    data: nd.NdArray,
    name: []u8,

    // -- 构造 / 释放 ------------------------------------------------------
    pub fn initFloat(allocator: std.mem.Allocator, data: []const f64) !Series {
        const array = try nd.NdArray.initFloat(allocator, data);
        return .{ .allocator = allocator, .data = array, .name = try allocator.dupe(u8, "") };
    }

    pub fn initBool(allocator: std.mem.Allocator, data: []const bool) !Series {
        const array = try nd.NdArray.initBool(allocator, data);
        return .{ .allocator = allocator, .data = array, .name = try allocator.dupe(u8, "") };
    }

    pub fn deinit(self: *Series) void {
        self.allocator.free(self.name);
        self.data.deinit();
    }

    fn wrap(self: *const Series, array: nd.NdArray) !Series {
        return .{ .allocator = self.allocator, .data = array, .name = try self.allocator.dupe(u8, self.name) };
    }

    pub fn setName(self: *Series, name: []const u8) !void {
        const owned = try self.allocator.dupe(u8, name);
        self.allocator.free(self.name);
        self.name = owned;
    }

    // -- 属性 -------------------------------------------------------------
    pub fn nameOf(self: *const Series) []const u8 {
        return self.name;
    }

    pub fn dtype(self: *const Series) nd.DType {
        return self.data.dtype();
    }

    pub fn len(self: *const Series) usize {
        return self.data.len();
    }

    pub fn isEmpty(self: *const Series) bool {
        return self.data.len() == 0;
    }

    pub fn isBool(self: *const Series) bool {
        return self.data.isBool();
    }

    pub fn values(self: *const Series, allocator: std.mem.Allocator) ![]f64 {
        return self.data.toF64(allocator);
    }

    pub fn toBools(self: *const Series, allocator: std.mem.Allocator) ![]bool {
        return self.data.toBools(allocator);
    }

    pub fn getF64(self: *const Series, index: usize) f64 {
        return self.data.getF64(index);
    }

    pub fn getBool(self: *const Series, index: usize) bool {
        return self.data.getBool(index);
    }

    pub fn setF64(self: *Series, index: usize, value: f64) void {
        self.data.setF64(index, value);
    }

    pub fn setBool(self: *Series, index: usize, value: bool) void {
        self.data.setBool(index, value);
    }

    pub fn copy(self: *const Series) !Series {
        return self.wrap(try self.data.copy(self.allocator));
    }

    pub fn slice(self: *const Series, start: usize, end: usize) !Series {
        return self.wrap(try self.data.slice(self.allocator, start, end));
    }

    // -- 变换 -------------------------------------------------------------
    pub fn shift(self: *const Series, periods: i64) !Series {
        return self.wrap(try self.data.shift(periods));
    }

    pub fn diff(self: *const Series, periods: i64) !Series {
        return self.wrap(try self.data.diff(periods));
    }

    pub fn cumsum(self: *const Series) !Series {
        return self.wrap(try self.data.cumsum());
    }

    pub fn fillNa(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.fillNa(value));
    }

    pub fn apply(self: *const Series, func: fn (f64) f64) !Series {
        const buffer = try self.data.toF64(self.allocator);
        for (buffer, 0..) |value, i| buffer[i] = func(value);
        return self.wrap(nd.NdArray.fromFloats(self.allocator, buffer));
    }

    // -- 四则 -------------------------------------------------------------
    pub fn add(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.add(&other.data));
    }
    pub fn sub(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.sub(&other.data));
    }
    pub fn mul(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.mul(&other.data));
    }
    pub fn div(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.div(&other.data));
    }
    pub fn addScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.addScalar(value));
    }
    pub fn subScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.subScalar(value));
    }
    pub fn mulScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.mulScalar(value));
    }
    pub fn divScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.divScalar(value));
    }

    // -- 比较 -------------------------------------------------------------
    pub fn gt(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.gt(&other.data));
    }
    pub fn gte(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.gte(&other.data));
    }
    pub fn lt(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.lt(&other.data));
    }
    pub fn lte(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.lte(&other.data));
    }
    pub fn equal(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.equal(&other.data));
    }
    pub fn notEqual(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.notEqual(&other.data));
    }
    pub fn gtScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.gtScalar(value));
    }
    pub fn ltScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.ltScalar(value));
    }
    pub fn equalScalar(self: *const Series, value: f64) !Series {
        return self.wrap(try self.data.equalScalar(value));
    }

    // -- 逻辑 -------------------------------------------------------------
    pub fn logicAnd(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.logicAnd(&other.data));
    }
    pub fn logicOr(self: *const Series, other: *const Series) !Series {
        return self.wrap(try self.data.logicOr(&other.data));
    }
    pub fn logicNot(self: *const Series) !Series {
        return self.wrap(try self.data.logicNot());
    }

    // -- 归约 -------------------------------------------------------------
    pub fn sum(self: *const Series) f64 {
        return self.data.sum();
    }
    pub fn mean(self: *const Series) f64 {
        return self.data.mean();
    }
    pub fn median(self: *const Series) !f64 {
        return self.data.median();
    }
    pub fn minValue(self: *const Series) f64 {
        return self.data.minValue();
    }
    pub fn maxValue(self: *const Series) f64 {
        return self.data.maxValue();
    }
    pub fn argmin(self: *const Series) i64 {
        return self.data.argmin();
    }
    pub fn argmax(self: *const Series) i64 {
        return self.data.argmax();
    }
    pub fn variance(self: *const Series) f64 {
        return self.data.variance();
    }
    pub fn stddev(self: *const Series) f64 {
        return self.data.stddev();
    }

    // -- 滚动 / EWM -------------------------------------------------------
    pub fn rolling(self: *const Series, window: i64) Rolling {
        return .{ .series = self, .window = window };
    }

    pub fn ewm(self: *const Series, alpha: f64) Ewm {
        return .{ .series = self, .alpha = alpha };
    }

    pub fn ewmSpan(self: *const Series, span: i64) Ewm {
        const period: f64 = if (span <= 0) 1.0 else @floatFromInt(span);
        return .{ .series = self, .alpha = 2.0 / (period + 1.0) };
    }
};

/// 滚动窗口
pub const Rolling = struct {
    series: *const Series,
    window: i64,

    pub fn sum(self: Rolling) !Series {
        return self.series.wrap(try self.series.data.rollingSum(self.window));
    }
    pub fn mean(self: Rolling) !Series {
        return self.series.wrap(try self.series.data.rollingMean(self.window));
    }
    pub fn stddev(self: Rolling) !Series {
        return self.series.wrap(try self.series.data.rollingStd(self.window));
    }
    pub fn minValue(self: Rolling) !Series {
        return self.series.wrap(try self.series.data.rollingMin(self.window));
    }
    pub fn maxValue(self: Rolling) !Series {
        return self.series.wrap(try self.series.data.rollingMax(self.window));
    }
};

/// 指数加权窗口
pub const Ewm = struct {
    series: *const Series,
    alpha: f64,

    pub fn mean(self: Ewm) !Series {
        return self.series.wrap(try self.series.data.ewmMean(self.alpha));
    }
};
