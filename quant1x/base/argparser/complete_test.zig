// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 补全生成的用例: 三种 shell 的脚本骨架、候选词与转义。
//!
//! 与 `complete_test.py` 的同名用例逐条对应; 期望值只由 Python 侧生成。

const std = @import("std");
const spec = @import("spec.zig");
const argparser = @import("parser.zig");
const complete_mod = @import("complete.zig");

const testing = std.testing;

const complete_bash =
    \\_argparser_demo() {
    \\    local cur cmd word
    \\    cur="${COMP_WORDS[COMP_CWORD]}"
    \\    cmd=""
    \\    for word in "${COMP_WORDS[@]:1:COMP_CWORD-1}"; do
    \\        case "$cmd|$word" in
    \\            "|alpha") cmd="alpha" ;;
    \\            "|beta") cmd="beta" ;;
    \\        esac
    \\    done
    \\    case "$cmd" in
    \\        "") COMPREPLY=( $(compgen -W "-h --help -v --verbose --mode alpha beta x y" -- "$cur") ) ;;
    \\        "alpha") COMPREPLY=( $(compgen -W "-h --help --file -v --verbose" -- "$cur") ) ;;
    \\        "beta") COMPREPLY=( $(compgen -W "-h --help --count -v --verbose" -- "$cur") ) ;;
    \\    esac
    \\}
    \\complete -F _argparser_demo demo
    \\
;

const complete_zsh =
    \\#compdef demo
    \\
    \\_argparser_demo() {
    \\    local -a candidates
    \\    local cmd="" word
    \\    for word in ${words[2,CURRENT-1]}; do
    \\        case "$cmd|$word" in
    \\            "|alpha") cmd="alpha" ;;
    \\            "|beta") cmd="beta" ;;
    \\        esac
    \\    done
    \\    case "$cmd" in
    \\        "") candidates=(-h --help -v --verbose --mode alpha beta x y) ;;
    \\        "alpha") candidates=(-h --help --file -v --verbose) ;;
    \\        "beta") candidates=(-h --help --count -v --verbose) ;;
    \\    esac
    \\    compadd -- "${candidates[@]}"
    \\}
    \\
    \\_argparser_demo "$@"
    \\
;

const complete_fish =
    \\complete -c demo -s h -l help -d 'Print help'
    \\complete -c demo -s v -l verbose -d 'verbose output'
    \\complete -c demo -l mode -d 'mode'
    \\complete -c demo -f -a alpha -d 'alpha command'
    \\complete -c demo -f -a beta -d 'beta command'
    \\complete -c demo -f -a 'x y'
    \\complete -c demo -n '__fish_seen_subcommand_from alpha' -s h -l help -d 'Print help'
    \\complete -c demo -n '__fish_seen_subcommand_from alpha' -l file -d 'input file'
    \\complete -c demo -n '__fish_seen_subcommand_from beta' -s h -l help -d 'Print help'
    \\complete -c demo -n '__fish_seen_subcommand_from beta' -l count
    \\
;

/// 顶层三个选项 + 两条子命令, 子命令各带一个选项; 与 Python 用例的 `tool()` 同形。
fn build_tool(parser: *argparser.Parser) !void {
    _ = try parser.with_about("演示");
    _ = try parser.add_argument(&.{ "-v", "--verbose" }, .{
        .action = .store_true,
        .help = "verbose output",
        .global = true,
    });
    _ = try parser.add_argument(&.{"--mode"}, .{ .help = "mode" });
    _ = try parser.add_argument(&.{"--secret"}, .{ .hide = true });
    _ = try parser.add_argument(&.{"kind"}, .{
        .choices = &.{
            .{ .value = .{ .string = "x" } },
            .{ .value = .{ .string = "y" } },
        },
    });
    const subparsers = try parser.add_subparsers("", "Commands", false);
    const alpha = try subparsers.add_parser("alpha", "alpha command", &.{}, &.{});
    _ = try alpha.add_argument(&.{"--file"}, .{ .help = "input file" });
    const beta = try subparsers.add_parser("beta", "beta command", &.{}, &.{});
    _ = try beta.add_argument(&.{"--count"}, .{});
}

test "补全: 三种 shell 的脚本与 Python 样本一致" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_tool(parser);

    try testing.expectEqualStrings(complete_bash, try complete_mod.complete(parser, "bash", null));
    try testing.expectEqualStrings(complete_zsh, try complete_mod.complete(parser, "zsh", null));
    try testing.expectEqualStrings(complete_fish, try complete_mod.complete(parser, "fish", null));
}

test "补全: prog 可被覆盖且函数名归一" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_tool(parser);

    const script = try complete_mod.complete(parser, "bash", "my-tool");
    try testing.expect(std.mem.startsWith(u8, script, "_argparser_my_tool() {\n"));
    try testing.expect(std.mem.indexOf(u8, script, "complete -F _argparser_my_tool my-tool") != null);
}

test "补全: 未知 shell 报错" {
    const parser = try argparser.Parser.init(testing.allocator, "demo");
    defer parser.deinit();
    try build_tool(parser);

    try testing.expectError(error.UnsupportedShell, complete_mod.complete(parser, "powershell", null));
}

test "补全: 无子命令时不生成状态机" {
    const parser = try argparser.Parser.init(testing.allocator, "solo");
    defer parser.deinit();

    const script = try complete_mod.complete(parser, "bash", null);
    try testing.expect(std.mem.indexOf(u8, script, "$cmd|$word") == null);
    try testing.expect(std.mem.indexOf(u8, script, "        \"\") COMPREPLY=") != null);
}

test "补全: 空说明文字不写 -d" {
    const parser = try argparser.Parser.init(testing.allocator, "blank");
    defer parser.deinit();
    _ = try parser.add_argument(&.{"--flag"}, .{ .help = "" });
    _ = try parser.add_subparsers("", "Commands", false);

    const script = try complete_mod.complete(parser, "fish", null);
    try testing.expect(std.mem.indexOf(u8, script, "-d ''") == null);
}
