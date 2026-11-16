// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 参数本体: 动作、取值形态、取值、缺省值、转换器、参数定义与命名空间。
//!
//! 语义以 Python spec `quant1x/base/argparser/spec.py` 为准; 本文件的字符串与取值都由调用方给的
//! 分配器持有, 解析器内部统一用自身的 arena, 因此结果的有效期与解析器一致。

const std = @import("std");

/// 参数出现时怎么取值、怎么落进命名空间, 对应 Python 的 `Action`。
pub const Action = enum {
    store,
    store_true,
    store_false,
    store_const,
    append,
    append_const,
    extend,
    count,
    help,
    version,

    /// 动作名, 与 Python 的 `Action.value` 同形。
    pub fn name(self: Action) []const u8 {
        return @tagName(self);
    }

    /// 该动作是否从命令行吃掉取值。
    pub fn takes_values(self: Action) bool {
        return switch (self) {
            .store, .append, .extend => true,
            else => false,
        };
    }
};

/// 把动作名归一成 `Action`, 兼容 clap 的 `SetTrue` 一类拼法; 无法识别报 `error.InvalidAction`。
pub fn coerce_action(allocator: std.mem.Allocator, text: []const u8) !Action {
    var key = std.ArrayList(u8).empty;
    for (text) |item| {
        if (item == '_' or item == '-') {
            continue;
        }
        try key.append(allocator, std.ascii.toLower(item));
    }
    defer key.deinit(allocator);
    const body = key.items;
    if (std.mem.eql(u8, body, "store") or std.mem.eql(u8, body, "set")) return .store;
    if (std.mem.eql(u8, body, "storetrue") or std.mem.eql(u8, body, "settrue")) return .store_true;
    if (std.mem.eql(u8, body, "storefalse") or std.mem.eql(u8, body, "setfalse")) return .store_false;
    if (std.mem.eql(u8, body, "storeconst")) return .store_const;
    if (std.mem.eql(u8, body, "append")) return .append;
    if (std.mem.eql(u8, body, "appendconst")) return .append_const;
    if (std.mem.eql(u8, body, "extend")) return .extend;
    if (std.mem.eql(u8, body, "count")) return .count;
    if (std.mem.eql(u8, body, "help")) return .help;
    if (std.mem.eql(u8, body, "version")) return .version;
    return error.InvalidAction;
}

/// 取值形态的种类。
pub const NargsKind = enum { one, fixed, optional, zero_or_more, one_or_more, remainder };

/// 取值形态; 零值是一个取值。
pub const Nargs = struct {
    kind: NargsKind = .one,
    count: usize = 0,

    /// 一个取值。
    pub fn one() Nargs {
        return .{};
    }

    /// 恰好 `value` 个取值。
    pub fn fixed(value: usize) Nargs {
        return .{ .kind = .fixed, .count = value };
    }

    /// `?`: 最多一个取值。
    pub fn optional() Nargs {
        return .{ .kind = .optional };
    }

    /// `*`: 任意个取值。
    pub fn zero_or_more() Nargs {
        return .{ .kind = .zero_or_more };
    }

    /// `+`: 至少一个取值。
    pub fn one_or_more() Nargs {
        return .{ .kind = .one_or_more };
    }

    /// 其余 token 全部收下, 对应 `argparse.REMAINDER`。
    pub fn remainder() Nargs {
        return .{ .kind = .remainder };
    }

    /// 形态名, 与 Python 的 `nargs` 写法同形。
    pub fn kind_name(self: Nargs, allocator: std.mem.Allocator) ![]const u8 {
        return switch (self.kind) {
            .one => "",
            .fixed => try std.fmt.allocPrint(allocator, "{d}", .{self.count}),
            .optional => "?",
            .zero_or_more => "*",
            .one_or_more => "+",
            .remainder => "...",
        };
    }

    /// 最少吃几个取值。
    pub fn min_count(self: Nargs) usize {
        return switch (self.kind) {
            .fixed => self.count,
            .optional, .zero_or_more, .remainder => 0,
            .one, .one_or_more => 1,
        };
    }

    /// 最多吃几个取值; `null` 表示无上限。
    pub fn max_count(self: Nargs) ?usize {
        return switch (self.kind) {
            .fixed => self.count,
            .optional, .one => 1,
            .zero_or_more, .one_or_more, .remainder => null,
        };
    }

    /// 取值是否恒以列表形态落进命名空间。
    pub fn collects(self: Nargs) bool {
        return switch (self.kind) {
            .fixed, .zero_or_more, .one_or_more, .remainder => true,
            .one, .optional => false,
        };
    }

    /// 是否 REMAINDER 形态。
    pub fn is_remainder(self: Nargs) bool {
        return self.kind == .remainder;
    }
};

