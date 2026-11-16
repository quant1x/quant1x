// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 版式: usage 行、分节帮助、错误块与 ANSI 着色。
//!
//! 版式固定, 不随内容猜测: 章节顺序是 `Usage` → `Commands` → `Arguments` → `Options` → 自定义分组
//! → `epilog`; 左列超过半幅时说明文字换到下一行并对齐缩进。颜色只加在标题与选项字面量上,
//! `[default: ...]` / `[env: ...]` / `[possible values: ...]` 一律保留原样, 便于用例按文本断言。
//!
//! 宽度按码点计 (与 Python 的 `len()` 同口径), 因此中文说明与英文说明的折行位置一致。

const std = @import("std");
const spec = @import("spec.zig");
const err = @import("error.zig");
const parser_mod = @import("parser.zig");

const Parser = parser_mod.Parser;
const Argument = spec.Argument;

/// ANSI 着色开关; 关闭时所有方法原样返回文本, 因此版式与宽度计算只看可见字符。
pub const Style = struct {
    enabled: bool = false,

    /// 套上 ANSI 颜色码; 关闭着色或空串时原样返回。
    fn wrap(self: Style, allocator: std.mem.Allocator, code: []const u8, text: []const u8) ![]const u8 {
        if (!self.enabled or text.len == 0) return text;
        return std.fmt.allocPrint(allocator, "{s}{s}\x1b[0m", .{ code, text });
    }

    /// 章节标题与 `Usage:` / `error:` 前缀。
    pub fn heading(self: Style, allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
        return self.wrap(allocator, "\x1b[1m\x1b[4m", text);
    }

    /// 强调。
    pub fn bold(self: Style, allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
        return self.wrap(allocator, "\x1b[1m", text);
    }

    /// 选项与命令字面量。
    pub fn literal(self: Style, allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
        return self.wrap(allocator, "\x1b[32m", text);
    }

    /// `error:` 前缀; `error` 是关键字, 故叫 `error_text`。
    pub fn error_text(self: Style, allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
        return self.wrap(allocator, "\x1b[31m", text);
    }

    /// `tip:` 前缀。
    pub fn warning(self: Style, allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
        return self.wrap(allocator, "\x1b[33m", text);
    }
};

/// 表格行: 左列 (选项 / 命令 / 位置参数) 与右列说明。
const Row = struct {
    left: []const u8,
    text: []const u8,
};

/// 可见宽度: 码点个数, 与 Python 的 `len()` 同口径。
fn width_of(text: []const u8) usize {
    return std.unicode.utf8CountCodepoints(text) catch text.len;
}

/// `n` 个空格; 折行缩进用。
fn spaces(allocator: std.mem.Allocator, n: usize) ![]const u8 {
    const body = try allocator.alloc(u8, n);
    @memset(body, ' ');
    return body;
}

/// 单段文字按宽度折行; `width` 含缩进在内的整行宽度。
fn wrap_one(
    allocator: std.mem.Allocator,
    paragraph: []const u8,
    width: usize,
    initial_indent: []const u8,
    subsequent_indent: []const u8,
) ![]const []const u8 {
    var lines = std.ArrayList([]const u8).empty;
    var line = std.ArrayList(u8).empty;
    try line.appendSlice(allocator, initial_indent);
    var words = std.mem.tokenizeAny(u8, paragraph, " \t\r\n\x0b\x0c");
    var filled = false;
    while (words.next()) |word| {
        if (filled and width_of(line.items) + 1 + width_of(word) > width) {
            try lines.append(allocator, try line.toOwnedSlice(allocator));
            line = std.ArrayList(u8).empty;
            try line.appendSlice(allocator, subsequent_indent);
            filled = false;
        }
        if (filled) try line.append(allocator, ' ');
        try line.appendSlice(allocator, word);
        filled = true;
    }
    if (filled) {
        try lines.append(allocator, try line.toOwnedSlice(allocator));
    } else {
        try lines.append(allocator, "");
    }
    return lines.toOwnedSlice(allocator);
}

