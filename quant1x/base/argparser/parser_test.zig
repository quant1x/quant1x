// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 解析引擎的用例: 选项形态、nargs、动作、转换器、校验、子命令与错误文案。
//!
//! 环境变量兜底的用例不在这里 —— 改本进程环境变量在 Windows 与 POSIX 上写法不同,
//! 语义已由 `apply_env` 落地, 对拍见 `parser_test.py` 的同名用例。

const std = @import("std");
const spec = @import("spec.zig");
const err = @import("error.zig");
const argparser = @import("parser.zig");

const testing = std.testing;

/// 本节复用的命令: 计数 / 必填取值 / 可选值 / 位置参数。
fn build_base(parser: *argparser.Parser) !void {
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{ "-v", "--verbose" }, .{ .action = .count });
    _ = try parser.add_argument(&.{ "-o", "--output" }, .{ .required = true });
    _ = try parser.add_argument(&.{"--level"}, .{
        .choices = &.{
            .{ .value = .{ .string = "debug" } },
            .{ .value = .{ .string = "info" } },
            .{ .value = .{ .string = "warn" } },
        },
        .default_value = .{ .value = .{ .string = "info" } },
    });
    _ = try parser.add_argument(&.{"files"}, .{ .nargs = spec.Nargs.one_or_more() });
}

/// 两级子命令: tool → build / run, build → release / debug。
fn build_tool(parser: *argparser.Parser) !void {
    _ = try parser.with_version("9.9.9");
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{ "-q", "--quiet" }, .{ .action = .store_true, .global = true });
    const subparsers = try parser.add_subparsers("command", "Commands", true);
    const build = try subparsers.add_parser("build", "构建", &.{}, &.{"b"});
    _ = try build.add_argument(&.{ "-j", "--jobs" }, .{
        .value_parser = .{ .int = .{} },
        .default_value = .{ .value = .{ .integer = 4 } },
    });
    const targets = try build.add_subparsers("target", "Targets", false);
    _ = try targets.add_parser("release", "发布版", &.{}, &.{});
    _ = try targets.add_parser("debug", "调试版", &.{}, &.{});
    const run_parser = try subparsers.add_parser("run", "运行", &.{"r"}, &.{});
    _ = try run_parser.add_argument(&.{"script"}, .{});
}

/// 取命名空间里某个名字的文本形态。
fn text(allocator: std.mem.Allocator, namespace: spec.Namespace, key: []const u8) ![]const u8 {
    const value = namespace.get(key) orelse return "";
    return value.text(allocator);
}

/// 断言解析成功并返回命名空间。
fn expect_ok(parser: *argparser.Parser, argv: []const []const u8) !spec.Namespace {
    const outcome = try parser.parse(argv);
    return switch (outcome) {
        .ok => |namespace| namespace,
        else => {
            const rendered = try outcome.render(parser.mem());
            std.debug.print("解析失败: {s}\n", .{rendered});
            return error.TestExpectedOk;
        },
    };
}

/// 断言解析失败并返回错误。
fn expect_failure(parser: *argparser.Parser, argv: []const []const u8) !err.Outcome {
    const outcome = try parser.parse(argv);
    return switch (outcome) {
        .ok => error.TestExpectedFailure,
        else => outcome,
    };
}

/// 取用法错误的渲染文本。
fn usage_text(parser: *argparser.Parser, argv: []const []const u8) ![]const u8 {
    const outcome = try expect_failure(parser, argv);
    return outcome.render(parser.mem());
}

test "选项按各种形态取到取值" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_base(parser);
    const allocator = parser.mem();

    const namespace = try expect_ok(parser, &.{ "-o", "out.txt", "-vv", "--level=warn", "a.txt", "b.txt" });
    try testing.expectEqualStrings("out.txt", try text(allocator, namespace, "output"));
    try testing.expectEqualStrings("2", try text(allocator, namespace, "verbose"));
    try testing.expectEqualStrings("warn", try text(allocator, namespace, "level"));
    try testing.expectEqualStrings("a.txt, b.txt", try text(allocator, namespace, "files"));

    // 短选项粘值 `-oout.txt` 与 `-o=out.txt` 等价
    for ([_][]const []const u8{ &.{ "-oout.txt", "a.txt" }, &.{ "-o=out.txt", "a.txt" } }) |argv| {
        const glued = try expect_ok(parser, argv);
        try testing.expectEqualStrings("out.txt", try text(allocator, glued, "output"));
    }

    // 长选项前缀缩写照常解析
    const abbreviated = try expect_ok(parser, &.{ "-o", "x", "--verbos", "a.txt" });
    try testing.expectEqualStrings("1", try text(allocator, abbreviated, "verbose"));
}