/// 解析结果里的一个取值, 对应 Python 侧的任意对象。
pub const Value = union(enum) {
    nil,
    boolean: bool,
    integer: i64,
    float: f64,
    string: []const u8,
    list: []const Value,

    /// 文本形态: 浮点补 `.0`、`inf` / `nan` 小写、布尔 `true` / `false`、列表 `a, b`。
    pub fn text(self: Value, allocator: std.mem.Allocator) ![]const u8 {
        return switch (self) {
            .nil => "None",
            .boolean => |flag| if (flag) "true" else "false",
            .integer => |number| try std.fmt.allocPrint(allocator, "{d}", .{number}),
            .float => |number| try float_text(allocator, number),
            .string => |body| body,
            .list => |items| blk: {
                var body = std.ArrayList(u8).empty;
                for (items, 0..) |item, index| {
                    if (index != 0) try body.appendSlice(allocator, ", ");
                    try body.appendSlice(allocator, try item.text(allocator));
                }
                break :blk try body.toOwnedSlice(allocator);
            },
        };
    }

    /// 是否空值。
    pub fn is_nil(self: Value) bool {
        return std.meta.activeTag(self) == .nil;
    }
};

/// 浮点的文本形态, 与 Python `str(float)` 同形。
pub fn float_text(allocator: std.mem.Allocator, value: f64) ![]const u8 {
    if (std.math.isNan(value)) return "nan";
    if (std.math.isInf(value)) return if (value > 0) "inf" else "-inf";
    const body = try std.fmt.allocPrint(allocator, "{d}", .{value});
    if (std.mem.indexOfAny(u8, body, ".eE") != null) return body;
    return std.fmt.allocPrint(allocator, "{s}.0", .{body});
}

/// 缺省值来源: 没给 / 不写进命名空间 / 具体取值。
pub const DefaultValue = union(enum) {
    missing,
    suppress,
    value: Value,

    /// 是否没给默认值。
    pub fn is_missing(self: DefaultValue) bool {
        return std.meta.activeTag(self) == .missing;
    }

    /// 是否 `SUPPRESS`。
    pub fn is_suppress(self: DefaultValue) bool {
        return std.meta.activeTag(self) == .suppress;
    }

    /// 默认值本体; 没给或 `SUPPRESS` 时为 `null`。
    pub fn get(self: DefaultValue) ?Value {
        return switch (self) {
            .value => |item| item,
            else => null,
        };
    }

    /// 文本形态。
    pub fn text(self: DefaultValue, allocator: std.mem.Allocator) ![]const u8 {
        return switch (self) {
            .missing => "MISSING",
            .suppress => "SUPPRESS",
            .value => |item| try item.text(allocator),
        };
    }
};

/// 一个可选值及其说明。
pub const Choice = struct {
    value: Value,
    help: []const u8 = "",
};

