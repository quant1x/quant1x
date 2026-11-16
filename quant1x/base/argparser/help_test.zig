// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 帮助与错误版式的用例: usage 组成、章节顺序、标注、折行与着色。
//!
//! 与 `help_test.py` 的同名用例逐条对应; 期望值只由 Python 侧生成。

const std = @import("std");
const spec = @import("spec.zig");
const err = @import("error.zig");
const argparser = @import("parser.zig");
const help = @import("help.zig");

const testing = std.testing;

/// 取某个章节标题之后、下一个空行之前的行。
fn section(allocator: std.mem.Allocator, text: []const u8, title: []const u8) ![]const []const u8 {
    var body = std.ArrayList([]const u8).empty;
    var lines = std.mem.splitScalar(u8, text, '\n');
    var found = false;
    while (lines.next()) |line| {
        if (!found) {
            if (std.mem.eql(u8, line, title)) found = true;
            continue;
        }
        if (std.mem.trim(u8, line, " \t").len == 0) break;
        try body.append(allocator, line);
    }
    return body.toOwnedSlice(allocator);
}

/// 码点个数, 与 Python 的 `len()` 同口径。
fn width_of(text: []const u8) usize {
    return std.unicode.utf8CountCodepoints(text) catch text.len;
}

/// 子串出现次数。
fn count_of(text: []const u8, needle: []const u8) usize {
    var total: usize = 0;
    var start: usize = 0;
    while (std.mem.indexOfPos(u8, text, start, needle)) |found| {
        total += 1;
        start = found + needle.len;
    }
    return total;
}

/// 断言解析失败并取错误文本。
fn usage_text(parser: *argparser.Parser, argv: []const []const u8) ![]const u8 {
    const outcome = try parser.parse(argv);
    return switch (outcome) {
        .usage => |failure| try failure.render(parser.mem()),
        else => error.TestExpectedFailure,
    };
}

/// 去掉 ANSI 转义, 便于比较可见版式。
fn plain(allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
    var body = std.ArrayList(u8).empty;
    var index: usize = 0;
    while (index < text.len) {
        if (text[index] == 0x1b and index + 1 < text.len and text[index + 1] == '[') {
            index += 2;
            while (index < text.len and text[index] != 'm') index += 1;
            index += 1;
            continue;
        }
        try body.append(allocator, text[index]);
        index += 1;
    }
    return body.toOwnedSlice(allocator);
}

test "帮助: usage 的组成" {
    const bare = try argparser.Parser.init(testing.allocator, "bare");
    defer bare.deinit();
    _ = bare.with_color(.never);
    try testing.expectEqualStrings("bare", try bare.usage_spec());

    const opt = try argparser.Parser.init(testing.allocator, "opt");
    defer opt.deinit();
    _ = opt.with_color(.never);
    _ = try opt.add_argument(&.{ "-a", "--all" }, .{ .action = .store_true, .help = "全部" });
    _ = try opt.add_argument(&.{"--out"}, .{ .required = true, .help = "输出" });
    _ = try opt.add_argument(&.{"files"}, .{ .nargs = spec.Nargs.one_or_more(), .help = "文件" });
    try testing.expectEqualStrings("opt --out <OUT> [OPTIONS] <FILES>...", try opt.usage_spec());

    const cmd = try argparser.Parser.init(testing.allocator, "cmd");
    defer cmd.deinit();
    _ = cmd.with_color(.never);
    _ = try cmd.add_subparsers("", "", true);
    try testing.expectEqualStrings("cmd <COMMAND>", try cmd.usage_spec());

    const optional = try argparser.Parser.init(testing.allocator, "cmd2");
    defer optional.deinit();
    _ = optional.with_color(.never);
    _ = try optional.add_subparsers("", "", false);
    try testing.expectEqualStrings("cmd2 [COMMAND]", try optional.usage_spec());

    const own = try argparser.Parser.init(testing.allocator, "own");
    defer own.deinit();
    _ = try own.with_usage("own <自定义>");
    _ = own.with_color(.never);
    _ = try own.add_argument(&.{"--x"}, .{ .help = "忽略" });
    try testing.expectEqualStrings("own <自定义>", try own.usage_spec());
    try testing.expect(std.mem.startsWith(u8, try own.format_usage(), "Usage: own <自定义>"));
}