test "签名数字与 - 一律当取值" {
    const parser = try argparser.Parser.init(testing.allocator, "neg");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"files"}, .{ .nargs = spec.Nargs.one_or_more() });
    const namespace = try expect_ok(parser, &.{ "-1", "-.5", "+1", "-1.5", "+.5", "-" });
    try testing.expectEqualStrings("-1, -.5, +1, -1.5, +.5, -", try text(parser.mem(), namespace, "files"));
}

test "nargs 的四种形态与分隔符" {
    const parser = try argparser.Parser.init(testing.allocator, "shapes");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--pair"}, .{ .nargs = spec.Nargs.fixed(2) });
    _ = try parser.add_argument(&.{"--maybe"}, .{ .nargs = spec.Nargs.optional() });
    _ = try parser.add_argument(&.{"--many"}, .{ .nargs = spec.Nargs.zero_or_more() });
    _ = try parser.add_argument(&.{"--list"}, .{ .value_delimiter = "," });
    _ = try parser.add_argument(&.{"--rest"}, .{ .nargs = spec.Nargs.remainder() });
    const allocator = parser.mem();

    const namespace = try expect_ok(parser, &.{
        "--pair", "a",      "b",     "--maybe", "--many", "x",
        "y",      "--list", "1,2,3", "--rest",  "z",      "--flag",
    });
    try testing.expectEqualStrings("a, b", try text(allocator, namespace, "pair"));
    try testing.expectEqualStrings("None", try text(allocator, namespace, "maybe"));
    try testing.expectEqualStrings("x, y", try text(allocator, namespace, "many"));
    try testing.expectEqualStrings("1, 2, 3", try text(allocator, namespace, "list"));
    try testing.expectEqualStrings("z, --flag", try text(allocator, namespace, "rest"));
}

test "变长位置参数给后面的位置参数留位" {
    const parser = try argparser.Parser.init(testing.allocator, "var");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"src"}, .{ .nargs = spec.Nargs.zero_or_more() });
    _ = try parser.add_argument(&.{"dst"}, .{});
    const namespace = try expect_ok(parser, &.{ "a", "b", "c" });
    try testing.expectEqualStrings("a, b", try text(parser.mem(), namespace, "src"));
    try testing.expectEqualStrings("c", try text(parser.mem(), namespace, "dst"));
}

test "十种动作的落值" {
    const parser = try argparser.Parser.init(testing.allocator, "actions");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"-v"}, .{ .action = .count });
    _ = try parser.add_argument(&.{"--on"}, .{ .action = .store_true });
    _ = try parser.add_argument(&.{"--off"}, .{ .action = .store_false });
    _ = try parser.add_argument(&.{"--mode"}, .{
        .action = .store_const,
        .const_value = .{ .value = .{ .string = "fast" } },
        .default_value = .{ .value = .{ .string = "slow" } },
    });
    _ = try parser.add_argument(&.{"-D"}, .{ .action = .append });
    _ = try parser.add_argument(&.{"--tag"}, .{
        .action = .append_const,
        .const_value = .{ .value = .{ .string = "T" } },
    });
    _ = try parser.add_argument(&.{"-I"}, .{ .action = .extend, .nargs = spec.Nargs.one_or_more() });
    const allocator = parser.mem();

    const namespace = try expect_ok(parser, &.{
        "-vvv", "--on", "--off", "--mode", "-D", "A", "-D", "B", "--tag", "-I", "x", "y",
    });
    try testing.expectEqualStrings("3", try text(allocator, namespace, "v"));
    try testing.expectEqualStrings("true", try text(allocator, namespace, "on"));
    try testing.expectEqualStrings("false", try text(allocator, namespace, "off"));
    try testing.expectEqualStrings("fast", try text(allocator, namespace, "mode"));
    try testing.expectEqualStrings("A, B", try text(allocator, namespace, "D"));
    try testing.expectEqualStrings("T", try text(allocator, namespace, "tag"));
    try testing.expectEqualStrings("x, y", try text(allocator, namespace, "I"));
}