/// 取值转换器: 把命令行上的字符串翻成取值。
///
/// 失败时把原因写进 `message` 并返回 `error.InvalidValue`; `message` 由调用方给的分配器持有。
pub const ValueParser = union(enum) {
    int: struct { minimum: ?i64 = null, maximum: ?i64 = null },
    float: struct { minimum: ?f64 = null, maximum: ?f64 = null },
    boolean,
    custom: struct {
        context: *anyopaque,
        convert: *const fn (
            context: *anyopaque,
            allocator: std.mem.Allocator,
            text: []const u8,
            message: *[]const u8,
        ) anyerror!Value,
    },

    /// 类型名, 只用于文案。
    pub fn type_name(self: ValueParser) []const u8 {
        return switch (self) {
            .int => "INTEGER",
            .float => "FLOAT",
            .boolean => "BOOL",
            .custom => "VALUE",
        };
    }

    /// 转换一个取值。
    pub fn parse(
        self: ValueParser,
        allocator: std.mem.Allocator,
        text: []const u8,
        message: *[]const u8,
    ) !Value {
        return switch (self) {
            .int => |bounds| parse_int(allocator, text, bounds.minimum, bounds.maximum, message),
            .float => |bounds| parse_float(allocator, text, bounds.minimum, bounds.maximum, message),
            .boolean => parse_bool_value(text, message),
            .custom => |item| item.convert(item.context, allocator, text, message),
        };
    }
};

/// 十进制整数转换器, 可带闭区间上下界。
pub fn parse_int(allocator: std.mem.Allocator, text: []const u8, minimum: ?i64, maximum: ?i64, message: *[]const u8) !Value {
    const value = std.fmt.parseInt(i64, text, 10) catch {
        message.* = "invalid digit found in string";
        return error.InvalidValue;
    };
    const range = try int_range_text(allocator, minimum, maximum);
    if (minimum) |low| {
        if (value < low) {
            message.* = try std.fmt.allocPrint(allocator, "{d} is not in {s}", .{ value, range });
            return error.InvalidValue;
        }
    }
    if (maximum) |high| {
        if (value > high) {
            message.* = try std.fmt.allocPrint(allocator, "{d} is not in {s}", .{ value, range });
            return error.InvalidValue;
        }
    }
    return Value{ .integer = value };
}

/// 浮点转换器, 可带闭区间上下界。
pub fn parse_float(allocator: std.mem.Allocator, text: []const u8, minimum: ?f64, maximum: ?f64, message: *[]const u8) !Value {
    const value = std.fmt.parseFloat(f64, text) catch {
        message.* = "invalid float literal";
        return error.InvalidValue;
    };
    const range = try float_range_text(allocator, minimum, maximum);
    if (minimum) |low| {
        if (value < low) {
            message.* = try std.fmt.allocPrint(allocator, "{s} is not in {s}", .{ try float_text(allocator, value), range });
            return error.InvalidValue;
        }
    }
    if (maximum) |high| {
        if (value > high) {
            message.* = try std.fmt.allocPrint(allocator, "{s} is not in {s}", .{ try float_text(allocator, value), range });
            return error.InvalidValue;
        }
    }
    return Value{ .float = value };
}

/// 布尔转换器: `true/false`、`yes/no`、`on/off`、`1/0`, 大小写不敏感。
pub fn parse_bool_value(text: []const u8, message: *[]const u8) !Value {
    const body = std.mem.trim(u8, text, " \t\r\n");
    const words = [_][]const u8{ "true", "yes", "on", "1" };
    for (words) |word| {
        if (std.ascii.eqlIgnoreCase(body, word)) return Value{ .boolean = true };
    }
    const falsy = [_][]const u8{ "false", "no", "off", "0" };
    for (falsy) |word| {
        if (std.ascii.eqlIgnoreCase(body, word)) return Value{ .boolean = false };
    }
    message.* = "provided string was not one of true/false/yes/no/on/off/1/0";
    return error.InvalidValue;
}

/// 整数上下界的开区间记法。
fn int_range_text(allocator: std.mem.Allocator, minimum: ?i64, maximum: ?i64) ![]const u8 {
    if (minimum == null and maximum == null) return "";
    if (minimum == null) return std.fmt.allocPrint(allocator, "..={d}", .{maximum.?});
    if (maximum == null) return std.fmt.allocPrint(allocator, "{d}..", .{minimum.?});
    return std.fmt.allocPrint(allocator, "{d}..={d}", .{ minimum.?, maximum.? });
}

/// 浮点上下界的开区间记法。
fn float_range_text(allocator: std.mem.Allocator, minimum: ?f64, maximum: ?f64) ![]const u8 {
    if (minimum == null and maximum == null) return "";
    if (minimum == null) return std.fmt.allocPrint(allocator, "..={s}", .{try float_text(allocator, maximum.?)});
    if (maximum == null) return std.fmt.allocPrint(allocator, "{s}..", .{try float_text(allocator, minimum.?)});
    return std.fmt.allocPrint(allocator, "{s}..={s}", .{
        try float_text(allocator, minimum.?),
        try float_text(allocator, maximum.?),
    });
}

