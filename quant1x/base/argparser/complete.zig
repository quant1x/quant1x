// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 由命令定义生成 bash / zsh / fish 的补全脚本。
//!
//! 候选词只看「已敲定的子命令链」: 选项名、子命令名、位置参数的 `choices`;
//! 不区分「正在写取值」的语境, 也不补文件路径。

const std = @import("std");
const spec = @import("spec.zig");
const parser_mod = @import("parser.zig");

const Parser = parser_mod.Parser;
const Argument = spec.Argument;

/// 支持的 shell, 按登记顺序。
pub const SHELLS = [_][]const u8{ "bash", "zsh", "fish" };

/// 一个命令在状态机里的位置: 子命令链与命令。
const Node = struct {
    chain: []const []const u8,
    parser: *Parser,
};

/// 状态机里的一条边: 键与命中后的命令链文本。
const Edge = struct {
    key: []const u8,
    chain: []const u8,
};

/// 生成 `shell` 的补全脚本; `prog` 缺省取 `parser.prog`。
///
/// `shell` 不是 `bash` / `zsh` / `fish` 之一时返回 `error.UnsupportedShell`。
pub fn complete(parser: *Parser, shell: []const u8, prog: ?[]const u8) ![]const u8 {
    var known = false;
    for (SHELLS) |item| {
        if (std.mem.eql(u8, item, shell)) known = true;
    }
    if (!known) return error.UnsupportedShell;
    const allocator = parser.mem();
    const name = prog orelse parser.prog;
    var nodes = std.ArrayList(Node).empty;
    try walk(allocator, parser, &.{}, &nodes);
    if (std.mem.eql(u8, shell, "bash")) return bash(allocator, name, nodes.items);
    if (std.mem.eql(u8, shell, "zsh")) return zsh(allocator, name, nodes.items);
    return fish(allocator, name, nodes.items);
}

/// 按前序收集 (子命令链, 命令); 链为空的是顶层命令。
fn walk(allocator: std.mem.Allocator, parser: *Parser, chain: []const []const u8, nodes: *std.ArrayList(Node)) !void {
    try nodes.append(allocator, .{ .chain = chain, .parser = parser });
    const subparsers = parser.subparsers orelse return;
    for (subparsers.commands.items) |command| {
        const next = try allocator.alloc([]const u8, chain.len + 1);
        @memcpy(next[0..chain.len], chain);
        next[chain.len] = command.name;
        try walk(allocator, command.parser, next, nodes);
    }
}

/// 含空白字符。
fn has_space(text: []const u8) bool {
    for (text) |ch| {
        if (std.ascii.isWhitespace(ch)) return true;
    }
    return false;
}

/// 词表里是否已有该词。
fn contains(items: []const []const u8, word: []const u8) bool {
    for (items) |item| {
        if (std.mem.eql(u8, item, word)) return true;
    }
    return false;
}

/// 一个命令的候选词: 选项名、子命令名、位置参数的 `choices`, 按登记顺序去重。
fn words(allocator: std.mem.Allocator, parser: *Parser) ![]const []const u8 {
    var collected = std.ArrayList([]const u8).empty;
    for (parser.arguments.items) |argument| {
        if (argument.hide) continue;
        try collected.appendSlice(allocator, argument.names);
    }
    var globals = std.ArrayList(*Argument).empty;
    var current = parser.parent;
    while (current) |parent| : (current = parent.parent) {
        for (parent.arguments.items) |argument| {
            if (argument.global) try globals.append(allocator, argument);
        }
    }
    var index = globals.items.len;
    while (index > 0) {
        index -= 1;
        const argument = globals.items[index];
        if (argument.hide) continue;
        try collected.appendSlice(allocator, argument.names);
    }
    if (parser.subparsers) |subparsers| {
        for (subparsers.commands.items) |command| {
            try collected.append(allocator, command.name);
        }
    }
    for (parser.positionals.items) |argument| {
        for (try argument.choice_values(allocator)) |value| {
            try collected.append(allocator, try value.text(allocator));
        }
    }
    var unique = std.ArrayList([]const u8).empty;
    for (collected.items) |word| {
        if (word.len == 0 or has_space(word) or contains(unique.items, word)) continue;
        try unique.append(allocator, word);
    }
    return unique.toOwnedSlice(allocator);
}