test "转换器、区间与可选值建议" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_base(parser);
    const rendered = try usage_text(parser, &.{ "-o", "out.txt", "--level", "warnn", "a.txt" });
    try testing.expect(std.mem.indexOf(u8, rendered, "invalid value 'warnn' for '--level <LEVEL>'") != null);
    try testing.expect(std.mem.indexOf(u8, rendered, "[possible values: debug, info, warn]") != null);
    try testing.expect(std.mem.indexOf(u8, rendered, "a similar value exists: 'warn'") != null);

    const ranged = try argparser.Parser.init(testing.allocator, "num");
    defer ranged.deinit();
    _ = ranged.with_color(.never);
    _ = try ranged.add_argument(&.{"--n"}, .{ .value_parser = .{ .int = .{ .minimum = 1, .maximum = 10 } } });
    const overflow = try usage_text(ranged, &.{ "--n", "11" });
    try testing.expect(std.mem.indexOf(u8, overflow, "11 is not in 1..=10") != null);
    const invalid = try usage_text(ranged, &.{ "--n", "abc" });
    try testing.expect(std.mem.indexOf(u8, invalid, "invalid digit found in string") != null);
}

test "未知选项给建议, 无从建议时给 -- 提示" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_base(parser);

    const outcome = try expect_failure(parser, &.{ "-o", "x", "--verbsoe", "a.txt" });
    try testing.expect(switch (outcome) {
        .usage => |failure| failure.kind == .unknown_argument,
        else => false,
    });
    const rendered = try outcome.render(parser.mem());
    try testing.expect(std.mem.indexOf(u8, rendered, "unexpected argument '--verbsoe' found") != null);
    try testing.expect(std.mem.indexOf(u8, rendered, "a similar argument exists: '--verbose'") != null);

    const fallback = try usage_text(parser, &.{ "-o", "x", "--zzz", "a.txt" });
    try testing.expect(std.mem.indexOf(u8, fallback, "to pass '--zzz' as a value, use '-- --zzz'") != null);
}

test "前缀命中多个时列出候选" {
    const parser = try argparser.Parser.init(testing.allocator, "amb");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--output"}, .{});
    _ = try parser.add_argument(&.{"--outdir"}, .{});
    const outcome = try expect_failure(parser, &.{ "--out", "x" });
    try testing.expect(switch (outcome) {
        .usage => |failure| failure.kind == .ambiguous_argument,
        else => false,
    });
    const rendered = try outcome.render(parser.mem());
    try testing.expect(std.mem.indexOf(u8, rendered, "could be one of: --outdir, --output") != null);
}

test "缺取值与缺必填项的文案" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_base(parser);

    const missing = try usage_text(parser, &.{"-o"});
    try testing.expect(std.mem.indexOf(u8, missing, "a value is required for '--output <OUTPUT>' but none was supplied") != null);

    const required = try usage_text(parser, &.{"a.txt"});
    try testing.expect(std.mem.indexOf(u8, required, "the following required arguments were not provided:") != null);
    try testing.expect(std.mem.indexOf(u8, required, "--output <OUTPUT>") != null);

    const multi = try argparser.Parser.init(testing.allocator, "multi");
    defer multi.deinit();
    _ = multi.with_color(.never);
    _ = try multi.add_argument(&.{"--triple"}, .{ .nargs = spec.Nargs.fixed(3) });
    const shortage = try usage_text(multi, &.{ "--triple", "a" });
    try testing.expect(std.mem.indexOf(u8, shortage, "3 values required for '--triple <TRIPLE>' but 1 was provided") != null);
}

