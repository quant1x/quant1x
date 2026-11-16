// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 解析结果与控制流: 用法错误、帮助 / 版本请求、定义期错误, 以及相似项建议。
//!
//! 三类错误的分工:
//!
//!   - `usage` 命令行输入不合法 (未知参数、缺值、取值非法、校验未通过), 进程退出码 2;
//!   - `help` / `version` 请求打印帮助 / 版本, 不算错误, 退出码 0;
//!   - `definition` 定义期错误 (命令链 dest 重复、引用未登记的参数), 退出码 1。

const std = @import("std");
const spec = @import("spec.zig");

/// 用法错误的类别, 对应 clap 的 `ErrorKind`。
pub const ErrorKind = enum {
    other,
    unknown_argument,
    ambiguous_argument,
    missing_value,
    invalid_value,
    missing_required,
    conflict,

    /// 类别名, 与 Python 的 `ErrorKind.value` 同形。
    pub fn name(self: ErrorKind) []const u8 {
        return @tagName(self);
    }
};

/// 命令行输入不合法。
pub const UsageError = struct {
    message: []const u8,
    kind: ErrorKind = .other,
    reason: []const u8 = "",
    lines: []const []const u8 = &.{},
    tips: []const []const u8 = &.{},
    usage: []const u8 = "",
    hint: []const u8 = "",

    /// 用法错误的退出码。
    pub fn exit_code(self: UsageError) u8 {
        _ = self;
        return 2;
    }

    /// 非着色版式: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示。
    pub fn render(self: UsageError, allocator: std.mem.Allocator) ![]const u8 {
        var body = std.ArrayList(u8).empty;
        try body.appendSlice(allocator, "error: ");
        try body.appendSlice(allocator, self.message);
        if (self.reason.len != 0) {
            try body.appendSlice(allocator, ": ");
            try body.appendSlice(allocator, self.reason);
        }
        for (self.lines) |line| {
            try body.appendSlice(allocator, "\n  ");
            try body.appendSlice(allocator, line);
        }
        if (self.tips.len != 0) {
            try body.append(allocator, '\n');
            for (self.tips) |tip| {
                try body.appendSlice(allocator, "\n  tip: ");
                try body.appendSlice(allocator, tip);
            }
        }
        if (self.usage.len != 0) {
            try body.appendSlice(allocator, "\n\nUsage: ");
            try body.appendSlice(allocator, self.usage);
        }
        if (self.hint.len != 0) {
            try body.appendSlice(allocator, "\n\n");
            try body.appendSlice(allocator, self.hint);
        }
        return body.toOwnedSlice(allocator);
    }
};

/// `-h` / `--help`: 请求帮助; `long_form` 为真表示走长帮助 (`--help`)。
pub const HelpRequested = struct {
    long_form: bool = false,
    prog: []const u8,
};

/// `-V` / `--version`: 请求打印版本。
pub const VersionRequested = struct {
    text: []const u8,
};

/// 定义期错误, 对应 Python 侧直接抛出的 `ValueError`, 不属于用法错误。
pub const DefinitionError = struct {
    message: []const u8,
};

/// 解析结果: 成功给出命名空间, 失败给出错误或控制流请求。
pub const Outcome = union(enum) {
    ok: spec.Namespace,
    usage: UsageError,
    help: HelpRequested,
    version: VersionRequested,
    definition: DefinitionError,

    /// 取错误对应的进程退出码: 成功 0, 用法错误 2, 定义期错误 1。
    pub fn exit_code(self: Outcome) u8 {
        return switch (self) {
            .ok, .help, .version => 0,
            .usage => 2,
            .definition => 1,
        };
    }

    /// 渲染文本; 成功解析时为空。
    pub fn render(self: Outcome, allocator: std.mem.Allocator) ![]const u8 {
        return switch (self) {
            .ok => "",
            .usage => |failure| try failure.render(allocator),
            .help => |requested| if (requested.long_form) "--help" else "-h",
            .version => |requested| requested.text,
            .definition => |failure| failure.message,
        };
    }
};