test "帮助: 章节顺序与内容" {
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer arena.deinit();
    const scratch = arena.allocator();
    const parser = try argparser.Parser.init(testing.allocator, "tool");
    defer parser.deinit();
    _ = try parser.with_about("工具集");
    _ = try parser.with_long_about("工具集的完整说明, 只在 --help 里出现。");
    _ = try parser.with_epilog("示例: tool build -j 8");
    _ = try parser.with_version("1.0.0");
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{ "-v", "--verbose" }, .{ .action = .count, .help = "输出更详细" });
    _ = try parser.add_argument(&.{"--level"}, .{
        .choices = &.{
            .{ .value = .{ .string = "debug" } },
            .{ .value = .{ .string = "info" } },
            .{ .value = .{ .string = "warn" } },
        },
        .default_value = .{ .value = .{ .string = "info" } },
        .help = "日志级别",
    });
    _ = try parser.add_argument(&.{"--jobs"}, .{
        .value_parser = .{ .int = .{} },
        .default_value = .{ .value = .{ .integer = 4 } },
        .env = "TOOL_JOBS",
        .help = "并发数",
    });
    _ = try parser.add_argument(&.{"--secret"}, .{ .hide = true, .help = "不展示" });
    _ = try parser.add_argument(&.{"--quiet"}, .{ .action = .store_true, .hide_default = true, .help = "安静" });
    _ = try parser.add_argument(&.{"src"}, .{ .nargs = spec.Nargs.zero_or_more(), .help = "源文件" });
    const subparsers = try parser.add_subparsers("command", "Commands", false);
    _ = try subparsers.add_parser("build", "构建", &.{}, &.{"b"});
    _ = try subparsers.add_parser("run", "运行", &.{}, &.{});
    const allocator = scratch;

    const text = try parser.format_help(false);
    try testing.expect(std.mem.startsWith(u8, text, "工具集\n"));
    const usage_at = std.mem.indexOf(u8, text, "Usage:").?;
    const commands_at = std.mem.indexOf(u8, text, "Commands:").?;
    const arguments_at = std.mem.indexOf(u8, text, "Arguments:").?;
    const options_at = std.mem.indexOf(u8, text, "Options:").?;
    try testing.expect(usage_at < commands_at and commands_at < arguments_at and arguments_at < options_at);
    try testing.expect(std.mem.endsWith(u8, std.mem.trimEnd(u8, text, " \n"), "示例: tool build -j 8"));
    try testing.expect(std.mem.indexOf(u8, text, "--secret") == null);
    try testing.expect(std.mem.indexOf(u8, text, "-v, --verbose...") != null);
    try testing.expect(std.mem.indexOf(u8, text, "[default: info]") != null);
    try testing.expect(std.mem.indexOf(u8, text, "[env: TOOL_JOBS]") != null);
    try testing.expect(std.mem.indexOf(u8, text, "[possible values: debug, info, warn]") != null);
    try testing.expect(std.mem.indexOf(u8, text, "安静") != null);
    try testing.expect(std.mem.indexOf(u8, text, "[default: false]") == null);
    try testing.expect(std.mem.indexOf(u8, text, "-V, --version") != null);
    try testing.expect(std.mem.indexOf(u8, text, "-h, --help") != null);

    const long_text = try parser.format_help(true);
    try testing.expect(std.mem.startsWith(u8, long_text, "工具集的完整说明"));
    try testing.expectEqual(@as(usize, 1), count_of(long_text, "工具集"));

    const options = try section(allocator, text, "Options:");
    try testing.expectEqual(@as(usize, 6), options.len);
    try testing.expect(std.mem.indexOf(u8, options[options.len - 2], "Print help") != null);
    try testing.expect(std.mem.indexOf(u8, options[options.len - 1], "Print version") != null);
    try testing.expect(std.mem.indexOf(u8, text, "[default: true]") == null);

    const commands = try section(allocator, text, "Commands:");
    try testing.expect(std.mem.startsWith(u8, std.mem.trimStart(u8, commands[0], " "), "build, b"));
    try testing.expect(std.mem.startsWith(u8, std.mem.trimStart(u8, commands[1], " "), "run"));
    try testing.expect(std.mem.startsWith(u8, std.mem.trimStart(u8, commands[commands.len - 1], " "), "help"));
    try testing.expect(std.mem.indexOf(u8, commands[commands.len - 1], "Print this message") != null);

    const positionals = try section(allocator, text, "Arguments:");
    try testing.expect(std.mem.startsWith(u8, std.mem.trimStart(u8, positionals[0], " "), "[SRC]..."));
}