test "校验顺序: 冲突 → 依赖 → 必填" {
    const parser = try argparser.Parser.init(testing.allocator, "check");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--a"}, .{ .required = true });
    _ = try parser.add_argument(&.{"--b"}, .{ .conflicts_with = &.{"--a"} });
    _ = try parser.add_argument(&.{"--c"}, .{ .requires_args = &.{"--a"} });

    const conflict = try expect_failure(parser, &.{ "--a", "1", "--b", "2" });
    try testing.expect(switch (conflict) {
        .usage => |failure| failure.kind == .conflict,
        else => false,
    });
    const conflict_text = try conflict.render(parser.mem());
    try testing.expect(std.mem.indexOf(u8, conflict_text, "the argument '--b <B>' cannot be used with '--a <A>'") != null);

    const dependent = try usage_text(parser, &.{ "--c", "3" });
    try testing.expect(std.mem.indexOf(u8, dependent, "the argument '--c <C>' requires '--a <A>'") != null);

    const mutex = try argparser.Parser.init(testing.allocator, "mutex");
    defer mutex.deinit();
    _ = mutex.with_color(.never);
    const group = try mutex.add_mutually_exclusive_group(true);
    _ = try group.add_argument(&.{"--x"}, .{});
    _ = try group.add_argument(&.{"--y"}, .{});

    const empty_group = try usage_text(mutex, &.{});
    try testing.expect(std.mem.indexOf(u8, empty_group, "one of the following arguments is required:") != null);
    try testing.expect(std.mem.indexOf(u8, empty_group, "  --x <X>\n  --y <Y>") != null);
    const both = try usage_text(mutex, &.{ "--x", "1", "--y", "2" });
    try testing.expect(std.mem.indexOf(u8, both, "'--x <X>' cannot be used with '--y <Y>'") != null);
}

test "缺省值优先级与 SUPPRESS" {
    const parser = try argparser.Parser.init(testing.allocator, "defaults");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = parser.with_argument_default(.{ .value = .{ .string = "fallback" } });
    _ = try parser.add_argument(&.{"--name"}, .{});
    _ = try parser.add_argument(&.{"--quiet"}, .{ .default_value = .suppress });
    _ = try parser.add_argument(&.{"--count"}, .{ .action = .count });
    const allocator = parser.mem();

    const namespace = try expect_ok(parser, &.{});
    try testing.expectEqualStrings("fallback", try text(allocator, namespace, "name"));
    try testing.expect(!namespace.contains("quiet"));
    try testing.expectEqualStrings("0", try text(allocator, namespace, "count"));

    const inferred = try argparser.Parser.init(testing.allocator, "infer");
    defer inferred.deinit();
    _ = inferred.with_color(.never);
    _ = try inferred.add_argument(&.{"--port"}, .{ .default_value = .{ .value = .{ .integer = 80 } } });
    const typed = try expect_ok(inferred, &.{ "--port", "8080" });
    const value = typed.get("port").?;
    try testing.expect(value == .integer and value.integer == 8080);
}

test "子命令、别名、dest 与全局参数" {
    const parser = try argparser.Parser.init(testing.allocator, "tool");
    defer parser.deinit();
    try build_tool(parser);
    const allocator = parser.mem();

    const namespace = try expect_ok(parser, &.{ "b", "-j", "8", "release" });
    try testing.expectEqualStrings("build", try text(allocator, namespace, "command"));
    try testing.expectEqualStrings("release", try text(allocator, namespace, "target"));
    try testing.expectEqualStrings("8", try text(allocator, namespace, "jobs"));

    const forwarded = try expect_ok(parser, &.{ "run", "-q", "x.sh" });
    try testing.expectEqualStrings("true", try text(allocator, forwarded, "quiet"));

    const no_command = try usage_text(parser, &.{});
    try testing.expect(std.mem.indexOf(u8, no_command, "<COMMAND>") != null);
    const no_script = try usage_text(parser, &.{"run"});
    try testing.expect(std.mem.indexOf(u8, no_script, "<SCRIPT>") != null);
}