/// 单个命令行参数的定义, 由 `Parser.add_argument` 登记。
///
/// 位置参数没有 `names`; 选项至少有一个以 `-` 开头的名字。
pub const Argument = struct {
    names: []const []const u8 = &.{},
    dest: []const u8 = "",
    action: Action = .store,
    nargs: Nargs = .{},
    /// 取值转换器; `type` 是 Zig 关键字, 故叫 `value_parser`。
    value_parser: ?ValueParser = null,
    choices: ?[]const Choice = null,
    default_value: DefaultValue = .missing,
    const_value: DefaultValue = .missing,
    required: bool = false,
    metavar: ?[]const u8 = null,
    help: ?[]const u8 = null,
    long_help: ?[]const u8 = null,
    env: ?[]const u8 = null,
    alias: []const []const u8 = &.{},
    visible_alias: []const []const u8 = &.{},
    conflicts_with: []const []const u8 = &.{},
    requires_args: []const []const u8 = &.{},
    /// 与之互相覆盖的参数 (名字或 `dest`); 一对里命令行上后出现的留下。
    overrides_with: []const []const u8 = &.{},
    /// 列出的参数里只要有一个出现过, 本参数就不算必填。
    required_unless_present: []const []const u8 = &.{},
    value_delimiter: ?[]const u8 = null,
    hide: bool = false,
    hide_default: bool = false,
    allow_hyphen_values: bool = false,
    global: bool = false,
    group: ?[]const u8 = null,

    /// 位置参数: 没有选项名, 按命令行顺序吃取值。
    pub fn is_positional(self: Argument) bool {
        return self.names.len == 0;
    }

    /// 是否吃取值; 开关类动作为假。
    pub fn takes_values(self: Argument) bool {
        return self.action.takes_values();
    }

    /// 取值是否恒以列表形态落进命名空间。
    pub fn collects(self: Argument) bool {
        return self.value_delimiter != null or self.nargs.collects();
    }

    /// 取值的显示名: `metavar` 优先, 其次大写化的 `dest`。
    pub fn value_name(self: Argument, allocator: std.mem.Allocator) ![]const u8 {
        if (self.metavar) |value| return value;
        return std.ascii.allocUpperString(allocator, self.dest);
    }

    /// 取值占位, 如 `<FILE>`。
    pub fn value_spec(self: Argument, allocator: std.mem.Allocator) ![]const u8 {
        return std.fmt.allocPrint(allocator, "<{s}>", .{try self.value_name(allocator)});
    }

    /// 指代该参数时优先用长选项; 只有短选项时退回它。
    pub fn primary_name(self: Argument) []const u8 {
        for (self.names) |name| {
            if (std.mem.startsWith(u8, name, "--")) return name;
        }
        if (self.names.len > 0) return self.names[0];
        return self.dest;
    }

    /// 错误文案里的指代, 如 `--output <FILE>` / `<FILE>` / `-v`。
    pub fn error_name(self: Argument, allocator: std.mem.Allocator) ![]const u8 {
        if (self.is_positional()) return self.value_spec(allocator);
        const name = self.primary_name();
        if (!self.takes_values()) return name;
        return std.fmt.allocPrint(allocator, "{s} {s}", .{ name, try self.value_spec(allocator) });
    }

    /// 帮助左列。
    pub fn help_names(self: Argument, allocator: std.mem.Allocator) ![]const u8 {
        if (self.is_positional()) return self.usage_spec(allocator);
        var names = std.ArrayList([]const u8).empty;
        try names.appendSlice(allocator, self.names);
        try names.appendSlice(allocator, self.visible_alias);
        var head = try format_names(allocator, names.items);
        if (self.action == .count) {
            head = try std.fmt.allocPrint(allocator, "{s}...", .{head});
        }
        if (!self.takes_values()) return head;
        return std.fmt.allocPrint(allocator, "{s} {s}", .{ head, try self.value_spec(allocator) });
    }

    /// usage 行里的位置参数占位。
    pub fn usage_spec(self: Argument, allocator: std.mem.Allocator) ![]const u8 {
        const name = try self.value_name(allocator);
        return switch (self.nargs.kind) {
            .optional => std.fmt.allocPrint(allocator, "[{s}]", .{name}),
            .zero_or_more, .remainder => std.fmt.allocPrint(allocator, "[{s}]...", .{name}),
            .one_or_more, .fixed => std.fmt.allocPrint(allocator, "<{s}>...", .{name}),
            .one => std.fmt.allocPrint(allocator, "<{s}>", .{name}),
        };
    }

    /// 可选值集合; 没配 `choices` 时为空。
    pub fn choice_values(self: Argument, allocator: std.mem.Allocator) ![]const Value {
        const choices = self.choices orelse return &.{};
        const values = try allocator.alloc(Value, choices.len);
        for (choices, 0..) |choice, index| {
            values[index] = choice.value;
        }
        return values;
    }
};

