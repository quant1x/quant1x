// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 命令定义与解析: 选项匹配、位置参数分配、子命令分发、缺省值与校验。
//!
//! 解析是单趟左到右的扫描: 遇到选项就地取走它的取值, 其余 token 攒起来交给位置参数;
//! 遇到子命令名则把剩余 token 整段交给子命令, 此前攒下的 token 仍归本命令的位置参数。
//!
//! 定义与解析结果的字符串都由解析器自己的 arena 持有, 有效期到 `deinit` 为止。

const std = @import("std");
const spec = @import("spec.zig");
const err = @import("error.zig");

/// 着色的时机, 对应 Python 的 `color` 参数。
pub const ColorChoice = enum { auto, always, never };

/// 把错误记进 run 并返回控制流信号。
fn fail(run: *Run, outcome: err.Outcome) error{Failure} {
    run.failure = outcome;
    return error.Failure;
}

/// 选项名到参数定义的映射项。
const NameEntry = struct {
    name: []const u8,
    argument: *spec.Argument,
};

/// 未匹配的 token 与其扫描序号; 序号只用于还原命令行顺序。
const Extra = struct {
    slot: usize,
    token: []const u8,
};

/// `@file` 展开的嵌套层数上限; 只用来挡住自引用, 正常用法远到不了。
const max_include_depth: usize = 16;

/// 按 LF 拆行, 行尾的 CR 去掉, 末尾那个空元素不算一行。
fn split_lines(allocator: std.mem.Allocator, body: []const u8) ![]const []const u8 {
    var lines = std.ArrayList([]const u8).empty;
    var start: usize = 0;
    while (std.mem.indexOfScalarPos(u8, body, start, '\n')) |found| {
        try lines.append(allocator, strip_cr(body[start..found]));
        start = found + 1;
    }
    if (start < body.len) {
        try lines.append(allocator, strip_cr(body[start..]));
    }
    return lines.toOwnedSlice(allocator);
}

/// 去掉行尾一个 CR。
fn strip_cr(line: []const u8) []const u8 {
    if (line.len != 0 and line[line.len - 1] == '\r') return line[0 .. line.len - 1];
    return line;
}

/// 帮助里的一个分组的标题与说明。
pub const GroupInfo = struct {
    title: []const u8,
    description: ?[]const u8 = null,
};

/// 一次解析的共享状态: 命名空间、出现记录、参与的命令链与待报缺口。
pub const Run = struct {
    allocator: std.mem.Allocator,
    tolerant: bool = false,
    namespace: spec.Namespace = .{},
    present: std.ArrayList([]const u8) = .empty,
    parsers: std.ArrayList(*Parser) = .empty,
    missing: std.ArrayList(*spec.Argument) = .empty,
    extras: std.ArrayList(Extra) = .empty,
    subcommand: []const u8 = "",
    failure: err.Outcome = .{ .ok = .{} },
    environ: ?*const std.process.Environ.Map = null,

    /// 该 dest 是否出现过。
    pub fn has(self: *Run, dest: []const u8) bool {
        for (self.present.items) |item| {
            if (std.mem.eql(u8, item, dest)) return true;
        }
        return false;
    }

    /// 记下一次出现。
    pub fn mark(self: *Run, dest: []const u8) !void {
        if (self.has(dest)) return;
        try self.present.append(self.allocator, dest);
    }
};

/// 互斥组: 组内参数最多出现一个; `required` 为真时至少出现一个。
pub const MutexGroup = struct {
    parser: *Parser,
    required: bool = false,
    arguments: std.ArrayList(*spec.Argument) = .empty,

    /// 登记一个参数并纳入本互斥组。
    pub fn add_argument(self: *MutexGroup, names: []const []const u8, argument: spec.Argument) !*spec.Argument {
        const registered = try self.parser.add_argument(names, argument);
        try self.arguments.append(self.parser.mem(), registered);
        return registered;
    }
};

/// 多值组: 组内参数可以同时出现; `required` 为真时至少要出现一个。
pub const ValueGroup = struct {
    parser: *Parser,
    name: []const u8,
    required: bool = false,
    arguments: std.ArrayList(*spec.Argument) = .empty,

    /// 登记一个参数并纳入本组。
    pub fn add_argument(self: *ValueGroup, names: []const []const u8, argument: spec.Argument) !*spec.Argument {
        const registered = try self.parser.add_argument(names, argument);
        try self.arguments.append(self.parser.mem(), registered);
        return registered;
    }
};

/// 帮助里的一个参数分组, 只影响渲染不影响解析。
pub const ArgumentGroup = struct {
    parser: *Parser,
    title: []const u8,
    description: ?[]const u8 = null,

    /// 登记一个参数并把它归到本组。
    pub fn add_argument(self: *ArgumentGroup, names: []const []const u8, argument: spec.Argument) !*spec.Argument {
        var declared = argument;
        declared.group = self.title;
        return self.parser.add_argument(names, declared);
    }
};

/// 一条已登记的子命令。
pub const Command = struct {
    name: []const u8,
    aliases: []const []const u8 = &.{},
    visible_aliases: []const []const u8 = &.{},
    parser: *Parser,
};

/// 子命令集合: 名字 (含别名) 到命令的映射, 并记录帮助里的展示顺序。
pub const SubParsers = struct {
    parent: *Parser,
    dest: []const u8 = "",
    title: []const u8 = "Commands",
    help: []const u8 = "",
    required: bool = false,
    commands: std.ArrayList(Command) = .empty,

    /// 登记一条子命令; `aliases` 不出现在帮助里, `visible_aliases` 出现。
    pub fn add_parser(
        self: *SubParsers,
        name: []const u8,
        about: []const u8,
        aliases: []const []const u8,
        visible_aliases: []const []const u8,
    ) !*Parser {
        const allocator = self.parent.mem();
        const prog = try std.fmt.allocPrint(allocator, "{s} {s}", .{ self.parent.prog, name });
        const child = try Parser.init(self.parent.gpa, prog);
        child.name = try allocator.dupe(u8, name);
        if (about.len != 0) child.about = try allocator.dupe(u8, about);
        const inherited = self.parent.version();
        if (inherited.len != 0) {
            child.version_value = try allocator.dupe(u8, inherited);
            try child.register_version();
        }
        child.allow_abbrev = self.parent.allow_abbrev;
        child.color = self.parent.color;
        child.max_width = self.parent.max_width;
        child.auto_width = self.parent.auto_width;
        child.parent = self.parent;
        if (self.has_command(name)) return error.InvalidDefinition;
        for (aliases) |alias| {
            if (self.has_command(alias)) return error.InvalidDefinition;
        }
        for (visible_aliases) |alias| {
            if (self.has_command(alias)) return error.InvalidDefinition;
        }
        try self.commands.append(allocator, .{
            .name = child.name,
            .aliases = try allocator.dupe([]const u8, aliases),
            .visible_aliases = try allocator.dupe([]const u8, visible_aliases),
            .parser = child,
        });
        return child;
    }

    /// 按名字或别名找子命令。
    pub fn lookup(self: *SubParsers, name: []const u8) ?*Parser {
        for (self.commands.items) |command| {
            if (std.mem.eql(u8, command.name, name)) return command.parser;
            for (command.aliases) |alias| {
                if (std.mem.eql(u8, alias, name)) return command.parser;
            }
            for (command.visible_aliases) |alias| {
                if (std.mem.eql(u8, alias, name)) return command.parser;
            }
        }
        return null;
    }

    /// 该名字 (含别名) 是否已被登记成子命令。
    pub fn has_command(self: *SubParsers, name: []const u8) bool {
        return self.lookup(name) != null;
    }

    /// 名字归一成注册时的正式名 (别名解析回正名)。
    pub fn canonical(self: *SubParsers, name: []const u8) []const u8 {
        if (self.lookup(name)) |command| {
            if (command.name.len != 0) return command.name;
        }
        return name;
    }
};

/// `@file` 展开要的两样东西: 前缀字符集与读文件用的 `Io`。
pub const Include = struct {
    chars: []const u8,
    io: std.Io,
};

