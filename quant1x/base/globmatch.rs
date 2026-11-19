// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! spec: quant1x/base/globmatch.py
//!
//! 锚定 glob 匹配 (globstar)。
//!
//! 整条路径从根起逐段比对: `**` 匹配零到多段, 其余段按 `*` / `?` / `[...]`
//! 通配 (`*` 不跨段、大小写敏感、点开头的名字照常匹配)。模式以 `/**` 结尾时
//! 追加一个隐式 `*`(要求目录下至少一个条目), 等价 `/**/*`。
//!
//! 与 Python 3.13 `PurePath.full_match()`、bash `globstar` 同义; 只做匹配,
//! 不接触文件系统。
//!
//! 路径与模式都可写成**项目根绝对形式** (前导 `/`), 与相对形式等价: 匹配一律以
//! 项目根为锚, `/third_party/a.h` 与 `third_party/a.h` 是同一个位置, 两侧可混写。

/// 单条模式是否匹配整条路径 (锚定)
///
/// # Arguments
///
/// * `path` - `/` 分隔的路径
/// * `pattern` - `/` 分隔的模式
///
/// # Returns
///
/// 整条匹配为真; `path` 或 `pattern` 为空时为假。
pub fn matches(path: &str, pattern: &str) -> bool {
    if path.is_empty() || pattern.is_empty() {
        return false;
    }
    let parts: Vec<&str> = path.split('/').filter(|s| !s.is_empty()).collect();
    let mut pats: Vec<&str> = pattern.split('/').filter(|s| !s.is_empty()).collect();
    if pats.is_empty() {
        return false;
    }
    if pats.last() == Some(&"**") {
        // 尾段 `/**` 表示「目录下至少一个条目」, 等价 `/**/*`
        pats.push("*");
    }
    let (n, m) = (parts.len(), pats.len());
    // dp[i][j] = parts[i..] 与 pats[j..] 是否匹配
    let mut dp = vec![vec![false; m + 1]; n + 1];
    dp[n][m] = true;
    for j in (0..m).rev() {
        if pats[j] == "**" {
            dp[n][j] = dp[n][j + 1];
        }
    }
    for i in (0..n).rev() {
        for j in (0..m).rev() {
            dp[i][j] = if pats[j] == "**" {
                dp[i][j + 1] || dp[i + 1][j]
            } else {
                dp[i + 1][j + 1] && segment_matches(pats[j], parts[i])
            };
        }
    }
    dp[0][0]
}

/// 任一条模式匹配整条路径即为真
///
/// # Arguments
///
/// * `path` - `/` 分隔的路径
/// * `patterns` - 模式序列
///
/// # Returns
///
/// 命中任一模式为真。
pub fn matches_any<S: AsRef<str>>(path: &str, patterns: &[S]) -> bool {
    patterns
        .iter()
        .any(|pattern| matches(path, pattern.as_ref()))
}

/// 单段通配: `*` 任意序列, `?` 单个字符, `[...]` 单字符集合
fn segment_matches(pattern: &str, text: &str) -> bool {
    let p: Vec<char> = pattern.chars().collect();
    let s: Vec<char> = text.chars().collect();
    match_at(&p, 0, &s, 0)
}

/// 递归匹配 `p[i..]` 与 `s[j..]`; `*` 以回溯枚举剩余位置
fn match_at(p: &[char], mut i: usize, s: &[char], mut j: usize) -> bool {
    while i < p.len() {
        match p[i] {
            '*' => {
                while i < p.len() && p[i] == '*' {
                    i += 1;
                }
                if i == p.len() {
                    return true;
                }
                let mut k = j;
                while k <= s.len() {
                    if match_at(p, i, s, k) {
                        return true;
                    }
                    k += 1;
                }
                return false;
            }
            '?' => {
                if j >= s.len() {
                    return false;
                }
                i += 1;
                j += 1;
            }
            '[' => match parse_class(p, i) {
                Some((class, next)) => {
                    if j >= s.len() || !class.matches(s[j]) {
                        return false;
                    }
                    i = next;
                    j += 1;
                }
                None => {
                    if j >= s.len() || s[j] != '[' {
                        return false;
                    }
                    i += 1;
                    j += 1;
                }
            },
            c => {
                if j >= s.len() || s[j] != c {
                    return false;
                }
                i += 1;
                j += 1;
            }
        }
    }
    j == s.len()
}

