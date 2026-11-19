// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! spec: quant1x/base/globmatch.py
//!
//! 锚定 glob 匹配 (globstar)。
//!
//! 整条路径从根起逐段比对: `**` 匹配零到多段, 其余段按 `*` / `?` / `[...]`
//! 通配 (`*` 不跨段、大小写敏感、点开头的名字照常匹配)。模式以 `/**` 结尾时要求
//! 目录下至少一个条目, 等价 `/**/*`。
//!
//! 与 Python 3.13 `PurePath.full_match()`、bash `globstar` 同义; 只做匹配,
//! 不接触文件系统。
//!
//! 路径与模式都可写成**项目根绝对形式** (前导 `/`), 与相对形式等价: 匹配一律以
//! 项目根为锚, `/third_party/a.h` 与 `third_party/a.h` 是同一个位置, 两侧可混写。

const std = @import("std");

/// 一个 Unicode 标量及其后的字节位置
const Cp = struct {
    cp: u21,
    next: usize,
};

/// 取 `s[i..]` 首字符; 非法 UTF-8 时按单字节处理
fn nextCp(s: []const u8, i: usize) Cp {
    if (i >= s.len) {
        return .{ .cp = 0, .next = i };
    }
    const n = std.unicode.utf8ByteSequenceLength(s[i]) catch 1;
    if (i + n > s.len) {
        return .{ .cp = s[i], .next = i + 1 };
    }
    const cp = std.unicode.utf8Decode(s[i..][0..n]) catch {
        return .{ .cp = s[i], .next = i + 1 };
    };
    return .{ .cp = cp, .next = i + n };
}

/// 按 `/` 取下一段 (跳过连续分隔符); 没有下一段时为 `null`
const Seg = struct {
    text: []const u8,
    next: usize,
};

fn nextSeg(s: []const u8, pos: usize) ?Seg {
    var i = pos;
    while (i < s.len and s[i] == '/') {
        i += 1;
    }
    if (i >= s.len) {
        return null;
    }
    const start = i;
    while (i < s.len and s[i] != '/') {
        i += 1;
    }
    return .{ .text = s[start..i], .next = i };
}

/// 单条模式是否匹配整条路径 (锚定); `path` 或 `pattern` 为空时为假
pub fn matches(path: []const u8, pattern: []const u8) bool {
    if (path.len == 0 or pattern.len == 0) {
        return false;
    }
    return matchSegs(path, 0, pattern, 0);
}

/// 任一条模式匹配整条路径即为真
pub fn matchesAny(path: []const u8, patterns: []const []const u8) bool {
    for (patterns) |pattern| {
        if (matches(path, pattern)) {
            return true;
        }
    }
    return false;
}

/// 递归比对路径段与模式段; `**` 枚举零到多段
fn matchSegs(path: []const u8, ppos: usize, pattern: []const u8, gpos: usize) bool {
    const pseg = nextSeg(pattern, gpos) orelse {
        return nextSeg(path, ppos) == null;
    };
    if (std.mem.eql(u8, pseg.text, "**")) {
        if (nextSeg(pattern, pseg.next) == null) {
            // 尾段 `/**` 表示「目录下至少一个条目」
            return nextSeg(path, ppos) != null;
        }
        var pos = ppos;
        while (true) {
            if (matchSegs(path, pos, pattern, pseg.next)) {
                return true;
            }
            const rest = nextSeg(path, pos) orelse return false;
            pos = rest.next;
        }
    }
    const sseg = nextSeg(path, ppos) orelse return false;
    if (!segmentMatches(pseg.text, sseg.text)) {
        return false;
    }
    return matchSegs(path, sseg.next, pattern, pseg.next);
}

/// 单段通配: `*` 任意序列, `?` 单个字符, `[...]` 单字符集合
fn segmentMatches(pattern: []const u8, text: []const u8) bool {
    return segMatch(pattern, 0, text, 0);
}

fn segMatch(p: []const u8, pi: usize, s: []const u8, si: usize) bool {
    var i = pi;
    var j = si;
    while (i < p.len) {
        switch (p[i]) {
            '*' => {
                while (i < p.len and p[i] == '*') {
                    i += 1;
                }
                if (i == p.len) {
                    return true;
                }
                var k = j;
                while (true) {
                    if (segMatch(p, i, s, k)) {
                        return true;
                    }
                    if (k >= s.len) {
                        return false;
                    }
                    k = nextCp(s, k).next;
                }
            },
            '?' => {
                if (j >= s.len) {
                    return false;
                }
                j = nextCp(s, j).next;
                i += 1;
            },
            '[' => {
                const scan = scanClass(p, i, if (j < s.len) nextCp(s, j).cp else 0);
                if (scan) |result| {
                    if (j >= s.len or !result.hit) {
                        return false;
                    }
                    i = result.next;
                    j = nextCp(s, j).next;
                } else {
                    // 未闭合的 `[` 按字面量
                    if (j >= s.len or nextCp(s, j).cp != '[') {
                        return false;
                    }
                    i += 1;
                    j = nextCp(s, j).next;
                }
            },
            else => {
                const pc = nextCp(p, i);
                if (j >= s.len) {
                    return false;
                }
                const sc = nextCp(s, j);
                if (pc.cp != sc.cp) {
                    return false;
                }
                i = pc.next;
                j = sc.next;
            },
        }
    }
    return j == s.len;
}

const ClassScan = struct {
    hit: bool,
    next: usize,
};

/// 扫描 `[...]` 判定字符是否命中; 未闭合时返回 `null` (调用方按字面量 `[` 处理)
fn scanClass(p: []const u8, start: usize, cp: u21) ?ClassScan {
    var i = start + 1;
    if (i >= p.len) {
        return null;
    }
    var negated = false;
    if (p[i] == '!') {
        negated = true;
        i += 1;
    }
    var hit = false;
    if (i < p.len and p[i] == ']') {
        // 紧邻的 `]` 作为首个字面量
        if (cp == ']') hit = true;
        i += 1;
    }
    while (i < p.len and p[i] != ']') {
        const low = nextCp(p, i);
        i = low.next;
        if (i + 1 < p.len and p[i] == '-' and p[i + 1] != ']') {
            const high = nextCp(p, i + 1);
            if (low.cp <= cp and cp <= high.cp) hit = true;
            i = high.next;
        } else if (low.cp == cp) {
            hit = true;
        }
    }
    if (i >= p.len) {
        return null;
    }
    return .{ .hit = hit != negated, .next = i + 1 };
}
