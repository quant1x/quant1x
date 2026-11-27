// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! spec: quant1x/num/*.py
//!
//! 数值规格层的 Zig 对齐实现（扩展角色）。口径见 quant1x/num/num.md:
//! 一维 f64 序列、NaN 缺失、顺序累加、完整窗口、总体方差、均值中心化回归、半程远离零。
//! 容器用 []f64 / []bool，调用方传入 allocator，返回的切片由调用方释放。

const std = @import("std");

pub const Series = []f64;
pub const Bools = []bool;

/// 缺失值
pub fn nan() f64 {
    return std.math.nan(f64);
}

/// 判断缺失
pub fn isNan(value: f64) bool {
    return std.math.isNan(value);
}

/// 判断真值
pub fn isTrue(value: f64) bool {
    return !std.math.isNan(value) and value != 0.0;
}

/// 生成与输入等长的全 NaN 序列
pub fn defaultLike(allocator: std.mem.Allocator, length: usize) ![]f64 {
    const out = try allocator.alloc(f64, length);
    @memset(out, nan());
    return out;
}

/// 取最后一个值; 空序列返回 NaN
pub fn toScalar(values: []const f64) f64 {
    if (values.len == 0) return nan();
    return values[values.len - 1];
}

fn rightAt(values: []const f64, index: usize) f64 {
    if (index < values.len) return values[index];
    return nan();
}

// ---------------------------------------------------------------------------
// 逐元素
// ---------------------------------------------------------------------------

pub fn abs(allocator: std.mem.Allocator, x: []const f64) ![]f64 {
    const out = try allocator.alloc(f64, x.len);
    for (x, 0..) |v, i| out[i] = @abs(v);
    return out;
}

pub fn sqrt(allocator: std.mem.Allocator, x: []const f64) ![]f64 {
    const out = try allocator.alloc(f64, x.len);
    for (x, 0..) |v, i| out[i] = @sqrt(v);
    return out;
}

pub fn log(allocator: std.mem.Allocator, x: []const f64) ![]f64 {
    const out = try allocator.alloc(f64, x.len);
    for (x, 0..) |v, i| out[i] = @log(v);
    return out;
}

pub fn pow(allocator: std.mem.Allocator, x: []const f64, exponent: f64) ![]f64 {
    const out = try allocator.alloc(f64, x.len);
    for (x, 0..) |v, i| out[i] = std.math.pow(f64, v, exponent);
    return out;
}

fn binary(allocator: std.mem.Allocator, left: []const f64, right: []const f64, comptime op: fn (f64, f64) f64) ![]f64 {
    const out = try allocator.alloc(f64, left.len);
    for (left, 0..) |v, i| out[i] = op(v, rightAt(right, i));
    return out;
}

fn addOp(a: f64, b: f64) f64 {
    return a + b;
}
fn subOp(a: f64, b: f64) f64 {
    return a - b;
}
fn mulOp(a: f64, b: f64) f64 {
    return a * b;
}
fn divOp(a: f64, b: f64) f64 {
    return a / b;
}

pub fn add(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]f64 {
    return binary(allocator, left, right, addOp);
}
pub fn sub(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]f64 {
    return binary(allocator, left, right, subOp);
}
pub fn mul(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]f64 {
    return binary(allocator, left, right, mulOp);
}
pub fn div(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]f64 {
    return binary(allocator, left, right, divOp);
}

fn scalarOp(allocator: std.mem.Allocator, x: []const f64, value: f64, comptime op: fn (f64, f64) f64) ![]f64 {
    const out = try allocator.alloc(f64, x.len);
    for (x, 0..) |v, i| out[i] = op(v, value);
    return out;
}

pub fn addScalar(allocator: std.mem.Allocator, x: []const f64, value: f64) ![]f64 {
    return scalarOp(allocator, x, value, addOp);
}
pub fn subScalar(allocator: std.mem.Allocator, x: []const f64, value: f64) ![]f64 {
    return scalarOp(allocator, x, value, subOp);
}
pub fn mulScalar(allocator: std.mem.Allocator, x: []const f64, value: f64) ![]f64 {
    return scalarOp(allocator, x, value, mulOp);
}
pub fn divScalar(allocator: std.mem.Allocator, x: []const f64, value: f64) ![]f64 {
    return scalarOp(allocator, x, value, divOp);
}