/// 解析结果的容器, 对应 `argparse.Namespace`; 保持首次写入的顺序。
pub const Namespace = struct {
    entries: std.ArrayList(Entry) = .empty,

    /// 一条落值。
    pub const Entry = struct {
        key: []const u8,
        value: Value,
    };

    /// 按名字读取; 未写入时为 `null`。
    pub fn get(self: Namespace, key: []const u8) ?Value {
        for (self.entries.items) |entry| {
            if (std.mem.eql(u8, entry.key, key)) return entry.value;
        }
        return null;
    }

    /// 该名字是否已写入。
    pub fn contains(self: Namespace, key: []const u8) bool {
        return self.get(key) != null;
    }

    /// 写入一个取值; 同名覆盖且位置不变。
    pub fn set(self: *Namespace, allocator: std.mem.Allocator, key: []const u8, value: Value) !void {
        for (self.entries.items) |*entry| {
            if (std.mem.eql(u8, entry.key, key)) {
                entry.value = value;
                return;
            }
        }
        try self.entries.append(allocator, .{ .key = key, .value = value });
    }

    /// 抹掉一个已写入的取值; 没写过时什么也不做。
    pub fn remove(self: *Namespace, key: []const u8) void {
        var kept: usize = 0;
        for (self.entries.items) |entry| {
            if (std.mem.eql(u8, entry.key, key)) continue;
            self.entries.items[kept] = entry;
            kept += 1;
        }
        self.entries.shrinkRetainingCapacity(kept);
    }

    /// 已写入的条目数。
    pub fn count(self: Namespace) usize {
        return self.entries.items.len;
    }

    /// 文本形态, 形如 `Namespace(k=v, ...)`。
    pub fn text(self: Namespace, allocator: std.mem.Allocator) ![]const u8 {
        var body = std.ArrayList(u8).empty;
        for (self.entries.items, 0..) |entry, index| {
            if (index != 0) try body.appendSlice(allocator, ", ");
            try body.appendSlice(allocator, entry.key);
            try body.append(allocator, '=');
            try body.appendSlice(allocator, try entry.value.text(allocator));
        }
        return std.fmt.allocPrint(allocator, "Namespace({s})", .{body.items});
    }
};

/// 把取值拼成 `a, b, c` 形态。
pub fn format_items(allocator: std.mem.Allocator, values: []const Value) ![]const u8 {
    var body = std.ArrayList(u8).empty;
    for (values, 0..) |value, index| {
        if (index != 0) try body.appendSlice(allocator, ", ");
        try body.appendSlice(allocator, try value.text(allocator));
    }
    return body.toOwnedSlice(allocator);
}

/// 把名字拼成 `a, b, c` 形态。
pub fn format_names(allocator: std.mem.Allocator, names: []const []const u8) ![]const u8 {
    var body = std.ArrayList(u8).empty;
    for (names, 0..) |name, index| {
        if (index != 0) try body.appendSlice(allocator, ", ");
        try body.appendSlice(allocator, name);
    }
    return body.toOwnedSlice(allocator);
}