/// 按段落换行; 段间保留一个空行位。
fn wrap_paragraphs(
    allocator: std.mem.Allocator,
    text: []const u8,
    width: usize,
    indent: []const u8,
    first_indent: ?[]const u8,
) ![]const []const u8 {
    var lines = std.ArrayList([]const u8).empty;
    var paragraphs = std.mem.splitScalar(u8, text, '\n');
    var index: usize = 0;
    while (paragraphs.next()) |paragraph| : (index += 1) {
        const head = if (index == 0) (first_indent orelse indent) else indent;
        try lines.appendSlice(allocator, try wrap_one(allocator, paragraph, @max(width, 20), head, indent));
    }
    return lines.toOwnedSlice(allocator);
}

/// 取值是否同一个; 可选值的说明按它回查。
fn same_value(left: spec.Value, right: spec.Value) bool {
    if (std.meta.activeTag(left) != std.meta.activeTag(right)) return false;
    return switch (left) {
        .nil => true,
        .boolean => |value| value == right.boolean,
        .integer => |value| value == right.integer,
        .float => |value| value == right.float,
        .string => |value| std.mem.eql(u8, value, right.string),
        .list => |items| blk: {
            if (items.len != right.list.len) break :blk false;
            for (items, right.list) |item, other| {
                if (!same_value(item, other)) break :blk false;
            }
            break :blk true;
        },
    };
}

/// 某个可选值的说明; 没配说明时为 `null`。
fn choice_help(argument: *Argument, value: spec.Value) ?[]const u8 {
    const choices = argument.choices orelse return null;
    for (choices) |choice| {
        if (!same_value(choice.value, value)) continue;
        if (choice.help.len == 0) return null;
        return choice.help;
    }
    return null;
}

/// 一行说明文字: 帮助正文 + 默认值 / 环境变量 / 可选值。
fn argument_text(allocator: std.mem.Allocator, argument: *Argument, long: bool) ![]const u8 {
    var text: []const u8 = "";
    if (long) {
        if (argument.long_help) |value| {
            if (value.len != 0) text = value;
        }
    }
    if (text.len == 0) {
        if (argument.help) |value| text = value;
    }
    var extras = std.ArrayList([]const u8).empty;
    const shows_default = switch (argument.action) {
        .store, .append, .extend => true,
        else => false,
    };
    if (shows_default and !argument.hide_default) {
        if (argument.default_value.get()) |value| {
            if (!value.is_nil()) {
                try extras.append(allocator, try std.fmt.allocPrint(allocator, "[default: {s}]", .{try value.text(allocator)}));
            }
        }
    }
    if (argument.env) |env| {
        try extras.append(allocator, try std.fmt.allocPrint(allocator, "[env: {s}]", .{env}));
    }
    const choices = try argument.choice_values(allocator);
    if (choices.len != 0) {
        try extras.append(allocator, try std.fmt.allocPrint(allocator, "[possible values: {s}]", .{
            try spec.format_items(allocator, choices),
        }));
    }
    if (extras.items.len != 0) {
        const joined = try std.mem.join(allocator, " ", extras.items);
        text = std.mem.trim(u8, try std.fmt.allocPrint(allocator, "{s} {s}", .{ text, joined }), " \t\r\n");
    }
    if (long) {
        var notes = std.ArrayList([]const u8).empty;
        for (choices) |value| {
            const note = choice_help(argument, value) orelse continue;
            try notes.append(allocator, try std.fmt.allocPrint(allocator, "- {s}: {s}", .{
                try value.text(allocator),
                note,
            }));
        }
        if (notes.items.len != 0) {
            const joined = try std.mem.join(allocator, "\n", notes.items);
            text = if (text.len == 0) joined else try std.fmt.allocPrint(allocator, "{s}\n{s}", .{ text, joined });
        }
    }
    return text;
}

/// 祖先命令上标记为 `global` 的参数: 子命令里同样可用, 也出现在子命令帮助里。
fn global_arguments(allocator: std.mem.Allocator, parser: *Parser) ![]const *Argument {
    var collected = std.ArrayList(*Argument).empty;
    var ancestor = parser.parent;
    while (ancestor) |parent| : (ancestor = parent.parent) {
        for (parent.arguments.items) |argument| {
            if (argument.global) try collected.append(allocator, argument);
        }
    }
    std.mem.reverse(*Argument, collected.items);
    return collected.toOwnedSlice(allocator);
}