/// 一条命令: 参数定义、解析入口与 usage 片段。子命令本身也是 `Parser`。
pub const Parser = struct {
    gpa: std.mem.Allocator,
    arena: std.heap.ArenaAllocator,
    prog: []const u8 = "",
    about: ?[]const u8 = null,
    long_about: ?[]const u8 = null,
    version_value: ?[]const u8 = null,
    epilog: ?[]const u8 = null,
    usage_text: ?[]const u8 = null,
    allow_abbrev: bool = true,
    argument_default: spec.DefaultValue = .missing,
    color: ColorChoice = .auto,
    max_width: u32 = 100,
    /// 是否按 `COLUMNS` 取帮助宽度。
    auto_width: bool = false,
    subcommand_negates_reqs: bool = false,
    /// `@file` 展开的前缀字符集与读文件用的 `Io`; `null` 表示不展开。
    include: ?Include = null,
    arguments: std.ArrayList(*spec.Argument) = .empty,
    positionals: std.ArrayList(*spec.Argument) = .empty,
    names: std.ArrayList(NameEntry) = .empty,
    dests: std.ArrayList(*spec.Argument) = .empty,
    groups: std.ArrayList(GroupInfo) = .empty,
    mutex: std.ArrayList(*MutexGroup) = .empty,
    value_groups: std.ArrayList(*ValueGroup) = .empty,
    subparsers: ?*SubParsers = null,
    parent: ?*Parser = null,
    name: []const u8 = "",
    /// 环境变量兜底用的映射; Zig 侧由调用方注入, 不设则 `env` 不生效。
    environ: ?*const std.process.Environ.Map = null,

    /// 起一条命令; 缺省带 `-h` / `--help`。
    pub fn init(gpa: std.mem.Allocator, prog: []const u8) !*Parser {
        const self = try gpa.create(Parser);
        self.* = .{ .gpa = gpa, .arena = std.heap.ArenaAllocator.init(gpa) };
        const allocator = self.mem();
        self.prog = try allocator.dupe(u8, prog);
        try self.push(.{
            .names = try allocator.dupe([]const u8, &.{ "-h", "--help" }),
            .dest = "help",
            .action = .help,
            .nargs = spec.Nargs.fixed(0),
            .default_value = .suppress,
            .help = "Print help",
        });
        return self;
    }

    /// 释放定义与解析结果, 连同整棵子命令树。
    pub fn deinit(self: *Parser) void {
        if (self.subparsers) |subparsers| {
            for (subparsers.commands.items) |command| command.parser.deinit();
        }
        self.arena.deinit();
        self.gpa.destroy(self);
    }

    /// 本解析器所有内部字符串的分配器。
    pub fn mem(self: *Parser) std.mem.Allocator {
        return self.arena.allocator();
    }

    /// 设置一段简介。
    pub fn with_about(self: *Parser, about: []const u8) !*Parser {
        self.about = try self.mem().dupe(u8, about);
        return self;
    }

    /// 设置长简介 (`--help` 用)。
    pub fn with_long_about(self: *Parser, about: []const u8) !*Parser {
        self.long_about = try self.mem().dupe(u8, about);
        return self;
    }

    /// 设置版本号并登记 `-V` / `--version`; 子命令没自带时沿用父命令的。
    pub fn with_version(self: *Parser, text: []const u8) !*Parser {
        self.version_value = try self.mem().dupe(u8, text);
        try self.register_version();
        return self;
    }

    /// 设置尾注。
    pub fn with_epilog(self: *Parser, epilog: []const u8) !*Parser {
        self.epilog = try self.mem().dupe(u8, epilog);
        return self;
    }

    /// 用给定文本替换 usage 行里的命令片段。
    pub fn with_usage(self: *Parser, usage: []const u8) !*Parser {
        self.usage_text = try self.mem().dupe(u8, usage);
        return self;
    }

    /// 关掉长选项前缀匹配。
    pub fn without_abbrev(self: *Parser) *Parser {
        self.allow_abbrev = false;
        return self;
    }

    /// 设置所有参数的兜底默认值。
    pub fn with_argument_default(self: *Parser, value: spec.DefaultValue) *Parser {
        self.argument_default = value;
        return self;
    }

    /// 设置着色的时机。
    pub fn with_color(self: *Parser, color: ColorChoice) *Parser {
        self.color = color;
        return self;
    }

    /// 注入环境变量映射; `env` 兜底与 `NO_COLOR` 都从这里读。
    pub fn with_environ(self: *Parser, map: *const std.process.Environ.Map) *Parser {
        self.environ = map;
        return self;
    }

    /// 设置帮助的折行宽度。
    pub fn with_max_width(self: *Parser, limit: u32) *Parser {
        self.max_width = limit;
        return self;
    }

    /// 帮助宽度按 `COLUMNS` 取; 取不到或非法时仍用 `max_width`。
    pub fn with_auto_width(self: *Parser) *Parser {
        self.auto_width = true;
        return self;
    }

    /// 把 `@file` 形态的 token 展开成文件里的行; 传空串等于不展开。
    ///
    /// `io` 只用于读文件 —— 0.17 起标准库的文件读写都要 `Io`, 故与前缀一起登记。
    pub fn with_fromfile_prefix_chars(self: *Parser, chars: []const u8, io: std.Io) !*Parser {
        self.include = if (chars.len == 0)
            null
        else
            .{ .chars = try self.mem().dupe(u8, chars), .io = io };
        return self;
    }

    /// 帮助的生效宽度。
    pub fn width(self: *Parser) u32 {
        if (self.auto_width) {
            if (self.environ) |map| {
                if (map.get("COLUMNS")) |raw| {
                    const text = std.mem.trim(u8, raw, " \t");
                    if (std.fmt.parseInt(u32, text, 10)) |parsed| {
                        if (parsed > 0) return parsed;
                    } else |_| {}
                }
            }
        }
        return self.max_width;
    }

    /// 子命令出现时不再要求本命令的必填参数。
    pub fn with_subcommand_negates_reqs(self: *Parser, flag: bool) *Parser {
        self.subcommand_negates_reqs = flag;
        return self;
    }

    /// 有版本号就登记 `-V` / `--version`; 已占住 `version` 这个 dest 的不再重复登记。
    fn register_version(self: *Parser) !void {
        if (self.version_value == null) return;
        if (self.dest_of("version") != null) return;
        const allocator = self.mem();
        try self.push(.{
            .names = try allocator.dupe([]const u8, &.{ "-V", "--version" }),
            .dest = "version",
            .action = .version,
            .nargs = spec.Nargs.fixed(0),
            .default_value = .suppress,
            .help = "Print version",
        });
    }

    /// 登记一条已构造好的参数定义, 不做定义期校验。
    fn push(self: *Parser, argument: spec.Argument) !void {
        const allocator = self.mem();
        const stored = try allocator.create(spec.Argument);
        stored.* = argument;
        try self.arguments.append(allocator, stored);
        try self.dests.append(allocator, stored);
        var all = std.ArrayList([]const u8).empty;
        try all.appendSlice(allocator, stored.names);
        try all.appendSlice(allocator, stored.alias);
        try all.appendSlice(allocator, stored.visible_alias);
        for (all.items) |name| {
            try self.names.append(allocator, .{ .name = name, .argument = stored });
        }
        if (stored.is_positional()) {
            try self.positionals.append(allocator, stored);
        }
    }

    /// 按名字找已登记的参数。
    fn name_of(self: *Parser, name: []const u8) ?*spec.Argument {
        for (self.names.items) |entry| {
            if (std.mem.eql(u8, entry.name, name)) return entry.argument;
        }
        return null;
    }

    /// 按 `dest` 找已登记的参数。
    fn dest_of(self: *Parser, dest: []const u8) ?*spec.Argument {
        for (self.dests.items) |argument| {
            if (std.mem.eql(u8, argument.dest, dest)) return argument;
        }
        return null;
    }

    /// 登记一个参数: 首个名字以 `-` 开头是选项, 否则是位置参数。
    ///
    /// 定义期错误报 `error.InvalidDefinition`。
    pub fn add_argument(self: *Parser, names: []const []const u8, argument: spec.Argument) !*spec.Argument {
        if (names.len == 0) return error.InvalidDefinition;
        const allocator = self.mem();
        const positional = !std.mem.startsWith(u8, names[0], "-");
        if (positional) {
            if (names.len > 1) return error.InvalidDefinition;
            for (self.positionals.items) |existing| {
                if (existing.nargs.is_remainder()) return error.InvalidDefinition;
            }
            if (argument.required) return error.InvalidDefinition;
        }
        var declared = argument;
        var all = std.ArrayList([]const u8).empty;
        if (!positional) try all.appendSlice(allocator, names);
        try all.appendSlice(allocator, argument.alias);
        try all.appendSlice(allocator, argument.visible_alias);
        for (all.items) |name| {
            if (!std.mem.startsWith(u8, name, "-")) return error.InvalidDefinition;
            if (is_signed_number(name)) return error.InvalidDefinition;
        }
        const dest = if (argument.dest.len != 0)
            argument.dest
        else if (positional)
            try replace_char(allocator, names[0], '-', '_')
        else
            try dest_of_option(allocator, names);

        const action = argument.action;
        var nargs = argument.nargs;
        var const_value = argument.const_value;
        var default_value = argument.default_value;
        if (!action.takes_values()) {
            const zero_or_one = nargs.kind == .one or (nargs.kind == .fixed and nargs.count == 0);
            if (!zero_or_one) return error.InvalidDefinition;
            nargs = spec.Nargs.fixed(0);
            switch (action) {
                .store_true, .store_false => {
                    if (const_value.is_missing()) {
                        const_value = .{ .value = .{ .boolean = action == .store_true } };
                    }
                    if (default_value.is_missing()) {
                        const enabled = const_value.get();
                        const flag = if (enabled) |item| switch (item) {
                            .boolean => |state| state,
                            else => false,
                        } else false;
                        default_value = .{ .value = .{ .boolean = !flag } };
                    }
                },
                .count => {
                    if (default_value.is_missing()) {
                        default_value = .{ .value = .{ .integer = 0 } };
                    }
                },
                .store_const, .append_const => {
                    if (const_value.is_missing()) return error.InvalidDefinition;
                },
                else => {},
            }
        } else if (argument.value_delimiter != null and nargs.kind == .one) {
            nargs = spec.Nargs.fixed(1);
        }
        if (declared.value_parser == null and action.takes_values() and !default_value.is_missing()) {
            declared.value_parser = infer_parser(default_value);
        }
        for (all.items) |name| {
            if (self.name_of(name) != null) return error.InvalidDefinition;
        }
        if (self.dest_of(dest) != null) return error.InvalidDefinition;

        declared.names = if (positional) &.{} else names;
        declared.dest = dest;
        declared.nargs = nargs;
        declared.default_value = default_value;
        declared.const_value = const_value;
        try self.push(declared);
        return self.arguments.items[self.arguments.items.len - 1];
    }

    /// 登记一个帮助分组。
    pub fn add_group(self: *Parser, title: []const u8, description: ?[]const u8) !*ArgumentGroup {
        const allocator = self.mem();
        const stored_description = if (description) |value| try allocator.dupe(u8, value) else null;
        try self.groups.append(allocator, .{
            .title = try allocator.dupe(u8, title),
            .description = stored_description,
        });
        const group = try allocator.create(ArgumentGroup);
        group.* = .{
            .parser = self,
            .title = try allocator.dupe(u8, title),
            .description = stored_description,
        };
        return group;
    }

    /// 登记一个互斥组。
    pub fn add_mutually_exclusive_group(self: *Parser, required: bool) !*MutexGroup {
        const allocator = self.mem();
        const group = try allocator.create(MutexGroup);
        group.* = .{ .parser = self, .required = required };
        try self.mutex.append(allocator, group);
        return group;
    }

    /// 登记一个多值组: 组内参数可同时出现, `required` 表示至少出现一个。
    pub fn add_value_group(self: *Parser, name: []const u8, required: bool) !*ValueGroup {
        const allocator = self.mem();
        const group = try allocator.create(ValueGroup);
        group.* = .{ .parser = self, .name = try allocator.dupe(u8, name), .required = required };
        try self.value_groups.append(allocator, group);
        return group;
    }

    /// 登记子命令集合; `dest` 给出时把命中 (含别名) 的原样写进命名空间。
    pub fn add_subparsers(self: *Parser, dest: []const u8, title: []const u8, required: bool) !*SubParsers {
        if (self.subparsers != null) return error.InvalidDefinition;
        const allocator = self.mem();
        const subparsers = try allocator.create(SubParsers);
        subparsers.* = .{
            .parent = self,
            .dest = try allocator.dupe(u8, dest),
            .title = if (title.len != 0) try allocator.dupe(u8, title) else "Commands",
            .required = required,
        };
        self.subparsers = subparsers;
        return subparsers;
    }

    /// 版本号; 子命令没自带时沿用父命令的。
    pub fn version(self: *Parser) []const u8 {
        if (self.version_value) |value| return value;
        if (self.parent) |parent| return parent.version();
        return "";
    }

    /// 版本行的正文: `<prog> <version>`。
    pub fn version_text(self: *Parser) ![]const u8 {
        const inherited = self.version();
        if (inherited.len == 0) return self.prog;
        return std.fmt.allocPrint(self.mem(), "{s} {s}", .{ self.prog, inherited });
    }

    /// 是否着色。
    pub fn color_enabled(self: *Parser) bool {
        switch (self.color) {
            .always => return true,
            .never => return false,
            .auto => {},
        }
        if (self.environ) |map| {
            if (map.get("NO_COLOR") != null) return false;
        }
        const io = std.Io.Threaded.global_single_threaded.io();
        return std.Io.File.stdout().isTty(io) catch false;
    }

    /// `-` / `--` 与签名数字以外的 `-` 开头 token 是选项。
    pub fn is_option_token(token: []const u8) bool {
        if (std.mem.eql(u8, token, "--")) return true;
        if (token.len == 0 or token[0] != '-' or std.mem.eql(u8, token, "-")) return false;
        return !is_signed_number(token);
    }

    /// 不再自动登记 `-h` / `--help`。
    pub fn without_help(self: *Parser) !*Parser {
        try self.drop_arguments(.help);
        return self;
    }

    /// 从各张表里摘掉某个动作的参数定义。
    fn drop_arguments(self: *Parser, action: spec.Action) !void {
        const allocator = self.mem();
        var arguments = std.ArrayList(*spec.Argument).empty;
        for (self.arguments.items) |argument| {
            if (argument.action != action) try arguments.append(allocator, argument);
        }
        self.arguments = arguments;
        var dests = std.ArrayList(*spec.Argument).empty;
        for (self.dests.items) |argument| {
            if (argument.action != action) try dests.append(allocator, argument);
        }
        self.dests = dests;
        var names = std.ArrayList(NameEntry).empty;
        for (self.names.items) |entry| {
            if (entry.argument.action != action) try names.append(allocator, entry);
        }
        self.names = names;
        var positionals = std.ArrayList(*spec.Argument).empty;
        for (self.positionals.items) |argument| {
            if (argument.action != action) try positionals.append(allocator, argument);
        }
        self.positionals = positionals;
    }

    /// 整段帮助文本 (不含末尾换行)。
    pub fn format_help(self: *Parser, long: bool) ![]const u8 {
        return @import("help.zig").render_help(self.mem(), self, long);
    }

    /// 单行用法文本 (`Usage:` 前缀在内)。
    pub fn format_usage(self: *Parser) ![]const u8 {
        return @import("help.zig").render_usage(self.mem(), self);
    }

    /// 把帮助写到流, 末尾补一个换行。
    pub fn print_help(self: *Parser, writer: *std.Io.Writer, long: bool) !void {
        try writer.print("{s}\n", .{try self.format_help(long)});
    }

    /// usage 行 (`Usage:` 之后) 的字符串形态。
    pub fn usage_spec(self: *Parser) ![]const u8 {
        if (self.usage_text) |value| return value;
        const allocator = self.mem();
        var parts = std.ArrayList([]const u8).empty;
        try parts.append(allocator, self.prog);
        for (self.arguments.items) |argument| {
            if (argument.required and !argument.hide) {
                try parts.append(allocator, try argument.error_name(allocator));
            }
        }
        var has_optional = false;
        for (self.arguments.items) |argument| {
            if (!argument.hide and !argument.required and !argument.is_positional() and
                argument.action != .help and argument.action != .version)
            {
                has_optional = true;
                break;
            }
        }
        if (has_optional) try parts.append(allocator, "[OPTIONS]");
        for (self.positionals.items) |argument| {
            if (!argument.hide) try parts.append(allocator, try argument.usage_spec(allocator));
        }
        if (self.subparsers) |subparsers| {
            try parts.append(allocator, if (subparsers.required) "<COMMAND>" else "[COMMAND]");
        }
        var body = std.ArrayList(u8).empty;
        for (parts.items, 0..) |part, index| {
            if (index != 0) try body.append(allocator, ' ');
            try body.appendSlice(allocator, part);
        }
        return body.toOwnedSlice(allocator);
    }

    /// `parse_known` 的结果: 命名空间与认不出的 token (按命令行顺序)。
    pub const KnownOutcome = union(enum) {
        ok: struct {
            namespace: spec.Namespace,
            extras: []const []const u8,
        },
        failure: err.Outcome,
    };

    /// 解析命令行; 输入非法给出错误, 请求帮助 / 版本给出对应变体。
    pub fn parse(self: *Parser, argv: []const []const u8) !err.Outcome {
        var run = Run{ .allocator = self.mem(), .tolerant = false, .environ = self.environ };
        self.execute(&run, argv) catch |failure| {
            if (failure == error.Failure) return run.failure;
            return failure;
        };
        return .{ .ok = run.namespace };
    }

    /// 同 `parse`, 但认不出的 token 收进返回值而不报错。
    pub fn parse_known(self: *Parser, argv: []const []const u8) !KnownOutcome {
        var run = Run{ .allocator = self.mem(), .tolerant = true, .environ = self.environ };
        self.execute(&run, argv) catch |failure| {
            if (failure == error.Failure) return .{ .failure = run.failure };
            return failure;
        };
        std.mem.sort(Extra, run.extras.items, {}, less_than_slot);
        const extras = try self.mem().alloc([]const u8, run.extras.items.len);
        for (run.extras.items, 0..) |item, index| {
            extras[index] = item.token;
        }
        return .{ .ok = .{ .namespace = run.namespace, .extras = extras } };
    }

    /// `parse_or_exit` 的结果: 成功给出命名空间, 其余情况只需按退出码退出。
    pub const ExitResult = union(enum) {
        ok: spec.Namespace,
        exit: u8,

        /// 进程退出码: 成功 0, 帮助 / 版本 0, 用法错误 2。
        pub fn exit_code(self: ExitResult) u8 {
            return switch (self) {
                .ok => 0,
                .exit => |code| code,
            };
        }
    };

    /// 解析并落到进程出口: 帮助 / 版本写 `out` 退出码 0, 用法错误写 `err` 退出码 2。
    ///
    /// 定义期错误照 Python 侧一样直接 panic, 不退化成一次正常退出。
    pub fn parse_or_exit(
        self: *Parser,
        argv: []const []const u8,
        out: *std.Io.Writer,
        err_writer: *std.Io.Writer,
    ) !ExitResult {
        switch (try self.parse(argv)) {
            .ok => |namespace| return .{ .ok = namespace },
            .help => |requested| {
                const target = self.parser_by_prog(requested.prog) orelse self;
                try target.print_help(out, requested.long_form);
                return .{ .exit = 0 };
            },
            .version => |requested| {
                try out.print("{s}\n", .{requested.text});
                return .{ .exit = 0 };
            },
            .usage => |failure| {
                try err_writer.print("{s}\n", .{try failure.render(self.mem())});
                return .{ .exit = failure.exit_code() };
            },
            .definition => |failure| std.debug.panic("定义期错误: {s}", .{failure.message}),
        }
    }

    /// 按命令全名定位到命令本身, 供帮助渲染找到请求帮助的那条子命令。
    fn parser_by_prog(self: *Parser, prog: []const u8) ?*Parser {
        if (std.mem.eql(u8, self.prog, prog)) return self;
        const subparsers = self.subparsers orelse return null;
        for (subparsers.commands.items) |command| {
            if (command.parser.parser_by_prog(prog)) |found| return found;
        }
        return null;
    }

    /// 两个解析入口的共用骨架。
    fn execute(self: *Parser, run: *Run, argv: []const []const u8) !void {
        const tokens = try self.expand_files(run, argv, 0);
        try self.parse_tokens(run, tokens, &.{}, 0);
        try finish(run);
    }

    /// `@file` 展开: 每行原样作一个 token (不拆词、不去空白、不认注释), 展开结果里再出现的
    /// 前缀继续展开; 打不开文件与嵌套过深都按用法错误报。
    fn expand_files(
        self: *Parser,
        run: *Run,
        tokens: []const []const u8,
        depth: usize,
    ) ![]const []const u8 {
        const include = self.include orelse return tokens;
        if (include.chars.len == 0) return tokens;
        const allocator = run.allocator;
        var expanded = std.ArrayList([]const u8).empty;
        for (tokens) |token| {
            if (token.len == 0 or std.mem.indexOfScalar(u8, include.chars, token[0]) == null) {
                try expanded.append(allocator, token);
                continue;
            }
            const path = token[1..];
            if (depth >= max_include_depth) {
                const message = try std.fmt.allocPrint(
                    allocator,
                    "@file nesting too deep: '{s}'",
                    .{path},
                );
                return fail(run, try self.make_error(message, .other));
            }
            const body = std.Io.Dir.cwd().readFileAlloc(
                include.io,
                path,
                allocator,
                .unlimited,
            ) catch {
                const message = try std.fmt.allocPrint(
                    allocator,
                    "unable to open file '{s}'",
                    .{path},
                );
                return fail(run, try self.make_error(message, .other));
            };
            const lines = try split_lines(allocator, body);
            const nested = try self.expand_files(run, lines, depth + 1);
            try expanded.appendSlice(allocator, nested);
        }
        return expanded.toOwnedSlice(allocator);
    }

    /// 扫描一段 token; 命中子命令时把剩余 token 整段交给子命令。
    fn parse_tokens(self: *Parser, run: *Run, tokens: []const []const u8, chain: []const *Parser, base: usize) !void {
        const allocator = run.allocator;
        try run.parsers.append(allocator, self);
        var pending = std.ArrayList(Extra).empty;
        var stream = tokens;
        var index: usize = 0;
        while (index < stream.len) {
            const token = stream[index];
            if (std.mem.eql(u8, token, "--")) {
                for (stream[index + 1 ..], index + 1..) |leftover, slot| {
                    try pending.append(allocator, .{ .slot = base + slot, .token = leftover });
                }
                break;
            }
            if (is_option_token(token)) {
                if (try self.expand_cluster(run, token, chain)) |expanded| {
                    var rebuilt = std.ArrayList([]const u8).empty;
                    try rebuilt.appendSlice(allocator, stream[0..index]);
                    try rebuilt.appendSlice(allocator, expanded);
                    try rebuilt.appendSlice(allocator, stream[index + 1 ..]);
                    stream = rebuilt.items;
                    continue;
                }
                index = try self.consume_option(run, stream, index, chain, base);
                continue;
            }
            if (self.remainder_ready(pending.items.len)) {
                for (stream[index..], index..) |leftover, slot| {
                    try pending.append(allocator, .{ .slot = base + slot, .token = leftover });
                }
                break;
            }
            if (self.help_subcommand_of(token, pending.items.len)) {
                const target = try self.help_target(run, stream[index + 1 ..]);
                return fail(run, .{ .help = .{ .long_form = true, .prog = target.prog } });
            }
            if (self.subcommand_of(token, pending.items.len)) |child| {
                run.subcommand = token;
                if (self.subparsers) |subparsers| {
                    if (subparsers.dest.len != 0) {
                        try run.namespace.set(allocator, subparsers.dest, .{ .string = subparsers.canonical(token) });
                    }
                }
                var next_chain = std.ArrayList(*Parser).empty;
                try next_chain.appendSlice(allocator, chain);
                try next_chain.append(allocator, self);
                try child.parse_tokens(run, stream[index + 1 ..], next_chain.items, base + index + 1);
                index = stream.len;
                break;
            }
            try pending.append(allocator, .{ .slot = base + index, .token = token });
            index += 1;
        }
        try self.assign_positionals(run, pending.items);
    }

    /// 短选项聚合 `-abc` 拆成 `-a -b -c`; 拆不动时为空。
    fn expand_cluster(self: *Parser, run: *Run, token: []const u8, chain: []const *Parser) !?[]const []const u8 {
        if (std.mem.startsWith(u8, token, "--") or token.len <= 2 or
            std.mem.indexOfScalar(u8, token, '=') != null)
        {
            return null;
        }
        const allocator = run.allocator;
        const first = (try self.lookup(run, token[0..2], chain)) orelse return null;
        if (first.takes_values()) return null;
        var expanded = std.ArrayList([]const u8).empty;
        var index: usize = 1;
        while (index < token.len) {
            const piece = try allocator.dupe(u8, &.{ '-', token[index] });
            const found = try self.lookup(run, piece, chain);
            if (found == null) {
                // 未认出的尾巴连短横线一起留下, 由后续识别失败报出具体的短选项
                try expanded.append(allocator, try std.fmt.allocPrint(allocator, "-{s}", .{token[index..]}));
                break;
            }
            if (found.?.takes_values()) {
                var rest = token[index + 1 ..];
                if (rest.len != 0 and rest[0] == '=') rest = rest[1..];
                if (rest.len == 0) {
                    try expanded.append(allocator, piece);
                } else {
                    try expanded.append(allocator, try std.fmt.allocPrint(allocator, "{s}={s}", .{ piece, rest }));
                }
                break;
            }
            try expanded.append(allocator, piece);
            index += 1;
        }
        return expanded.items;
    }

    /// 本命令位置参数至少要几个 token 才够填。
    fn positional_minimum(self: *Parser) usize {
        var needed: usize = 0;
        for (self.positionals.items) |argument| {
            needed += argument.nargs.min_count();
        }
        return needed;
    }

    /// token 是子命令名且本命令的位置参数已够填, 才当成子命令。
    fn subcommand_of(self: *Parser, token: []const u8, pending: usize) ?*Parser {
        const subparsers = self.subparsers orelse return null;
        const child = subparsers.lookup(token) orelse return null;
        if (pending < self.positional_minimum()) return null;
        return child;
    }

    /// token 是否命中自动生成的 `help` 子命令。
    fn help_subcommand_of(self: *Parser, token: []const u8, pending: usize) bool {
        if (!std.mem.eql(u8, token, "help")) return false;
        const subparsers = self.subparsers orelse return false;
        if (subparsers.has_command("help")) return false;
        return pending >= self.positional_minimum();
    }

    /// `help a b` 的下沉目标: 按子命令名逐级走, 走到认不出的名字即报错。
    fn help_target(self: *Parser, run: *Run, rest: []const []const u8) !*Parser {
        var target = self;
        for (rest) |token| {
            const child = if (target.subparsers) |subparsers| subparsers.lookup(token) else null;
            if (child == null) {
                const message = try std.fmt.allocPrint(run.allocator, "unrecognized subcommand '{s}'", .{token});
                return fail(run, try target.make_error(message, .unknown_argument));
            }
            target = child.?;
        }
        return target;
    }

    /// REMAINDER 位置参数的前序位置参数已经填够, 后续 token 全归它。
    fn remainder_ready(self: *Parser, pending: usize) bool {
        for (self.positionals.items, 0..) |argument, index| {
            if (!argument.nargs.is_remainder()) continue;
            var needed: usize = 0;
            for (self.positionals.items[0..index]) |earlier| {
                needed += earlier.nargs.min_count();
            }
            return pending >= needed;
        }
        return false;
    }

    /// 先本命令 (含长选项前缀匹配), 再祖先命令上的 `global` 参数。
    fn lookup(self: *Parser, run: *Run, name: []const u8, chain: []const *Parser) !?*spec.Argument {
        if (self.name_of(name)) |argument| return argument;
        if (self.allow_abbrev and std.mem.startsWith(u8, name, "--") and name.len > 2) {
            if (try self.abbreviate(run, name)) |argument| return argument;
        }
        var index = chain.len;
        while (index > 0) {
            index -= 1;
            if (chain[index].name_of(name)) |argument| {
                if (argument.global) return argument;
            }
        }
        return null;
    }

    /// 长选项唯一前缀匹配; 命中多个即报歧义。
    fn abbreviate(self: *Parser, run: *Run, name: []const u8) !?*spec.Argument {
        const allocator = run.allocator;
        var matched = std.ArrayList(*spec.Argument).empty;
        var seen = std.ArrayList([]const u8).empty;
        for (self.names.items) |entry| {
            if (!std.mem.startsWith(u8, entry.name, "--") or !std.mem.startsWith(u8, entry.name, name)) continue;
            var duplicated = false;
            for (seen.items) |dest| {
                if (std.mem.eql(u8, dest, entry.argument.dest)) duplicated = true;
            }
            if (duplicated) continue;
            try seen.append(allocator, entry.argument.dest);
            try matched.append(allocator, entry.argument);
        }
        if (matched.items.len == 0) return null;
        if (matched.items.len > 1) {
            var options = std.ArrayList([]const u8).empty;
            for (self.names.items) |entry| {
                if (!std.mem.startsWith(u8, entry.name, name)) continue;
                var listed = false;
                for (options.items) |option| {
                    if (std.mem.eql(u8, option, entry.name)) listed = true;
                }
                if (!listed) try options.append(allocator, entry.name);
            }
            std.mem.sort([]const u8, options.items, {}, less_than);
            const message = try std.fmt.allocPrint(allocator, "ambiguous argument '{s}'", .{name});
            var failure = try self.make_error(message, .ambiguous_argument);
            const line = try std.fmt.allocPrint(allocator, "could be one of: {s}", .{
                try spec.format_names(allocator, options.items),
            });
            failure.usage.lines = try allocator.dupe([]const u8, &.{line});
            return fail(run, failure);
        }
        return matched.items[0];
    }

    /// 未知选项: 给出相似项建议, 长选项再补一句怎么当取值传。
    fn throw_unknown_argument(self: *Parser, run: *Run, name: []const u8, chain: []const *Parser) !void {
        const allocator = run.allocator;
        var candidates = std.ArrayList([]const u8).empty;
        for (self.names.items) |entry| {
            try candidates.append(allocator, entry.name);
        }
        for (chain) |parent| {
            for (parent.names.items) |entry| {
                if (entry.argument.global) try candidates.append(allocator, entry.name);
            }
        }
        const message = try std.fmt.allocPrint(allocator, "unexpected argument '{s}' found", .{name});
        var failure = try self.make_error(message, .unknown_argument);
        if (try err.best_match(allocator, name, candidates.items)) |suggestion| {
            const tip = try std.fmt.allocPrint(allocator, "a similar argument exists: '{s}'", .{suggestion});
            failure.usage.tips = try allocator.dupe([]const u8, &.{tip});
        } else if (std.mem.startsWith(u8, name, "--")) {
            const tip = try std.fmt.allocPrint(allocator, "to pass '{s}' as a value, use '-- {s}'", .{ name, name });
            failure.usage.tips = try allocator.dupe([]const u8, &.{tip});
        }
        return fail(run, failure);
    }

    /// 处理一个选项 token, 返回下一个待处理下标。
    fn consume_option(
        self: *Parser,
        run: *Run,
        tokens: []const []const u8,
        index: usize,
        chain: []const *Parser,
        base: usize,
    ) !usize {
        const allocator = run.allocator;
        const token = tokens[index];
        const split = split_option(token);
        var argument = try self.lookup(run, split.name, chain);
        var inline_value = split.value;
        if (argument == null) {
            if (try self.attached_short(run, split.name, chain)) |attached| {
                argument = attached.argument;
                inline_value = attached.value;
            } else {
                if (run.tolerant) {
                    try run.extras.append(allocator, .{ .slot = base + index, .token = token });
                    return index + 1;
                }
                try self.throw_unknown_argument(run, split.name, chain);
                unreachable;
            }
        }
        const selected = argument.?;
        if (!selected.takes_values()) {
            if (inline_value) |value| {
                const message = try std.fmt.allocPrint(allocator, "unexpected value '{s}' for '{s}' found; no more were expected", .{
                    value,
                    try selected.error_name(allocator),
                });
                return fail(run, try self.make_error(message, .invalid_value));
            }
            // 长短帮助靠命中的名字区分 (`-h` 走短帮助, `--help` 走长帮助)
            switch (selected.action) {
                .help => return fail(run, .{ .help = .{
                    .long_form = std.mem.startsWith(u8, split.name, "--"),
                    .prog = self.prog,
                } }),
                .version => return fail(run, .{ .version = .{ .text = try self.version_text() } }),
                else => {},
            }
            try self.apply(run, selected, &.{});
            return index + 1;
        }
        if (inline_value) |value| {
            const values = try self.convert_texts(run, selected, &.{value});
            try self.apply(run, selected, values);
            return index + 1;
        }
        const taken = try self.take_values(run, selected, tokens, index + 1);
        const values = try self.convert_texts(run, selected, taken.values);
        try self.apply(run, selected, values);
        return taken.stop;
    }

    /// 短选项粘值 `-ovalue` / `-o=value`: 短选项后面的部分就是它的取值。
    const Attached = struct {
        argument: *spec.Argument,
        value: []const u8,
    };

    /// 短选项粘值 `-ovalue` / `-o=value`: 短选项后面的部分就是它的取值。
    fn attached_short(self: *Parser, run: *Run, name: []const u8, chain: []const *Parser) !?Attached {
        if (std.mem.startsWith(u8, name, "--") or name.len <= 2) return null;
        const argument = (try self.lookup(run, name[0..2], chain)) orelse return null;
        if (!argument.takes_values()) return null;
        var value = name[2..];
        if (value.len != 0 and value[0] == '=') value = value[1..];
        return Attached{ .argument = argument, .value = value };
    }

    /// 按 nargs 取走取值的结果。
    const Taken = struct {
        stop: usize,
        values: []const []const u8,
    };

    /// 按 nargs 取走取值; REMAINDER 取到末尾, 不足最小数时报错。
    fn take_values(
        self: *Parser,
        run: *Run,
        argument: *spec.Argument,
        tokens: []const []const u8,
        start: usize,
    ) !Taken {
        const allocator = run.allocator;
        if (argument.nargs.is_remainder()) {
            return .{ .stop = tokens.len, .values = tokens[start..] };
        }
        const minimum = argument.nargs.min_count();
        const maximum = argument.nargs.max_count();
        var taken = std.ArrayList([]const u8).empty;
        var index = start;
        while (index < tokens.len) {
            if (maximum) |limit| {
                if (taken.items.len >= limit) break;
            }
            if (is_option_token(tokens[index]) and !argument.allow_hyphen_values) break;
            try taken.append(allocator, tokens[index]);
            index += 1;
        }
        if (taken.items.len < minimum) {
            try self.throw_missing_value(run, argument, minimum, taken.items.len);
            unreachable;
        }
        return .{ .stop = index, .values = taken.items };
    }

    /// 取值不足的报错; 单个与多个的文案分开, 与 clap 同形。
    fn throw_missing_value(
        self: *Parser,
        run: *Run,
        argument: *spec.Argument,
        needed: usize,
        given: usize,
    ) !void {
        const allocator = run.allocator;
        const name = try argument.error_name(allocator);
        const message = if (needed <= 1)
            try std.fmt.allocPrint(allocator, "a value is required for '{s}' but none was supplied", .{name})
        else
            try std.fmt.allocPrint(allocator, "{d} values required for '{s}' but {d} {s} provided", .{
                needed,
                name,
                given,
                if (given == 1) "was" else "were",
            });
        return fail(run, try self.make_error(message, .missing_value));
    }

    /// 按 `value_delimiter` 拆分后逐个转换。
    fn convert_texts(self: *Parser, run: *Run, argument: *spec.Argument, raw: []const []const u8) ![]spec.Value {
        const allocator = run.allocator;
        var texts = std.ArrayList([]const u8).empty;
        for (raw) |text| {
            if (argument.value_delimiter) |delimiter| {
                if (delimiter.len == 0) {
                    try texts.append(allocator, text);
                    continue;
                }
                var iterator = std.mem.splitSequence(u8, text, delimiter);
                while (iterator.next()) |piece| {
                    try texts.append(allocator, piece);
                }
                continue;
            }
            try texts.append(allocator, text);
        }
        const values = try allocator.alloc(spec.Value, texts.items.len);
        for (texts.items, 0..) |text, index| {
            values[index] = try self.convert(run, argument, text);
        }
        return values;
    }

    /// 转换单个取值并校验可选值。
    fn convert(self: *Parser, run: *Run, argument: *spec.Argument, text: []const u8) !spec.Value {
        const allocator = run.allocator;
        var value = spec.Value{ .string = text };
        if (argument.value_parser) |parser| {
            var message: []const u8 = "";
            value = parser.parse(allocator, text, &message) catch {
                const failure_message = try std.fmt.allocPrint(allocator, "invalid value '{s}' for '{s}'", .{
                    text,
                    try argument.error_name(allocator),
                });
                var failure = try self.make_error(failure_message, .invalid_value);
                failure.usage.reason = message;
                return fail(run, failure);
            };
        }
        const choices = argument.choices orelse return value;
        const rendered = try value.text(allocator);
        var matched = false;
        for (choices) |choice| {
            if (std.mem.eql(u8, try choice.value.text(allocator), rendered)) matched = true;
        }
        if (matched) return value;

        const failure_message = try std.fmt.allocPrint(allocator, "invalid value '{s}' for '{s}'", .{
            text,
            try argument.error_name(allocator),
        });
        var failure = try self.make_error(failure_message, .invalid_value);
        const line = try std.fmt.allocPrint(allocator, "[possible values: {s}]", .{
            try spec.format_items(allocator, try argument.choice_values(allocator)),
        });
        failure.usage.lines = try allocator.dupe([]const u8, &.{line});
        var labels = std.ArrayList([]const u8).empty;
        for (choices) |choice| {
            try labels.append(allocator, try choice.value.text(allocator));
        }
        if (try err.best_match(allocator, text, labels.items)) |suggestion| {
            const tip = try std.fmt.allocPrint(allocator, "a similar value exists: '{s}'", .{suggestion});
            failure.usage.tips = try allocator.dupe([]const u8, &.{tip});
        }
        return fail(run, failure);
    }

    /// 把一次出现落进命名空间, 按动作决定覆盖 / 追加 / 计数。
    fn apply(self: *Parser, run: *Run, argument: *spec.Argument, values: []spec.Value) !void {
        const allocator = run.allocator;
        switch (argument.action) {
            .help => return fail(run, .{ .help = .{
                .long_form = std.mem.eql(u8, argument.primary_name(), "--help"),
                .prog = self.prog,
            } }),
            .version => return fail(run, .{ .version = .{ .text = try self.version_text() } }),
            else => {},
        }
        try run.mark(argument.dest);
        switch (argument.action) {
            .count => {
                var count: i64 = 0;
                if (run.namespace.get(argument.dest)) |current| {
                    switch (current) {
                        .integer => |number| count = number,
                        else => {},
                    }
                }
                try run.namespace.set(allocator, argument.dest, .{ .integer = count + 1 });
            },
            .store_true, .store_false, .store_const => {
                try run.namespace.set(allocator, argument.dest, argument.const_value.get() orelse .nil);
            },
            .append_const => try append_value(run, argument.dest, argument.const_value.get() orelse .nil),
            .extend => {
                var items = std.ArrayList(spec.Value).empty;
                if (run.namespace.get(argument.dest)) |current| {
                    switch (current) {
                        .list => |existing| try items.appendSlice(allocator, existing),
                        else => {},
                    }
                }
                if (items.items.len == 0) {
                    if (argument.default_value.get()) |fallback| {
                        switch (fallback) {
                            .list => |existing| try items.appendSlice(allocator, existing),
                            else => {},
                        }
                    }
                }
                try items.appendSlice(allocator, values);
                try run.namespace.set(allocator, argument.dest, .{ .list = items.items });
            },
            .append => {
                if (argument.collects()) {
                    try append_value(run, argument.dest, .{ .list = values });
                } else {
                    try append_value(run, argument.dest, if (values.len > 0) values[0] else .nil);
                }
            },
            else => {
                if (values.len == 0) {
                    try run.namespace.set(allocator, argument.dest, argument.const_value.get() orelse .nil);
                } else if (argument.collects()) {
                    try run.namespace.set(allocator, argument.dest, .{ .list = values });
                } else {
                    try run.namespace.set(allocator, argument.dest, values[0]);
                }
            },
        }
    }

    /// 往命名空间里的列表追加一项; 首次出现时新开列表。
    fn append_value(run: *Run, dest: []const u8, item: spec.Value) !void {
        const allocator = run.allocator;
        var items = std.ArrayList(spec.Value).empty;
        if (run.namespace.get(dest)) |current| {
            switch (current) {
                .list => |existing| try items.appendSlice(allocator, existing),
                else => {},
            }
        }
        try items.append(allocator, item);
        try run.namespace.set(allocator, dest, .{ .list = items.items });
    }

    /// 把攒下的非选项 token 按定义顺序分给位置参数。
    fn assign_positionals(self: *Parser, run: *Run, pending: []const Extra) !void {
        const allocator = run.allocator;
        var cursor: usize = 0;
        const total = pending.len;
        for (self.positionals.items, 0..) |argument, index| {
            var tail: usize = 0;
            for (self.positionals.items[index + 1 ..]) |after| {
                tail += after.nargs.min_count();
            }
            const available = if (total - cursor > tail) total - cursor - tail else 0;
            const take: usize = if (argument.nargs.is_remainder())
                total - cursor
            else if (argument.nargs.max_count()) |limit|
                @min(limit, available)
            else
                available;
            if (take < argument.nargs.min_count()) {
                try run.missing.append(allocator, argument);
                continue;
            }
            const chunk = try allocator.alloc([]const u8, take);
            for (0..take) |offset| {
                chunk[offset] = pending[cursor + offset].token;
            }
            cursor += take;
            try self.store_positional(run, argument, chunk);
        }
        for (pending[cursor..]) |leftover| {
            try run.extras.append(allocator, leftover);
        }
    }

    /// 位置参数的落值: 空片段按 const / default / 空列表依次退让。
    fn store_positional(self: *Parser, run: *Run, argument: *spec.Argument, chunk: []const []const u8) !void {
        const allocator = run.allocator;
        if (chunk.len == 0) {
            if (argument.const_value.get()) |constant| {
                try run.namespace.set(allocator, argument.dest, constant);
            } else if (argument.default_value.get()) |fallback| {
                try run.namespace.set(allocator, argument.dest, fallback);
            } else if (argument.nargs.kind == .zero_or_more or argument.nargs.is_remainder()) {
                try run.namespace.set(allocator, argument.dest, .{ .list = &.{} });
            } else {
                try run.namespace.set(allocator, argument.dest, .nil);
            }
            return;
        }
        try run.mark(argument.dest);
        const values = try self.convert_texts(run, argument, chunk);
        if (argument.collects()) {
            try run.namespace.set(allocator, argument.dest, .{ .list = values });
        } else {
            try run.namespace.set(allocator, argument.dest, values[0]);
        }
    }

    /// 解析后收尾: 覆盖 → 环境变量兜底 → 校验 → 填默认值。
    fn finish(run: *Run) !void {
        try apply_overrides(run);
        try apply_env(run);
        try validate(run);
        try apply_defaults(run);
    }

    /// 一对 `overrides_with` 的两个 dest。
    const DestPair = struct {
        first: []const u8,
        second: []const u8,
    };

    /// `overrides_with` 双向生效: 一对里命令行上后出现的留下, 先出现的退回默认值。
    fn apply_overrides(run: *Run) !void {
        const allocator = run.allocator;
        var registry = std.ArrayList(*spec.Argument).empty;
        var named = std.ArrayList(NameEntry).empty;
        for (run.parsers.items) |parser| {
            for (parser.arguments.items) |argument| {
                // 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
                if (argument.action == .help or argument.action == .version) continue;
                try registry.append(allocator, argument);
            }
            for (parser.names.items) |entry| {
                try named.append(allocator, entry);
            }
        }

        var pairs = std.ArrayList(DestPair).empty;
        for (registry.items) |argument| {
            for (argument.overrides_with) |reference| {
                const other = try resolve_reference(run, registry.items, named.items, reference);
                try pairs.append(allocator, .{ .first = argument.dest, .second = other });
            }
        }
        if (pairs.items.len == 0) return;

        var kept = std.ArrayList([]const u8).empty;
        for (run.present.items) |dest| {
            for (pairs.items) |pair| {
                const other: ?[]const u8 = if (std.mem.eql(u8, pair.first, dest))
                    pair.second
                else if (std.mem.eql(u8, pair.second, dest))
                    pair.first
                else
                    null;
                if (other) |name| drop_dest(&kept, name);
            }
            try kept.append(allocator, dest);
        }

        // 落选的那个: 从出现记录与命名空间里摘掉, 收尾时按默认值重填
        var index: usize = 0;
        for (run.present.items) |dest| {
            if (has_dest(kept.items, dest)) {
                run.present.items[index] = dest;
                index += 1;
                continue;
            }
            run.namespace.remove(dest);
        }
        run.present.shrinkRetainingCapacity(index);
    }

    /// 构造带类别、本命令 usage 与帮助提示的用法错误。
    fn make_error(self: *Parser, message: []const u8, kind: err.ErrorKind) !err.Outcome {
        const usage = try self.usage_spec();
        const hint: []const u8 = if (self.dest_of("help") != null) "For more information, try '--help'." else "";
        return .{ .usage = .{
            .message = message,
            .kind = kind,
            .usage = usage,
            .hint = hint,
        } };
    }
};