/// 把 UTF-8 文本拆成码点; 非法字节按单字节处理。
fn codepoints(allocator: std.mem.Allocator, text: []const u8) ![]u32 {
    var points = std.ArrayList(u32).empty;
    var index: usize = 0;
    while (index < text.len) {
        const width = std.unicode.utf8ByteSequenceLength(text[index]) catch 1;
        if (index + width > text.len) {
            try points.append(allocator, text[index]);
            index += 1;
            continue;
        }
        const value = std.unicode.utf8Decode(text[index .. index + width]) catch @as(u32, text[index]);
        try points.append(allocator, value);
        index += width;
    }
    return points.toOwnedSlice(allocator);
}

/// Jaro 相似度, 匹配窗口为 `max(长度) / 2 - 1`。
pub fn jaro(allocator: std.mem.Allocator, left: []const u8, right: []const u8) !f64 {
    if (std.mem.eql(u8, left, right)) return 1.0;
    const left_points = try codepoints(allocator, left);
    defer allocator.free(left_points);
    const right_points = try codepoints(allocator, right);
    defer allocator.free(right_points);
    if (left_points.len == 0 or right_points.len == 0) return 0.0;
    const longest = @max(left_points.len, right_points.len);
    const window: usize = if (longest / 2 > 0) longest / 2 - 1 else 0;
    const left_hit = try allocator.alloc(bool, left_points.len);
    defer allocator.free(left_hit);
    const right_hit = try allocator.alloc(bool, right_points.len);
    defer allocator.free(right_hit);
    @memset(left_hit, false);
    @memset(right_hit, false);
    var matches: usize = 0;
    for (left_points, 0..) |point, index| {
        const start = if (index > window) index - window else 0;
        const stop = @min(index + window + 1, right_points.len);
        var position = start;
        while (position < stop) : (position += 1) {
            if (!right_hit[position] and right_points[position] == point) {
                left_hit[index] = true;
                right_hit[position] = true;
                matches += 1;
                break;
            }
        }
    }
    if (matches == 0) return 0.0;
    var transpositions: usize = 0;
    var cursor: usize = 0;
    for (left_points, 0..) |point, index| {
        if (!left_hit[index]) continue;
        while (!right_hit[cursor]) : (cursor += 1) {}
        if (point != right_points[cursor]) transpositions += 1;
        cursor += 1;
    }
    transpositions /= 2;
    const match_size: f64 = @floatFromInt(matches);
    return (match_size / @as(f64, @floatFromInt(left_points.len)) +
        match_size / @as(f64, @floatFromInt(right_points.len)) +
        @as(f64, @floatFromInt(matches - transpositions)) / match_size) / 3.0;
}

/// Jaro-Winkler 相似度: 对相同前缀 (最多 4 字符) 加权。
pub fn jaro_winkler(allocator: std.mem.Allocator, left: []const u8, right: []const u8) !f64 {
    const score = try jaro(allocator, left, right);
    const left_points = try codepoints(allocator, left);
    defer allocator.free(left_points);
    const right_points = try codepoints(allocator, right);
    defer allocator.free(right_points);
    const shared = @min(left_points.len, right_points.len);
    var prefix: usize = 0;
    while (prefix < shared and prefix < 4 and left_points[prefix] == right_points[prefix]) : (prefix += 1) {}
    return score + @as(f64, @floatFromInt(prefix)) * 0.1 * (1.0 - score);
}

/// 相似度阈值, 与 clap 的 `did_you_mean` 一致 (Jaro-Winkler > 0.8)。
pub const match_threshold: f64 = 0.8;

/// 在候选里挑最像 `word` 的一个; 相似度不到阈值时为 `null` (宁缺勿滥)。
pub fn best_match(allocator: std.mem.Allocator, word: []const u8, candidates: []const []const u8) !?[]const u8 {
    return best_match_at(allocator, word, candidates, match_threshold);
}

/// 同 `best_match`, 阈值由调用方给。
pub fn best_match_at(
    allocator: std.mem.Allocator,
    word: []const u8,
    candidates: []const []const u8,
    threshold: f64,
) !?[]const u8 {
    var best: ?[]const u8 = null;
    var best_score = threshold;
    for (candidates) |candidate| {
        if (candidate.len == 0 or std.mem.eql(u8, candidate, word)) continue;
        const score = try jaro_winkler(allocator, word, candidate);
        if (score > best_score) {
            best = candidate;
            best_score = score;
        }
    }
    return best;
}