fn maxOp(a: f64, b: f64) f64 {
    if (std.math.isNan(a) or std.math.isNan(b)) return nan();
    return @max(a, b);
}
fn minOp(a: f64, b: f64) f64 {
    if (std.math.isNan(a) or std.math.isNan(b)) return nan();
    return @min(a, b);
}

pub fn maximum(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]f64 {
    return binary(allocator, left, right, maxOp);
}
pub fn minimum(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]f64 {
    return binary(allocator, left, right, minOp);
}

pub fn maximumScalar(allocator: std.mem.Allocator, x: []const f64, value: f64) ![]f64 {
    return scalarOp(allocator, x, value, maxOp);
}
pub fn minimumScalar(allocator: std.mem.Allocator, x: []const f64, value: f64) ![]f64 {
    return scalarOp(allocator, x, value, minOp);
}

/// 条件选择: 按最长对齐, 缺位补 NaN
pub fn where(allocator: std.mem.Allocator, condition: []const bool, trueValue: []const f64, falseValue: []const f64) ![]f64 {
    var length = condition.len;
    if (trueValue.len > length) length = trueValue.len;
    if (falseValue.len > length) length = falseValue.len;
    const out = try allocator.alloc(f64, length);
    for (0..length) |i| {
        const flag = i < condition.len and condition[i];
        out[i] = if (flag) rightAt(trueValue, i) else rightAt(falseValue, i);
    }
    return out;
}

// ---------------------------------------------------------------------------
// 逻辑与比较
// ---------------------------------------------------------------------------

fn compare(allocator: std.mem.Allocator, left: []const f64, right: []const f64, comptime op: fn (f64, f64) bool) ![]bool {
    const out = try allocator.alloc(bool, left.len);
    for (left, 0..) |v, i| out[i] = op(v, rightAt(right, i));
    return out;
}

fn gtOp(a: f64, b: f64) bool {
    return a > b;
}
fn gteOp(a: f64, b: f64) bool {
    return a >= b;
}
fn ltOp(a: f64, b: f64) bool {
    return a < b;
}
fn lteOp(a: f64, b: f64) bool {
    return a <= b;
}
fn eqOp(a: f64, b: f64) bool {
    return a == b;
}
fn neOp(a: f64, b: f64) bool {
    return a != b;
}

pub fn gt(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]bool {
    return compare(allocator, left, right, gtOp);
}
pub fn gte(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]bool {
    return compare(allocator, left, right, gteOp);
}
pub fn lt(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]bool {
    return compare(allocator, left, right, ltOp);
}
pub fn lte(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]bool {
    return compare(allocator, left, right, lteOp);
}
pub fn equal(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]bool {
    return compare(allocator, left, right, eqOp);
}
pub fn notEqual(allocator: std.mem.Allocator, left: []const f64, right: []const f64) ![]bool {
    return compare(allocator, left, right, neOp);
}

pub fn toBools(allocator: std.mem.Allocator, values: []const f64) ![]bool {
    const out = try allocator.alloc(bool, values.len);
    for (values, 0..) |v, i| out[i] = isTrue(v);
    return out;
}

pub fn logicAnd(allocator: std.mem.Allocator, left: []const bool, right: []const bool) ![]bool {
    var length = left.len;
    if (right.len > length) length = right.len;
    const out = try allocator.alloc(bool, length);
    for (0..length) |i| {
        const l = i < left.len and left[i];
        const r = i < right.len and right[i];
        out[i] = l and r;
    }
    return out;
}

pub fn logicOr(allocator: std.mem.Allocator, left: []const bool, right: []const bool) ![]bool {
    var length = left.len;
    if (right.len > length) length = right.len;
    const out = try allocator.alloc(bool, length);
    for (0..length) |i| {
        const l = i < left.len and left[i];
        const r = i < right.len and right[i];
        out[i] = l or r;
    }
    return out;
}

pub fn logicNot(allocator: std.mem.Allocator, values: []const bool) ![]bool {
    const out = try allocator.alloc(bool, values.len);
    for (values, 0..) |v, i| out[i] = !v;
    return out;
}

pub fn allOf(values: []const bool) bool {
    for (values) |v| {
        if (!v) return false;
    }
    return true;
}

pub fn anyOf(values: []const bool) bool {
    for (values) |v| {
        if (v) return true;
    }
    return false;
}

pub fn countTrue(values: []const bool) usize {
    var count: usize = 0;
    for (values) |v| {
        if (v) count += 1;
    }
    return count;
}