/// 命令行没出现时读环境变量; 命中即视为"出现过"。
fn apply_env(run: *Run) !void {
    const allocator = run.allocator;
    const map = run.environ orelse return;
    for (run.parsers.items) |parser| {
        for (parser.arguments.items) |argument| {
            const env_name = argument.env orelse continue;
            if (run.has(argument.dest)) continue;
            const text = map.get(env_name) orelse continue;
            if (argument.takes_values()) {
                const values = try parser.convert_texts(run, argument, &.{text});
                try parser.apply(run, argument, values);
                continue;
            }
            var message: []const u8 = "";
            const converted = spec.parse_bool_value(text, &message) catch {
                const failure_message = try std.fmt.allocPrint(allocator, "invalid value '{s}' for '{s}'", .{
                    text,
                    try argument.error_name(allocator),
                });
                var failure = try parser.make_error(failure_message, .invalid_value);
                failure.usage.reason = message;
                return fail(run, failure);
            };
            var fallback = spec.Value{ .boolean = false };
            if (argument.default_value.get()) |declared| fallback = declared;
            const enabled = switch (converted) {
                .boolean => |state| state,
                else => false,
            };
            try run.namespace.set(
                allocator,
                argument.dest,
                if (enabled) (argument.const_value.get() orelse .nil) else fallback,
            );
            try run.mark(argument.dest);
        }
    }
}