test "help 子命令可下沉, 认不出的名字报错" {
    const parser = try argparser.Parser.init(testing.allocator, "tool");
    defer parser.deinit();
    try build_tool(parser);

    const outcome = try expect_failure(parser, &.{ "help", "build" });
    try testing.expect(switch (outcome) {
        .help => |requested| requested.long_form and std.mem.eql(u8, requested.prog, "tool build"),
        else => false,
    });
    const unknown = try usage_text(parser, &.{ "help", "nosuch" });
    try testing.expect(std.mem.indexOf(u8, unknown, "unrecognized subcommand 'nosuch'") != null);
}

test "帮助与版本标志走控制流" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    _ = try parser.with_version("1.2.3");
    _ = parser.with_color(.never);

    const short_help = try expect_failure(parser, &.{"-h"});
    try testing.expect(switch (short_help) {
        .help => |requested| !requested.long_form and std.mem.eql(u8, requested.prog, "demo"),
        else => false,
    });
    for ([_][]const u8{ "-V", "--version" }) |flag| {
        const outcome = try expect_failure(parser, &.{flag});
        try testing.expect(switch (outcome) {
            .version => |requested| std.mem.eql(u8, requested.text, "demo 1.2.3"),
            else => false,
        });
    }

    const tool_parser = try argparser.Parser.init(testing.allocator, "tool");
    defer tool_parser.deinit();
    try build_tool(tool_parser);
    const outcome = try expect_failure(tool_parser, &.{ "build", "--version" });
    try testing.expect(switch (outcome) {
        .version => |requested| std.mem.eql(u8, requested.text, "tool build 9.9.9"),
        else => false,
    });
}

test "parse_known 只收未知项自身, extras 保命令行顺序" {
    const parser = try argparser.Parser.init(testing.allocator, "known");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--flag"}, .{});
    const outcome = try parser.parse_known(&.{ "--flag", "v", "--zzz", "left", "--yyy", "--flag", "w", "tail" });
    const known = switch (outcome) {
        .ok => |value| value,
        .failure => return error.TestExpectedOk,
    };
    try testing.expectEqualStrings("w", try text(parser.mem(), known.namespace, "flag"));
    try testing.expectEqual(@as(usize, 4), known.extras.len);
    try testing.expectEqualStrings("--zzz", known.extras[0]);
    try testing.expectEqualStrings("left", known.extras[1]);
    try testing.expectEqualStrings("--yyy", known.extras[2]);
    try testing.expectEqualStrings("tail", known.extras[3]);
}

test "REMAINDER 边界上的选项 token 仍报未知参数" {
    const parser = try argparser.Parser.init(testing.allocator, "rem");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"head"}, .{});
    _ = try parser.add_argument(&.{"rest"}, .{ .nargs = spec.Nargs.remainder() });
    const outcome = try expect_failure(parser, &.{ "a", "--flag" });
    try testing.expect(switch (outcome) {
        .usage => |failure| failure.kind == .unknown_argument,
        else => false,
    });
}

test "定义期错误在登记时抛出" {
    const parser = try argparser.Parser.init(testing.allocator, "dup");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--a"}, .{});
    try testing.expectError(error.InvalidDefinition, parser.add_argument(&.{"--a"}, .{}));
    try testing.expectError(error.InvalidDefinition, parser.add_argument(&.{"--b"}, .{ .dest = "a" }));
    try testing.expectError(error.InvalidDefinition, parser.add_argument(&.{"-1"}, .{}));

    const remainder = try argparser.Parser.init(testing.allocator, "rem");
    defer remainder.deinit();
    _ = remainder.with_color(.never);
    _ = try remainder.add_argument(&.{"rest"}, .{ .nargs = spec.Nargs.remainder() });
    try testing.expectError(error.InvalidDefinition, remainder.add_argument(&.{"tail"}, .{}));

    const required = try argparser.Parser.init(testing.allocator, "req");
    defer required.deinit();
    try testing.expectError(error.InvalidDefinition, required.add_argument(&.{"f"}, .{ .required = true }));
}

test "命令链 dest 重复是定义期错误" {
    const parser = try argparser.Parser.init(testing.allocator, "chain");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"-v"}, .{ .action = .store_true, .global = true });
    const subparsers = try parser.add_subparsers("", "Commands", false);
    const child = try subparsers.add_parser("c", "", &.{}, &.{});
    _ = try child.add_argument(&.{"-v"}, .{ .action = .store_true });
    const outcome = try expect_failure(parser, &.{"c"});
    try testing.expect(switch (outcome) {
        .definition => true,
        else => false,
    });
}