test "帮助: 长帮助里的可选值说明" {
    const parser = try argparser.Parser.init(testing.allocator, "level");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--level"}, .{
        .choices = &.{
            .{ .value = .{ .string = "debug" }, .help = "逐条打印" },
            .{ .value = .{ .string = "info" }, .help = "常规" },
        },
        .default_value = .{ .value = .{ .string = "info" } },
        .help = "级别",
        .long_help = "日志级别, 影响输出量",
    });

    const short_text = try parser.format_help(false);
    try testing.expect(std.mem.indexOf(u8, short_text, "影响输出量") == null);
    try testing.expect(std.mem.indexOf(u8, short_text, "级别") != null);
    const long_text = try parser.format_help(true);
    try testing.expect(std.mem.indexOf(u8, long_text, "影响输出量") != null);
    try testing.expect(std.mem.indexOf(u8, long_text, "- debug: 逐条打印") != null);
}

test "帮助: 分组" {
    const parser = try argparser.Parser.init(testing.allocator, "grouped");
    defer parser.deinit();
    _ = parser.with_color(.never);
    const group = try parser.add_group("Database", "数据库相关");
    _ = try group.add_argument(&.{"--host"}, .{ .default_value = .{ .value = .{ .string = "localhost" } }, .help = "主机" });
    _ = try group.add_argument(&.{"--port"}, .{
        .value_parser = .{ .int = .{} },
        .default_value = .{ .value = .{ .integer = 5432 } },
        .help = "端口",
    });
    _ = try parser.add_argument(&.{"--debug"}, .{ .action = .store_true, .help = "调试" });

    const text = try parser.format_help(false);
    try testing.expect(std.mem.indexOf(u8, text, "Database:") != null);
    try testing.expect(std.mem.indexOf(u8, text, "数据库相关") != null);
    try testing.expect(std.mem.indexOf(u8, text, "--host").? > std.mem.indexOf(u8, text, "Database:").?);
    try testing.expect(std.mem.indexOf(u8, text, "--debug").? < std.mem.indexOf(u8, text, "Database:").?);
}

test "帮助: 对齐与折行" {
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer arena.deinit();
    const scratch = arena.allocator();
    const aligned = try argparser.Parser.init(testing.allocator, "align");
    defer aligned.deinit();
    _ = aligned.with_color(.never);
    _ = try aligned.add_argument(&.{ "-a", "--all" }, .{ .help = "全部" });
    _ = try aligned.add_argument(&.{"--beta"}, .{ .metavar = "BETA", .help = "乙" });
    const text = try aligned.format_help(false);
    const rows = try section(scratch, text, "Options:");
    try testing.expectEqual(std.mem.indexOf(u8, rows[0], "全部").?, std.mem.indexOf(u8, rows[1], "乙").?);
    try testing.expect(std.mem.startsWith(u8, rows[0], "  -a, --all"));

    const wrap = try argparser.Parser.init(testing.allocator, "wrap");
    defer wrap.deinit();
    _ = wrap.with_color(.never);
    _ = wrap.with_max_width(44);
    _ = try wrap.add_argument(&.{"--long"}, .{
        .metavar = "VALUE",
        .help = "这是一段足够长的说明文字, 必须在窄幅下折行并保持对齐缩进",
    });
    const narrow = try wrap.format_help(false);
    const option_rows = try section(scratch, narrow, "Options:");
    try testing.expect(option_rows.len >= 2);
    var continuation: []const u8 = "";
    for (option_rows[1..]) |line| {
        if (std.mem.trim(u8, line, " ").len == 0) continue;
        continuation = line;
        break;
    }
    var pad: [18]u8 = undefined;
    @memset(&pad, ' ');
    try testing.expect(std.mem.startsWith(u8, continuation, &pad));
    var lines = std.mem.splitScalar(u8, narrow, '\n');
    while (lines.next()) |line| {
        try testing.expect(width_of(line) <= 44);
    }
}