/// 该参数这一刻是否算必填: `required`, 或 `required_unless_present` 列出的一个都没出现。
fn needs_argument(
    run: *Run,
    registry: []const *spec.Argument,
    named: []const NameEntry,
    argument: *spec.Argument,
) !bool {
    if (argument.required) return true;
    if (argument.required_unless_present.len == 0) return false;
    for (argument.required_unless_present) |reference| {
        const dest = try resolve_reference(run, registry, named, reference);
        if (run.has(dest)) return false;
    }
    return true;
}

/// `conflicts_with` → 互斥组 → 多值组 → `requires` → 必填; 报出第一处问题。
fn validate(run: *Run) !void {
    const allocator = run.allocator;
    const leaf = run.parsers.items[run.parsers.items.len - 1];
    var registry = std.ArrayList(*spec.Argument).empty;
    var named = std.ArrayList(NameEntry).empty;
    for (run.parsers.items) |parser| {
        for (parser.arguments.items) |argument| {
            // 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
            if (argument.action == .help or argument.action == .version) continue;
            if (find_dest(registry.items, argument.dest) != null) {
                const message = try std.fmt.allocPrint(
                    allocator,
                    "命令链上的 dest 重复: {s} (父命令与子命令不能共用 dest)",
                    .{argument.dest},
                );
                return fail(run, .{ .definition = .{ .message = message } });
            }
            try registry.append(allocator, argument);
        }
        for (parser.names.items) |entry| {
            try named.append(allocator, entry);
        }
    }
    for (registry.items) |argument| {
        if (!run.has(argument.dest)) continue;
        for (argument.conflicts_with) |reference| {
            const dest = try resolve_reference(run, registry.items, named.items, reference);
            if (run.has(dest)) {
                const message = try std.fmt.allocPrint(allocator, "the argument '{s}' cannot be used with '{s}'", .{
                    try argument.error_name(allocator),
                    try label_of(allocator, registry.items, dest),
                });
                return fail(run, try leaf.make_error(message, .conflict));
            }
        }
    }
    for (run.parsers.items) |parser| {
        for (parser.mutex.items) |group| {
            var hits = std.ArrayList(*spec.Argument).empty;
            for (group.arguments.items) |argument| {
                if (run.has(argument.dest)) try hits.append(allocator, argument);
            }
            if (hits.items.len > 1) {
                const message = try std.fmt.allocPrint(allocator, "the argument '{s}' cannot be used with '{s}'", .{
                    try hits.items[0].error_name(allocator),
                    try hits.items[1].error_name(allocator),
                });
                return fail(run, try leaf.make_error(message, .conflict));
            }
            if (hits.items.len == 0 and group.required) {
                var failure = try leaf.make_error("one of the following arguments is required:", .missing_required);
                var lines = std.ArrayList([]const u8).empty;
                for (group.arguments.items) |argument| {
                    try lines.append(allocator, try argument.error_name(allocator));
                }
                failure.usage.lines = lines.items;
                return fail(run, failure);
            }
        }
    }
    for (run.parsers.items) |parser| {
        for (parser.value_groups.items) |group| {
            if (!group.required) continue;
            var hit = false;
            for (group.arguments.items) |argument| {
                if (run.has(argument.dest)) hit = true;
            }
            if (hit) continue;
            var failure = try leaf.make_error("the following required arguments were not provided:", .missing_required);
            const label = try std.fmt.allocPrint(allocator, "<{s}>", .{group.name});
            failure.usage.lines = try allocator.dupe([]const u8, &.{label});
            return fail(run, failure);
        }
    }
    for (registry.items) |argument| {
        if (!run.has(argument.dest)) continue;
        for (argument.requires_args) |reference| {
            const dest = try resolve_reference(run, registry.items, named.items, reference);
            if (!run.has(dest)) {
                const message = try std.fmt.allocPrint(allocator, "the argument '{s}' requires '{s}'", .{
                    try argument.error_name(allocator),
                    try label_of(allocator, registry.items, dest),
                });
                return fail(run, try leaf.make_error(message, .missing_required));
            }
        }
    }
    var missing = std.ArrayList(*spec.Argument).empty;
    try missing.appendSlice(allocator, run.missing.items);
    for (run.parsers.items) |parser| {
        if (parser.subcommand_negates_reqs and run.subcommand.len != 0) continue;
        for (parser.arguments.items) |argument| {
            if (run.has(argument.dest)) continue;
            if (!try needs_argument(run, registry.items, named.items, argument)) continue;
            try missing.append(allocator, argument);
        }
        if (parser == leaf and parser.subparsers != null and parser.subparsers.?.required) {
            var failure = try leaf.make_error("the following required arguments were not provided:", .missing_required);
            failure.usage.lines = try allocator.dupe([]const u8, &.{"<COMMAND>"});
            return fail(run, failure);
        }
    }
    if (missing.items.len > 0) {
        var failure = try leaf.make_error("the following required arguments were not provided:", .missing_required);
        var lines = std.ArrayList([]const u8).empty;
        for (missing.items) |argument| {
            const label = try argument.error_name(allocator);
            var listed = false;
            for (lines.items) |existing| {
                if (std.mem.eql(u8, existing, label)) listed = true;
            }
            if (!listed) try lines.append(allocator, label);
        }
        failure.usage.lines = lines.items;
        return fail(run, failure);
    }
}