test "usage 行按 nargs 形态拼装" {
    const parser = try argparser.Parser.init(testing.allocator, "usage");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--out"}, .{ .required = true });
    _ = try parser.add_argument(&.{"--opt"}, .{});
    _ = try parser.add_argument(&.{"one"}, .{ .nargs = spec.Nargs.optional() });
    _ = try parser.add_argument(&.{"many"}, .{ .nargs = spec.Nargs.one_or_more() });
    try testing.expectEqualStrings("usage --out <OUT> [OPTIONS] [ONE] <MANY>...", try parser.usage_spec());
}

test "相似度与建议阈值" {
    const allocator = testing.allocator;
    const score = try err.jaro_winkler(allocator, "--verbose", "--version");
    try testing.expect(score > 0.8825 and score < 0.8826);
    const candidates = [_][]const u8{ "--help", "--flag", "--output" };
    const match = try err.best_match(allocator, "--flah", &candidates);
    try testing.expect(match != null and std.mem.eql(u8, match.?, "--flag"));
    try testing.expect((try err.best_match(allocator, "--zzz", &candidates)) == null);
}

test "取值的文本形态" {
    var arena = std.heap.ArenaAllocator.init(testing.allocator);
    defer arena.deinit();
    const allocator = arena.allocator();
    try testing.expectEqualStrings("None", try (spec.Value{ .nil = {} }).text(allocator));
    try testing.expectEqualStrings("true", try (spec.Value{ .boolean = true }).text(allocator));
    try testing.expectEqualStrings("false", try (spec.Value{ .boolean = false }).text(allocator));
    try testing.expectEqualStrings("3", try (spec.Value{ .integer = 3 }).text(allocator));
    try testing.expectEqualStrings("2.0", try (spec.Value{ .float = 2.0 }).text(allocator));
    try testing.expectEqualStrings("2.5", try (spec.Value{ .float = 2.5 }).text(allocator));
    try testing.expectEqualStrings("text", try (spec.Value{ .string = "text" }).text(allocator));
    const list = [_]spec.Value{ .{ .string = "a" }, .{ .string = "b" } };
    try testing.expectEqualStrings("a, b", try (spec.Value{ .list = &list }).text(allocator));
}

test "宽度: auto_width 读 COLUMNS" {
    var map = std.process.Environ.Map.init(testing.allocator);
    defer map.deinit();
    try map.put("COLUMNS", "60");

    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    _ = parser.with_max_width(44);
    try testing.expectEqual(@as(u32, 44), parser.width());

    _ = parser.with_environ(&map);
    try testing.expectEqual(@as(u32, 44), parser.width());

    _ = parser.with_auto_width();
    try testing.expectEqual(@as(u32, 60), parser.width());

    try map.put("COLUMNS", "abc");
    try testing.expectEqual(@as(u32, 44), parser.width());

    try map.put("COLUMNS", "0");
    try testing.expectEqual(@as(u32, 44), parser.width());
}

test "@file: 每行一个 token 与嵌套" {
    var tmp = testing.tmpDir(.{});
    defer tmp.cleanup();
    try tmp.dir.writeFile(testing.io, .{
        .sub_path = "inner.txt",
        .data = "--output\ninner.txt\nb.txt\n",
    });
    const inner = try tmp.dir.realPathFileAlloc(testing.io, "inner.txt", testing.allocator);
    defer testing.allocator.free(inner);
    const outer_body = try std.fmt.allocPrint(testing.allocator, "@{s}\n", .{inner});
    defer testing.allocator.free(outer_body);
    try tmp.dir.writeFile(testing.io, .{ .sub_path = "outer.txt", .data = outer_body });
    const outer = try tmp.dir.realPathFileAlloc(testing.io, "outer.txt", testing.allocator);
    defer testing.allocator.free(outer);

    const parser = try argparser.Parser.init(testing.allocator, "files");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.with_fromfile_prefix_chars("@", testing.io);
    _ = try parser.add_argument(&.{ "-o", "--output" }, .{});
    _ = try parser.add_argument(&.{"files"}, .{ .nargs = spec.Nargs.zero_or_more() });

    const arg = try std.fmt.allocPrint(testing.allocator, "@{s}", .{outer});
    defer testing.allocator.free(arg);
    const namespace = try expect_ok(parser, &.{arg});
    try testing.expectEqualStrings("inner.txt", try text(parser.mem(), namespace, "output"));
    try testing.expectEqualStrings("b.txt", try text(parser.mem(), namespace, "files"));

    const missing = try std.fmt.allocPrint(testing.allocator, "@{s}", .{"no-such-file.txt"});
    defer testing.allocator.free(missing);
    const failure = try expect_failure(parser, &.{missing});
    const rendered = try failure.render(parser.mem());
    try testing.expect(std.mem.indexOf(u8, rendered, "unable to open file") != null);
}