test "帮助: 子命令继承全局参数" {
    const parser = try argparser.Parser.init(testing.allocator, "tool");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{ "-q", "--quiet" }, .{ .action = .store_true, .global = true, .help = "安静模式" });
    const subparsers = try parser.add_subparsers("command", "Commands", false);
    const build = try subparsers.add_parser("build", "构建", &.{}, &.{});
    _ = try build.add_argument(&.{ "-j", "--jobs" }, .{
        .value_parser = .{ .int = .{} },
        .default_value = .{ .value = .{ .integer = 4 } },
        .help = "并发数",
    });
    const child = try build.format_help(false);
    try testing.expect(std.mem.indexOf(u8, child, "Usage: tool build [OPTIONS]") != null);
    try testing.expect(std.mem.indexOf(u8, child, "-q, --quiet") != null);
    try testing.expect(std.mem.indexOf(u8, child, "-j, --jobs") != null);

    const nested = try argparser.Parser.init(testing.allocator, "nested");
    defer nested.deinit();
    _ = nested.with_color(.never);
    const nested_commands = try nested.add_subparsers("command", "Commands", false);
    const nested_build = try nested_commands.add_parser("build", "构建", &.{}, &.{});
    const targets = try nested_build.add_subparsers("target", "Commands", false);
    _ = try targets.add_parser("release", "发布版", &.{}, &.{});
    const nested_text = try nested_build.format_help(false);
    try testing.expect(std.mem.indexOf(u8, nested_text, "Print this message") != null);

    const long_help = try parser.parse(&.{ "build", "--help" });
    try testing.expect(switch (long_help) {
        .help => |requested| requested.long_form and std.mem.eql(u8, requested.prog, "tool build"),
        else => false,
    });
    const short_help = try parser.parse(&.{"-h"});
    try testing.expect(switch (short_help) {
        .help => |requested| !requested.long_form and std.mem.eql(u8, requested.prog, "tool"),
        else => false,
    });
}

test "帮助: 错误版式" {
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer arena.deinit();
    const scratch = arena.allocator();
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    _ = try parser.with_about("演示");
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--level"}, .{
        .choices = &.{
            .{ .value = .{ .string = "debug" } },
            .{ .value = .{ .string = "info" } },
            .{ .value = .{ .string = "warn" } },
        },
        .help = "级别",
    });

    const text = try usage_text(parser, &.{ "--level", "warnn" });
    try testing.expect(std.mem.startsWith(u8, text, "error: invalid value 'warnn' for '--level <LEVEL>'"));
    try testing.expect(std.mem.indexOf(u8, text, "\n  [possible values: debug, info, warn]") != null);
    try testing.expect(std.mem.indexOf(u8, text, "\n  tip: a similar value exists: 'warn'") != null);
    try testing.expect(std.mem.indexOf(u8, text, "\nUsage: demo [OPTIONS]") != null);
    try testing.expect(std.mem.endsWith(u8, text, "For more information, try '--help'."));

    const quiet = try argparser.Parser.init(testing.allocator, "nohelp");
    defer quiet.deinit();
    _ = quiet.with_color(.never);
    _ = try quiet.without_help();
    _ = try quiet.add_argument(&.{"--x"}, .{ .help = "甲" });
    const no_hint = try usage_text(quiet, &.{"--y"});
    try testing.expect(std.mem.indexOf(u8, no_hint, "For more information") == null);

    const custom = err.UsageError{
        .message = "自定义",
        .reason = "原因",
        .lines = &.{"明细"},
        .tips = &.{"建议"},
        .usage = "demo",
    };
    try testing.expect(std.mem.startsWith(u8, try custom.render(scratch), "error: 自定义: 原因"));
}