// ---------------------------------------------------------------------------
// 归约
// ---------------------------------------------------------------------------

fn sequentialSum(values: []const f64) f64 {
    var total: f64 = 0;
    for (values) |v| total += v;
    return total;
}

pub fn sumValue(values: []const f64) f64 {
    return sequentialSum(values);
}

pub fn mean(values: []const f64) f64 {
    if (values.len == 0) return nan();
    return sequentialSum(values) / @as(f64, @floatFromInt(values.len));
}

fn hasNan(values: []const f64) bool {
    for (values) |v| {
        if (std.math.isNan(v)) return true;
    }
    return false;
}

fn reducible(values: []const f64) bool {
    return values.len > 0 and !hasNan(values);
}

pub fn median(allocator: std.mem.Allocator, values: []const f64) !f64 {
    if (!reducible(values)) return nan();
    const buf = try allocator.dupe(f64, values);
    defer allocator.free(buf);
    std.mem.sort(f64, buf, {}, std.sort.asc(f64));
    const middle = buf.len / 2;
    if (buf.len % 2 == 0) return (buf[middle - 1] + buf[middle]) / 2.0;
    return buf[middle];
}

pub fn minValue(values: []const f64) f64 {
    if (!reducible(values)) return nan();
    var result = std.math.inf(f64);
    for (values) |v| {
        if (v < result) result = v;
    }
    return result;
}

pub fn maxValue(values: []const f64) f64 {
    if (!reducible(values)) return nan();
    var result = -std.math.inf(f64);
    for (values) |v| {
        if (v > result) result = v;
    }
    return result;
}

pub fn argmin(values: []const f64) i64 {
    if (!reducible(values)) return -1;
    var best: usize = 0;
    for (values, 0..) |v, i| {
        if (v < values[best]) best = i;
    }
    return @intCast(best);
}

pub fn argmax(values: []const f64) i64 {
    if (!reducible(values)) return -1;
    var best: usize = 0;
    for (values, 0..) |v, i| {
        if (v > values[best]) best = i;
    }
    return @intCast(best);
}

pub fn variance(values: []const f64) f64 {
    if (values.len == 0) return 0;
    const average = mean(values);
    var total: f64 = 0;
    for (values) |v| {
        const d = v - average;
        total += mulOnly(d, d);
    }
    return total / @as(f64, @floatFromInt(values.len));
}

pub fn stddev(values: []const f64) f64 {
    return @sqrt(variance(values));
}

pub fn covariance(left: []const f64, right: []const f64) f64 {
    if (left.len != right.len or left.len == 0) return 0;
    const meanLeft = mean(left);
    const meanRight = mean(right);
    var total: f64 = 0;
    for (left, 0..) |v, i| {
        total += mulOnly(v - meanLeft, right[i] - meanRight);
    }
    return total / @as(f64, @floatFromInt(left.len));
}

// ---------------------------------------------------------------------------
// 滚动窗口
// ---------------------------------------------------------------------------

fn blockRange(index: usize, size: i64) ?struct { begin: usize, end: usize } {
    if (size <= 0) return null;
    const s: usize = @intCast(size);
    if (s > index + 1) return null;
    return .{ .begin = index + 1 - s, .end = index + 1 };
}

fn rollingReduce(allocator: std.mem.Allocator, x: []const f64, window: []const i64, comptime reduce: fn ([]const f64) f64) ![]f64 {
    const out = try allocator.alloc(f64, x.len);
    @memset(out, nan());
    for (x, 0..) |_, index| {
        const range = blockRange(index, window[index]) orelse continue;
        const block = x[range.begin..range.end];
        if (hasNan(block)) continue;
        out[index] = reduce(block);
    }
    return out;
}

fn blockSum(block: []const f64) f64 {
    return sequentialSum(block);
}
fn blockMean(block: []const f64) f64 {
    return sequentialSum(block) / @as(f64, @floatFromInt(block.len));
}
fn blockStd(block: []const f64) f64 {
    const count = @as(f64, @floatFromInt(block.len));
    const average = sequentialSum(block) / count;
    var total: f64 = 0;
    for (block) |v| {
        const d = v - average;
        total += mulOnly(d, d);
    }
    return @sqrt(total / count);
}
fn blockMin(block: []const f64) f64 {
    var result = std.math.inf(f64);
    for (block) |v| {
        if (v < result) result = v;
    }
    return result;
}
fn blockMax(block: []const f64) f64 {
    var result = -std.math.inf(f64);
    for (block) |v| {
        if (v > result) result = v;
    }
    return result;
}
fn blockAvedev(block: []const f64) f64 {
    const count = @as(f64, @floatFromInt(block.len));
    const average = sequentialSum(block) / count;
    var total: f64 = 0;
    for (block) |v| total += @abs(v - average);
    return total / count;
}