/// 没出现在命令行上的参数填默认值; `suppress` 表示不写进命名空间。
fn apply_defaults(run: *Run) !void {
    const allocator = run.allocator;
    for (run.parsers.items) |parser| {
        for (parser.arguments.items) |argument| {
            if (run.namespace.contains(argument.dest)) continue;
            var chosen = argument.default_value;
            if (chosen.is_missing()) chosen = parser.argument_default;
            if (chosen.is_suppress()) continue;
            const value = chosen.get() orelse implicit_default(argument);
            try run.namespace.set(allocator, argument.dest, value);
        }
    }
}

/// 在 registry 里按 dest 找参数。
/// 列表里是否有该 dest。
fn has_dest(items: []const []const u8, dest: []const u8) bool {
    for (items) |item| {
        if (std.mem.eql(u8, item, dest)) return true;
    }
    return false;
}

/// 从列表里摘掉该 dest; 没有时什么也不做。
fn drop_dest(items: *std.ArrayList([]const u8), dest: []const u8) void {
    for (items.items, 0..) |item, index| {
        if (std.mem.eql(u8, item, dest)) {
            _ = items.orderedRemove(index);
            return;
        }
    }
}

/// 按 dest 在注册表里找参数。
fn find_dest(registry: []const *spec.Argument, dest: []const u8) ?*spec.Argument {
    for (registry) |argument| {
        if (std.mem.eql(u8, argument.dest, dest)) return argument;
    }
    return null;
}