test "帮助: 着色" {
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer arena.deinit();
    const scratch = arena.allocator();
    const colored = try argparser.Parser.init(testing.allocator, "color");
    defer colored.deinit();
    _ = colored.with_color(.always);
    _ = try colored.add_argument(&.{"--x"}, .{ .help = "甲" });
    try testing.expect(std.mem.indexOf(u8, try colored.format_help(false), "\x1b[") != null);

    const mono = try argparser.Parser.init(testing.allocator, "color");
    defer mono.deinit();
    _ = mono.with_color(.never);
    _ = try mono.add_argument(&.{"--x"}, .{ .help = "甲" });
    const plain_text = try mono.format_help(false);
    try testing.expect(std.mem.indexOf(u8, plain_text, "\x1b[") == null);
    try testing.expect(std.mem.indexOf(u8, try usage_text(mono, &.{"--y"}), "\x1b[") == null);
    try testing.expectEqualStrings(plain_text, try plain(scratch, try colored.format_help(false)));
}

test "帮助: 打印与退出码" {
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer arena.deinit();
    const scratch = arena.allocator();
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    _ = try parser.with_version("1.0.0");
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--x"}, .{ .help = "甲" });

    var printed: std.Io.Writer.Allocating = .init(testing.allocator);
    defer printed.deinit();
    try parser.print_help(&printed.writer, false);
    try testing.expectEqualStrings(
        try std.fmt.allocPrint(scratch, "{s}\n", .{try parser.format_help(false)}),
        printed.written(),
    );

    var out: std.Io.Writer.Allocating = .init(testing.allocator);
    defer out.deinit();
    var errors: std.Io.Writer.Allocating = .init(testing.allocator);
    defer errors.deinit();

    const shown = try parser.parse_or_exit(&.{"--help"}, &out.writer, &errors.writer);
    try testing.expectEqual(@as(u8, 0), shown.exit_code());
    try testing.expectEqualStrings(
        try std.fmt.allocPrint(scratch, "{s}\n", .{try parser.format_help(true)}),
        out.written(),
    );
    try testing.expectEqual(@as(usize, 0), errors.written().len);

    out.clearRetainingCapacity();
    errors.clearRetainingCapacity();
    const version = try parser.parse_or_exit(&.{"-V"}, &out.writer, &errors.writer);
    try testing.expectEqual(@as(u8, 0), version.exit_code());
    try testing.expectEqualStrings("demo 1.0.0\n", out.written());

    out.clearRetainingCapacity();
    errors.clearRetainingCapacity();
    const failure = try parser.parse_or_exit(&.{"--y"}, &out.writer, &errors.writer);
    try testing.expectEqual(@as(u8, 2), failure.exit_code());
    try testing.expectEqual(@as(usize, 0), out.written().len);
    try testing.expect(std.mem.endsWith(u8, errors.written(), "\n"));

    out.clearRetainingCapacity();
    errors.clearRetainingCapacity();
    const parsed = try parser.parse_or_exit(&.{ "--x", "1" }, &out.writer, &errors.writer);
    try testing.expect(switch (parsed) {
        .ok => true,
        else => false,
    });
    try testing.expectEqual(@as(usize, 0), out.written().len);
    try testing.expectEqual(@as(usize, 0), errors.written().len);

    const tool = try argparser.Parser.init(testing.allocator, "tool");
    defer tool.deinit();
    _ = tool.with_color(.never);
    const subparsers = try tool.add_subparsers("command", "Commands", false);
    const build = try subparsers.add_parser("build", "构建", &.{}, &.{});
    out.clearRetainingCapacity();
    errors.clearRetainingCapacity();
    const nested = try tool.parse_or_exit(&.{ "build", "--help" }, &out.writer, &errors.writer);
    try testing.expectEqual(@as(u8, 0), nested.exit_code());
    try testing.expectEqualStrings(
        try std.fmt.allocPrint(scratch, "{s}\n", .{try build.format_help(true)}),
        out.written(),
    );
}