/// 本命令的选项 + 祖先命令下沉的 `global` 参数; 帮助 / 版本排最后 (与 clap 一致)。
fn visible_options(allocator: std.mem.Allocator, parser: *Parser) ![]const *Argument {
    var options = std.ArrayList(*Argument).empty;
    for (parser.arguments.items) |argument| {
        if (!argument.is_positional() and !argument.hide) try options.append(allocator, argument);
    }
    for (try global_arguments(allocator, parser)) |argument| {
        if (!argument.hide) try options.append(allocator, argument);
    }
    var ordered = std.ArrayList(*Argument).empty;
    for (options.items) |argument| {
        if (argument.action != .help and argument.action != .version) try ordered.append(allocator, argument);
    }
    for (options.items) |argument| {
        if (argument.action == .help or argument.action == .version) try ordered.append(allocator, argument);
    }
    return ordered.toOwnedSlice(allocator);
}

/// `标题 + 两列表格`: 左列对齐, 右列折行并与右列首字符对齐。
fn table(
    allocator: std.mem.Allocator,
    title: []const u8,
    rows: []const Row,
    width: usize,
    style: Style,
    description: ?[]const u8,
) ![]const u8 {
    if (rows.len == 0) return "";
    var widest: usize = 0;
    for (rows) |row| widest = @max(widest, width_of(row.left));
    const left_width = @min(widest, @max(@as(usize, 16), width / 3));
    const gutter: usize = 2;
    const base = 2 + left_width + gutter;
    var lines = std.ArrayList([]const u8).empty;
    try lines.append(allocator, title);
    if (description) |body| {
        if (body.len != 0) {
            try lines.appendSlice(allocator, try wrap_paragraphs(allocator, body, width, "  ", "  "));
            try lines.append(allocator, "");
        }
    }
    for (rows) |row| {
        const head = try style.literal(allocator, try std.fmt.allocPrint(allocator, "  {s}", .{row.left}));
        if (row.text.len == 0) {
            try lines.append(allocator, head);
            continue;
        }
        if (width_of(row.left) > left_width) {
            try lines.append(allocator, head);
            try lines.appendSlice(allocator, try wrap_paragraphs(allocator, row.text, width, "    ", null));
            continue;
        }
        const indent = try spaces(allocator, base);
        const chunks = try wrap_paragraphs(allocator, row.text, width, indent, indent);
        const pad = try spaces(allocator, base - 2 - width_of(row.left));
        try lines.append(allocator, try std.fmt.allocPrint(allocator, "{s}{s}{s}", .{
            head,
            pad,
            std.mem.trimStart(u8, chunks[0], " "),
        }));
        if (chunks.len > 1) try lines.appendSlice(allocator, chunks[1..]);
    }
    return std.mem.join(allocator, "\n", lines.items);
}

/// 子命令章节的行: 名字 + 展示别名; 自动生成的 `help` 排在最后。
fn command_rows(allocator: std.mem.Allocator, parser: *Parser) ![]const Row {
    const subparsers = parser.subparsers orelse return &.{};
    var rows = std.ArrayList(Row).empty;
    for (subparsers.commands.items) |command| {
        var head: []const u8 = command.name;
        if (command.visible_aliases.len != 0) {
            head = try std.fmt.allocPrint(allocator, "{s}, {s}", .{
                command.name,
                try spec.format_names(allocator, command.visible_aliases),
            });
        }
        try rows.append(allocator, .{ .left = head, .text = command.parser.about orelse "" });
    }
    if (!subparsers.has_command("help")) {
        try rows.append(allocator, .{
            .left = "help",
            .text = "Print this message or the help of the given subcommand(s)",
        });
    }
    return rows.toOwnedSlice(allocator);
}

/// 单行用法文本。
pub fn render_usage(allocator: std.mem.Allocator, parser: *Parser) ![]const u8 {
    const style = Style{ .enabled = parser.color_enabled() };
    return std.fmt.allocPrint(allocator, "{s} {s}", .{
        try style.heading(allocator, "Usage:"),
        try parser.usage_spec(),
    });
}