/// 把 `conflicts_with` / `requires` 的引用解析成 `dest`。
fn resolve_reference(
    run: *Run,
    registry: []const *spec.Argument,
    named: []const NameEntry,
    reference: []const u8,
) ![]const u8 {
    if (find_dest(registry, reference)) |argument| return argument.dest;
    for (named) |entry| {
        if (std.mem.eql(u8, entry.name, reference)) return entry.argument.dest;
    }
    const message = try std.fmt.allocPrint(
        run.allocator,
        "overrides_with / conflicts_with / requires 指向了未登记的参数: {s}",
        .{reference},
    );
    return fail(run, .{ .definition = .{ .message = message } });
}

/// 取某个 `dest` 的错误文案指代。
fn label_of(allocator: std.mem.Allocator, registry: []const *spec.Argument, dest: []const u8) ![]const u8 {
    const argument = find_dest(registry, dest) orelse return dest;
    return argument.error_name(allocator);
}

/// 排序用的比较。
fn less_than(_: void, left: []const u8, right: []const u8) bool {
    return std.mem.lessThan(u8, left, right);
}

/// 按扫描序号还原命令行顺序。
fn less_than_slot(_: void, left: Extra, right: Extra) bool {
    return left.slot < right.slot;
}

/// 是否全部是十进制数字; 空串视为全部满足。
fn all_digits(text: []const u8) bool {
    for (text) |item| {
        if (item < '0' or item > '9') return false;
    }
    return true;
}