/// 脚本里的函数名: 非标识符字符一律换成下划线; 多字节字符整体照留。
fn function_name(allocator: std.mem.Allocator, name: []const u8) ![]const u8 {
    var out = std.ArrayList(u8).empty;
    try out.appendSlice(allocator, "_argparser_");
    for (name) |ch| {
        const identifier = ch == '_' or std.ascii.isAlphanumeric(ch) or ch >= 0x80;
        try out.append(allocator, if (identifier) ch else '_');
    }
    return out.toOwnedSlice(allocator);
}

/// 状态机里的每条边: 键形如 `|alpha`、`alpha|beta`。
fn transitions(allocator: std.mem.Allocator, nodes: []const Node) ![]const Edge {
    var edges = std.ArrayList(Edge).empty;
    for (nodes) |node| {
        if (node.chain.len == 0) continue;
        const head = try std.mem.join(allocator, " ", node.chain[0 .. node.chain.len - 1]);
        const key = try std.fmt.allocPrint(allocator, "{s}|{s}", .{ head, node.chain[node.chain.len - 1] });
        try edges.append(allocator, .{ .key = key, .chain = try std.mem.join(allocator, " ", node.chain) });
    }
    return edges.toOwnedSlice(allocator);
}

/// 生成 bash 补全脚本: 先按已输入词推进到命令链, 再按链给出候选词。
fn bash(allocator: std.mem.Allocator, name: []const u8, nodes: []const Node) ![]const u8 {
    const function = try function_name(allocator, name);
    var out = std.ArrayList(u8).empty;
    try out.appendSlice(allocator, function);
    try out.appendSlice(allocator, "() {\n    local cur cmd word\n");
    try out.appendSlice(allocator, "    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n    cmd=\"\"\n");
    const edges = try transitions(allocator, nodes);
    if (edges.len != 0) {
        try out.appendSlice(allocator, "    for word in \"${COMP_WORDS[@]:1:COMP_CWORD-1}\"; do\n");
        try out.appendSlice(allocator, "        case \"$cmd|$word\" in\n");
        for (edges) |edge| {
            try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "            \"{s}\") cmd=\"{s}\" ;;\n", .{ edge.key, edge.chain }));
        }
        try out.appendSlice(allocator, "        esac\n    done\n");
    }
    try out.appendSlice(allocator, "    case \"$cmd\" in\n");
    for (nodes) |node| {
        const chain = try std.mem.join(allocator, " ", node.chain);
        const candidates = try std.mem.join(allocator, " ", try words(allocator, node.parser));
        try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "        \"{s}\") COMPREPLY=( $(compgen -W \"{s}\" -- \"$cur\") ) ;;\n", .{ chain, candidates }));
    }
    try out.appendSlice(allocator, "    esac\n}\n");
    try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "complete -F {s} {s}\n", .{ function, name }));
    return out.toOwnedSlice(allocator);
}

/// 生成 zsh 补全脚本: 结构同 bash, 候选走 compadd。
fn zsh(allocator: std.mem.Allocator, name: []const u8, nodes: []const Node) ![]const u8 {
    const function = try function_name(allocator, name);
    var out = std.ArrayList(u8).empty;
    try out.appendSlice(allocator, "#compdef ");
    try out.appendSlice(allocator, name);
    try out.appendSlice(allocator, "\n\n");
    try out.appendSlice(allocator, function);
    try out.appendSlice(allocator, "() {\n    local -a candidates\n    local cmd=\"\" word\n");
    const edges = try transitions(allocator, nodes);
    if (edges.len != 0) {
        try out.appendSlice(allocator, "    for word in ${words[2,CURRENT-1]}; do\n");
        try out.appendSlice(allocator, "        case \"$cmd|$word\" in\n");
        for (edges) |edge| {
            try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "            \"{s}\") cmd=\"{s}\" ;;\n", .{ edge.key, edge.chain }));
        }
        try out.appendSlice(allocator, "        esac\n    done\n");
    }
    try out.appendSlice(allocator, "    case \"$cmd\" in\n");
    for (nodes) |node| {
        const chain = try std.mem.join(allocator, " ", node.chain);
        const candidates = try std.mem.join(allocator, " ", try words(allocator, node.parser));
        try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "        \"{s}\") candidates=({s}) ;;\n", .{ chain, candidates }));
    }
    try out.appendSlice(allocator, "    esac\n    compadd -- \"${candidates[@]}\"\n}\n\n");
    try out.appendSlice(allocator, function);
    try out.appendSlice(allocator, " \"$@\"\n");
    return out.toOwnedSlice(allocator);
}