/// 一个 `[...]` 集合: 取反标志与闭区间表
struct CharClass {
    negated: bool,
    ranges: Vec<(char, char)>,
}

impl CharClass {
    /// 单个字符是否命中集合
    fn matches(&self, c: char) -> bool {
        let hit = self.ranges.iter().any(|&(lo, hi)| lo <= c && c <= hi);
        hit != self.negated
    }
}

/// 解析 `[...]` 集合; 未闭合时为 `None` (调用方按字面量 `[` 处理)
fn parse_class(p: &[char], start: usize) -> Option<(CharClass, usize)> {
    let mut i = start + 1;
    if i >= p.len() {
        return None;
    }
    let mut class = CharClass {
        negated: false,
        ranges: Vec::new(),
    };
    if p[i] == '!' {
        class.negated = true;
        i += 1;
    }
    if i < p.len() && p[i] == ']' {
        // 紧邻的 `]` 作为首个字面量
        class.ranges.push((']', ']'));
        i += 1;
    }
    while i < p.len() && p[i] != ']' {
        let low = p[i];
        if i + 2 < p.len() && p[i + 1] == '-' && p[i + 2] != ']' {
            class.ranges.push((low, p[i + 2]));
            i += 3;
        } else {
            class.ranges.push((low, low));
            i += 1;
        }
    }
    if i >= p.len() {
        return None;
    }
    Some((class, i + 1))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn globstar_zero_or_more_segments() {
        assert!(matches("stb.h", "**/*.h"));
        assert!(matches("a/b/c.h", "**/*.h"));
        assert!(matches("a/c.h", "a/**/c.h"));
        assert!(matches("a/x/y/c.h", "a/**/c.h"));
        assert!(matches("third_party/stb.h", "third_party/**"));
        assert!(matches("third_party/a/b.h", "third_party/**"));
    }

    #[test]
    fn anchored() {
        assert!(matches("libs/stb.h", "libs/*.h"));
        assert!(!matches("libs/sub/stb.h", "libs/*.h"));
        assert!(!matches("x/libs/stb.h", "libs/*.h"));
        assert!(!matches("x/third_party/a.h", "third_party/**"));
        assert!(!matches("third_partyish/x.h", "third_party/**"));
        assert!(!matches("b/a/c.h", "a/**/c.h"));
    }

    #[test]
    fn single_segment_wildcards() {
        assert!(matches("a.h", "*.h"));
        assert!(!matches("a.c", "*.h"));
        assert!(matches("a1.h", "a?.h"));
        assert!(matches("a1.h", "a[0-9].h"));
        assert!(!matches("a.h", "a[0-9].h"));
        assert!(!matches("a5.h", "a[!0-9].h"));
        assert!(matches("ax.h", "a[!0-9].h"));
        assert!(matches("a-.h", "a[a-].h"));
        assert!(!matches("az.h", "a[a-].h"));
        assert!(matches("a[b.h", "a[b.h"));
    }

    #[test]
    fn root_absolute_form() {
        assert!(matches("/third_party/a.h", "third_party/**"));
        assert!(matches("third_party/a.h", "/third_party/**"));
        assert!(matches("/third_party/a.h", "/third_party/**"));
        assert!(!matches("/x/third_party/a.h", "/third_party/**"));
        assert!(matches("/a/b.h", "/**/*.h"));
    }

    #[test]
    fn dotfile_and_case() {
        assert!(matches(".hidden.h", "*.h"));
        assert!(!matches("A.H", "*.h"));
    }

    #[test]
    fn trailing_globstar_needs_one_segment() {
        assert!(!matches("a", "a/**"));
        assert!(matches("a/b", "a/**"));
        assert!(matches("a/b/c", "a/**"));
        assert!(matches("a", "**"));
    }

    #[test]
    fn empty_inputs() {
        assert!(!matches("", "**"));
        assert!(!matches("a.h", ""));
        assert!(!matches_any("a.h", &[] as &[&str]));
    }

    #[test]
    fn matches_any_case() {
        assert!(matches_any("libs/a.h", &["third_party/**", "libs/*.h"]));
        assert!(!matches_any("libs/a.h", &["third_party/**", "**.c"]));
    }
}