pub fn rollingSum(allocator: std.mem.Allocator, x: []const f64, window: []const i64) ![]f64 {
    return rollingReduce(allocator, x, window, blockSum);
}
pub fn rollingMean(allocator: std.mem.Allocator, x: []const f64, window: []const i64) ![]f64 {
    return rollingReduce(allocator, x, window, blockMean);
}
pub fn rollingStd(allocator: std.mem.Allocator, x: []const f64, window: []const i64) ![]f64 {
    return rollingReduce(allocator, x, window, blockStd);
}
pub fn rollingMin(allocator: std.mem.Allocator, x: []const f64, window: []const i64) ![]f64 {
    return rollingReduce(allocator, x, window, blockMin);
}
pub fn rollingMax(allocator: std.mem.Allocator, x: []const f64, window: []const i64) ![]f64 {
    return rollingReduce(allocator, x, window, blockMax);
}
pub fn rollingAvedev(allocator: std.mem.Allocator, x: []const f64, window: []const i64) ![]f64 {
    return rollingReduce(allocator, x, window, blockAvedev);
}

/// 常量窗口展开
pub fn constantWindow(allocator: std.mem.Allocator, value: i64, length: usize) ![]i64 {
    const out = try allocator.alloc(i64, length);
    @memset(out, value);
    return out;
}

// ---------------------------------------------------------------------------
// 指数加权 / 移位 / 差分
// ---------------------------------------------------------------------------

// mulOnly 阻止编译器把 a*b + c 收缩成 FMA, 保证逐位稳定
inline fn mulOnly(a: f64, b: f64) f64 {
    return a * b;
}

pub fn ewmMean(allocator: std.mem.Allocator, values: []const f64, alpha: f64) ![]f64 {
    const out = try allocator.alloc(f64, values.len);
    @memset(out, nan());
    const decay = 1.0 - alpha;
    var previous: f64 = nan();
    var seeded = false;
    for (values, 0..) |current, i| {
        if (std.math.isNan(current)) {
            if (seeded) out[i] = previous;
            continue;
        }
        if (!seeded) {
            previous = current;
            seeded = true;
            out[i] = previous;
            continue;
        }
        previous = mulOnly(decay, previous) + mulOnly(alpha, current);
        out[i] = previous;
    }
    return out;
}

pub fn shift(allocator: std.mem.Allocator, values: []const f64, periods: i64) ![]f64 {
    const out = try allocator.alloc(f64, values.len);
    @memset(out, nan());
    const n = values.len;
    if (periods == 0) {
        @memcpy(out, values);
    } else if (periods > 0) {
        const offset: usize = @intCast(periods);
        if (offset < n) {
            for (offset..n) |i| out[i] = values[i - offset];
        }
    } else {
        const offset: usize = @intCast(-periods);
        if (offset < n) {
            for (0..n - offset) |i| out[i] = values[i + offset];
        }
    }
    return out;
}

pub fn ref(allocator: std.mem.Allocator, values: []const f64, periods: i64) ![]f64 {
    return shift(allocator, values, periods);
}

pub fn diff(allocator: std.mem.Allocator, values: []const f64, periods: i64) ![]f64 {
    const lagged = try shift(allocator, values, periods);
    defer allocator.free(lagged);
    const out = try allocator.alloc(f64, values.len);
    for (values, 0..) |v, i| out[i] = v - lagged[i];
    return out;
}

pub fn cumsum(allocator: std.mem.Allocator, values: []const f64) ![]f64 {
    const out = try allocator.alloc(f64, values.len);
    var total: f64 = 0;
    for (values, 0..) |v, i| {
        total += v;
        out[i] = total;
    }
    return out;
}

// ---------------------------------------------------------------------------
// 缺失值 / 对齐 / 回归 / 小数位 / 形状
// ---------------------------------------------------------------------------