/// 生成 fish 补全脚本: 每个选项一条 complete -c, 子命令用 __fish_seen_subcommand_from 限定。
fn fish(allocator: std.mem.Allocator, name: []const u8, nodes: []const Node) ![]const u8 {
    var out = std.ArrayList(u8).empty;
    for (nodes) |node| {
        const condition = if (node.chain.len == 0)
            ""
        else
            try std.fmt.allocPrint(allocator, " -n '__fish_seen_subcommand_from {s}'", .{node.chain[node.chain.len - 1]});
        for (node.parser.arguments.items) |argument| {
            if (argument.hide or argument.is_positional()) continue;
            var spelled = std.ArrayList([]const u8).empty;
            for (argument.names) |flag| {
                if (try fish_flag(allocator, flag)) |word| try spelled.append(allocator, word);
            }
            if (spelled.items.len == 0) continue;
            var detail: []const u8 = "";
            if (argument.help) |help| {
                if (help.len != 0) detail = try std.fmt.allocPrint(allocator, " -d '{s}'", .{try quote(allocator, help)});
            }
            try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "complete -c {s}{s} {s}{s}\n", .{
                name,
                condition,
                try std.mem.join(allocator, " ", spelled.items),
                detail,
            }));
        }
        if (node.parser.subparsers) |subparsers| {
            for (subparsers.commands.items) |command| {
                var detail: []const u8 = "";
                if (command.parser.about) |about| {
                    if (about.len != 0) detail = try std.fmt.allocPrint(allocator, " -d '{s}'", .{try quote(allocator, about)});
                }
                try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "complete -c {s}{s} -f -a {s}{s}\n", .{
                    name,
                    condition,
                    command.name,
                    detail,
                }));
            }
        }
        var spelled = std.ArrayList([]const u8).empty;
        for (node.parser.positionals.items) |argument| {
            for (try argument.choice_values(allocator)) |value| {
                const text = try value.text(allocator);
                if (!has_space(text)) try spelled.append(allocator, text);
            }
        }
        if (spelled.items.len != 0) {
            try out.appendSlice(allocator, try std.fmt.allocPrint(allocator, "complete -c {s}{s} -f -a '{s}'\n", .{
                name,
                condition,
                try std.mem.join(allocator, " ", spelled.items),
            }));
        }
    }
    return out.toOwnedSlice(allocator);
}

/// fish 的单选项拼法: `-h` → `-s h`, `--help` → `-l help`; 其余形态不补。
fn fish_flag(allocator: std.mem.Allocator, flag: []const u8) !?[]const u8 {
    if (std.mem.startsWith(u8, flag, "--") and flag.len > 2) {
        return try std.fmt.allocPrint(allocator, "-l {s}", .{flag[2..]});
    }
    if (flag.len == 2 and flag[0] == '-') {
        return try std.fmt.allocPrint(allocator, "-s {s}", .{flag[1..]});
    }
    return null;
}

/// 把说明文字放进单引号: 反斜杠与单引号各转义一层。
fn quote(allocator: std.mem.Allocator, text: []const u8) ![]const u8 {
    var out = std.ArrayList(u8).empty;
    for (text) |ch| {
        if (ch == '\\' or ch == '\'') try out.append(allocator, '\\');
        try out.append(allocator, ch);
    }
    return out.toOwnedSlice(allocator);
}