/// 整段帮助文本, 段间空一行。
pub fn render_help(allocator: std.mem.Allocator, parser: *Parser, long: bool) ![]const u8 {
    const style = Style{ .enabled = parser.color_enabled() };
    const width: usize = @intCast(parser.width());
    var blocks = std.ArrayList([]const u8).empty;
    const about = if (long) (parser.long_about orelse parser.about) else parser.about;
    if (about) |body| {
        if (body.len != 0) {
            try blocks.append(allocator, try std.mem.join(
                allocator,
                "\n",
                try wrap_paragraphs(allocator, body, width, "", null),
            ));
        }
    }
    try blocks.append(allocator, try render_usage(allocator, parser));
    if (parser.subparsers) |subparsers| {
        const title = try style.heading(allocator, try std.fmt.allocPrint(allocator, "{s}:", .{subparsers.title}));
        const commands = try table(allocator, title, try command_rows(allocator, parser), width, style, null);
        if (commands.len != 0) try blocks.append(allocator, commands);
    }
    var positionals = std.ArrayList(Row).empty;
    for (parser.positionals.items) |argument| {
        if (argument.hide) continue;
        try positionals.append(allocator, .{
            .left = try argument.help_names(allocator),
            .text = try argument_text(allocator, argument, long),
        });
    }
    if (positionals.items.len != 0) {
        try blocks.append(allocator, try table(
            allocator,
            try style.heading(allocator, "Arguments:"),
            positionals.items,
            width,
            style,
            null,
        ));
    }
    const options = try visible_options(allocator, parser);
    var default_options = std.ArrayList(Row).empty;
    for (options) |argument| {
        if (argument.group) |group| {
            if (has_group(parser, group)) continue;
        }
        try default_options.append(allocator, .{
            .left = try argument.help_names(allocator),
            .text = try argument_text(allocator, argument, long),
        });
    }
    if (default_options.items.len != 0) {
        try blocks.append(allocator, try table(
            allocator,
            try style.heading(allocator, "Options:"),
            default_options.items,
            width,
            style,
            null,
        ));
    }
    for (parser.groups.items) |group| {
        var grouped = std.ArrayList(Row).empty;
        for (options) |argument| {
            const title = argument.group orelse continue;
            if (!std.mem.eql(u8, title, group.title)) continue;
            try grouped.append(allocator, .{
                .left = try argument.help_names(allocator),
                .text = try argument_text(allocator, argument, long),
            });
        }
        if (grouped.items.len == 0) continue;
        try blocks.append(allocator, try table(
            allocator,
            try style.heading(allocator, try std.fmt.allocPrint(allocator, "{s}:", .{group.title})),
            grouped.items,
            width,
            style,
            group.description,
        ));
    }
    if (parser.epilog) |body| {
        if (body.len != 0) {
            try blocks.append(allocator, try std.mem.join(
                allocator,
                "\n",
                try wrap_paragraphs(allocator, body, width, "", null),
            ));
        }
    }
    return std.mem.join(allocator, "\n\n", blocks.items);
}

/// 该标题是否登记成了本命令的分组。
fn has_group(parser: *Parser, title: []const u8) bool {
    for (parser.groups.items) |group| {
        if (std.mem.eql(u8, group.title, title)) return true;
    }
    return false;
}

/// 错误块: 首行 `error:` + 缩进的细节 / 建议 + usage + 帮助提示。
pub fn render_error(allocator: std.mem.Allocator, failure: err.UsageError, color: bool) ![]const u8 {
    if (!color) return failure.render(allocator);
    const style = Style{ .enabled = true };
    var body = std.ArrayList(u8).empty;
    try body.appendSlice(allocator, try style.error_text(allocator, "error:"));
    try body.append(allocator, ' ');
    try body.appendSlice(allocator, failure.message);
    if (failure.reason.len != 0) {
        try body.appendSlice(allocator, ": ");
        try body.appendSlice(allocator, failure.reason);
    }
    for (failure.lines) |line| {
        try body.appendSlice(allocator, "\n  ");
        try body.appendSlice(allocator, line);
    }
    if (failure.tips.len != 0) {
        try body.append(allocator, '\n');
        for (failure.tips) |tip| {
            try body.appendSlice(allocator, "\n  ");
            try body.appendSlice(allocator, try style.warning(allocator, "tip:"));
            try body.append(allocator, ' ');
            try body.appendSlice(allocator, tip);
        }
    }
    if (failure.usage.len != 0) {
        try body.appendSlice(allocator, "\n\n");
        try body.appendSlice(allocator, try style.heading(allocator, "Usage:"));
        try body.append(allocator, ' ');
        try body.appendSlice(allocator, failure.usage);
    }
    if (failure.hint.len != 0) {
        try body.appendSlice(allocator, "\n\n");
        try body.appendSlice(allocator, failure.hint);
    }
    return body.toOwnedSlice(allocator);
}