test "覆盖: overrides_with 后出现者胜" {
    const parser = try argparser.Parser.init(testing.allocator, "over");
    defer parser.deinit();
    _ = parser.with_color(.never);
    _ = try parser.add_argument(&.{"--a"}, .{ .default_value = .{ .value = .{ .string = "A" } } });
    _ = try parser.add_argument(&.{"--b"}, .{
        .default_value = .{ .value = .{ .string = "B" } },
        .overrides_with = &.{"a"},
    });

    const last_b = try expect_ok(parser, &.{ "--a", "1", "--b", "2" });
    try testing.expectEqualStrings("2", try text(parser.mem(), last_b, "b"));
    try testing.expectEqualStrings("A", try text(parser.mem(), last_b, "a"));

    const last_a = try expect_ok(parser, &.{ "--b", "2", "--a", "1" });
    try testing.expectEqualStrings("1", try text(parser.mem(), last_a, "a"));
    try testing.expectEqualStrings("B", try text(parser.mem(), last_a, "b"));
}

test "校验: 必选组与 required_unless_present 决定要不要报缺" {
    const grouped = try argparser.Parser.init(testing.allocator, "grouped");
    defer grouped.deinit();
    _ = grouped.with_color(.never);
    const inputs = try grouped.add_value_group("input", true);
    _ = try inputs.add_argument(&.{"--file"}, .{});
    _ = try inputs.add_argument(&.{"--dir"}, .{});

    const one = try expect_ok(grouped, &.{ "--file", "x" });
    try testing.expectEqualStrings("x", try text(grouped.mem(), one, "file"));
    const both = try expect_ok(grouped, &.{ "--file", "x", "--dir", "y" });
    try testing.expectEqualStrings("y", try text(grouped.mem(), both, "dir"));
    const empty = try usage_text(grouped, &.{});
    try testing.expect(std.mem.indexOf(u8, empty, "the following required arguments were not provided:") != null);
    try testing.expect(std.mem.indexOf(u8, empty, "<input>") != null);

    const loose = try argparser.Parser.init(testing.allocator, "loose");
    defer loose.deinit();
    _ = loose.with_color(.never);
    const optional_group = try loose.add_value_group("maybe", false);
    _ = try optional_group.add_argument(&.{"--a"}, .{});
    _ = try expect_ok(loose, &.{});

    const gated = try argparser.Parser.init(testing.allocator, "gated");
    defer gated.deinit();
    _ = gated.with_color(.never);
    _ = try gated.add_argument(&.{"--token"}, .{ .required_unless_present = &.{"user"} });
    _ = try gated.add_argument(&.{"--user"}, .{});
    _ = try gated.add_argument(&.{"--key"}, .{ .overrides_with = &.{"user"} });

    _ = try expect_ok(gated, &.{ "--user", "u" });
    const without = try usage_text(gated, &.{});
    try testing.expect(std.mem.indexOf(u8, without, "--token <TOKEN>") != null);
    // 被 `--key` 覆盖掉的 `--user` 不再算出现过, 于是 `--token` 又变回必填
    const overridden = try usage_text(gated, &.{ "--user", "u", "--key", "k" });
    try testing.expect(std.mem.indexOf(u8, overridden, "--token <TOKEN>") != null);
}