pub fn fill(allocator: std.mem.Allocator, values: []const f64, value: f64) ![]f64 {
    const out = try allocator.alloc(f64, values.len);
    for (values, 0..) |v, i| out[i] = if (std.math.isNan(v)) value else v;
    return out;
}

pub fn isNa(allocator: std.mem.Allocator, values: []const f64) ![]bool {
    const out = try allocator.alloc(bool, values.len);
    for (values, 0..) |v, i| out[i] = std.math.isNan(v);
    return out;
}

// align 是 Zig 关键字, 用 padTo 命名 (对应 Python 的 align / pad_to)
//
// 跨语言参考算法: Python spec 自身不调用 align / padTo / repeat, 应用层请用
// quant1x/pandas 的容器对齐; 这里只为与 Python 的对齐口径逐位对照而存在.
pub fn padTo(allocator: std.mem.Allocator, values: []const f64, length: usize, fillValue: f64) ![]f64 {
    const out = try allocator.alloc(f64, length);
    @memset(out, fillValue);
    const count = @min(values.len, length);
    @memcpy(out[0..count], values[0..count]);
    return out;
}

pub fn repeat(allocator: std.mem.Allocator, values: []const f64, length: usize) ![]f64 {
    const out = try allocator.alloc(f64, length);
    if (values.len == 0) {
        @memset(out, nan());
        return out;
    }
    for (0..length) |i| out[i] = values[i % values.len];
    return out;
}

pub const Regression = struct {
    slope: f64,
    intercept: f64,
};

pub fn linearRegression(values: []const f64) Regression {
    const count = values.len;
    if (count < 2 or hasNan(values)) return .{ .slope = nan(), .intercept = nan() };
    const meanT = @as(f64, @floatFromInt(count - 1)) / 2.0;
    const meanY = sequentialSum(values) / @as(f64, @floatFromInt(count));
    var numerator: f64 = 0;
    var denominator: f64 = 0;
    for (values, 0..) |v, i| {
        const offset = @as(f64, @floatFromInt(i)) - meanT;
        numerator += mulOnly(offset, v - meanY);
        denominator += mulOnly(offset, offset);
    }
    const slope = numerator / denominator;
    return .{ .slope = slope, .intercept = meanY - slope * meanT };
}

pub fn slopeValue(values: []const f64) f64 {
    return linearRegression(values).slope;
}

pub fn forcast(values: []const f64) f64 {
    const result = linearRegression(values);
    if (std.math.isNan(result.slope)) return nan();
    return result.intercept + result.slope * (@as(f64, @floatFromInt(values.len)) - 1.0);
}

pub fn decimal(value: f64, digits: i32) f64 {
    const places: i32 = if (digits > 0) digits else 0;
    var input = value;
    if (std.math.isNan(input)) input = 0;
    const half: f64 = if (std.math.signbit(input)) -5.0 else 5.0;
    const scale = std.math.pow(f64, 10.0, @floatFromInt(places));
    const wider = std.math.pow(f64, 10.0, @floatFromInt(places + 1));
    return @trunc((input * wider + half) / 10.0) / scale;
}

pub fn decimalSeries(allocator: std.mem.Allocator, values: []const f64, digits: i32) ![]f64 {
    const out = try allocator.alloc(f64, values.len);
    for (values, 0..) |v, i| out[i] = decimal(v, digits);
    return out;
}

pub fn changeRate(base: f64, current: f64) f64 {
    return current / base;
}

pub fn netChangeRate(base: f64, current: f64) f64 {
    return (changeRate(base, current) - 1.0) * 100.0;
}

pub fn zeros(allocator: std.mem.Allocator, length: usize) ![]f64 {
    const out = try allocator.alloc(f64, length);
    @memset(out, 0);
    return out;
}

pub fn ones(allocator: std.mem.Allocator, length: usize) ![]f64 {
    const out = try allocator.alloc(f64, length);
    @memset(out, 1);
    return out;
}

pub fn linspace(allocator: std.mem.Allocator, start: f64, stop: f64, count: usize) ![]f64 {
    if (count == 0) return allocator.alloc(f64, 0);
    const out = try allocator.alloc(f64, count);
    if (count == 1) {
        out[0] = start;
        return out;
    }
    const step = (stop - start) / @as(f64, @floatFromInt(count - 1));
    for (0..count) |i| out[i] = start + step * @as(f64, @floatFromInt(i));
    return out;
}