/// 签名数字: 可选的 `+` / `-` 号加整数或小数, 整个 token 当一个取值参数 (`-1`、`-.5`、`+3`、`-1.5`)。
fn is_signed_number(token: []const u8) bool {
    if (token.len < 2 or (token[0] != '-' and token[0] != '+')) return false;
    const body = token[1..];
    if (std.mem.indexOfScalar(u8, body, '.')) |position| {
        if (!all_digits(body[0..position])) return false;
        const fraction = body[position + 1 ..];
        return fraction.len != 0 and all_digits(fraction);
    }
    return all_digits(body);
}

/// 拆分结果的形态。
const SplitOption = struct {
    name: []const u8,
    value: ?[]const u8 = null,
};

/// 把 `--key=value` / `-k=value` 拆成 (名字, 内联取值)。
fn split_option(token: []const u8) SplitOption {
    if (std.mem.indexOfScalar(u8, token, '=')) |position| {
        return .{ .name = token[0..position], .value = token[position + 1 ..] };
    }
    return .{ .name = token };
}

/// 由选项名推 dest: 优先最长形态的长选项, 中划线转下划线。
fn dest_of_option(allocator: std.mem.Allocator, names: []const []const u8) ![]const u8 {
    for (names) |name| {
        if (std.mem.startsWith(u8, name, "--")) {
            return replace_char(allocator, name[2..], '-', '_');
        }
    }
    if (names.len == 0) return "";
    var body = names[0];
    while (body.len != 0 and body[0] == '-') body = body[1..];
    return replace_char(allocator, body, '-', '_');
}

/// 把文本里的 `from` 换成 `to`, 返回新分配的副本。
fn replace_char(allocator: std.mem.Allocator, text: []const u8, from: u8, to: u8) ![]const u8 {
    const buffer = try allocator.dupe(u8, text);
    for (buffer) |*item| {
        if (item.* == from) item.* = to;
    }
    return buffer;
}

/// 没显式给默认值时的落值。
fn implicit_default(argument: *spec.Argument) spec.Value {
    return switch (argument.action) {
        .store_true => .{ .boolean = false },
        .store_false => .{ .boolean = true },
        .count => .{ .integer = 0 },
        else => blk: {
            if (argument.is_positional() and
                (argument.nargs.kind == .zero_or_more or argument.nargs.is_remainder()))
            {
                break :blk .{ .list = &.{} };
            }
            break :blk .nil;
        },
    };
}

/// 按默认值的类型推断取值转换器。
fn infer_parser(value: spec.DefaultValue) ?spec.ValueParser {
    const resolved = value.get() orelse return null;
    return switch (resolved) {
        .boolean => .boolean,
        .integer => .{ .int = .{} },
        .float => .{ .float = .{} },
        else => null,
    };
}
