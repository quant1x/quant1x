// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#include <quant1x/base/globmatch.hpp>

#include <cstddef>
#include <optional>

namespace globmatch {
    namespace {

        /// 一个 Unicode 标量及其后的字节位置
        struct Cp {
            char32_t cp;
            std::size_t next;
        };

        /// 取 `s[i..]` 首字符; 非法 UTF-8 时按单字节处理
        Cp next_cp(std::string_view s, std::size_t i) {
            if (i >= s.size()) {
                return {0, i};
            }
            const auto first = static_cast<unsigned char>(s[i]);
            std::size_t width = 1;
            char32_t cp = first;
            if (first >= 0xF0) {
                width = 4;
                cp = first & 0x07;
            } else if (first >= 0xE0) {
                width = 3;
                cp = first & 0x0F;
            } else if (first >= 0xC0) {
                width = 2;
                cp = first & 0x1F;
            }
            if (width > 1) {
                if (i + width > s.size()) {
                    return {first, i + 1};
                }
                for (std::size_t k = 1; k < width; ++k) {
                    const auto byte = static_cast<unsigned char>(s[i + k]);
                    if ((byte & 0xC0) != 0x80) {
                        return {first, i + 1};
                    }
                    cp = (cp << 6) | (byte & 0x3F);
                }
            }
            return {cp, i + width};
        }

        /// 按 `/` 取下一段 (跳过连续分隔符)
        struct Seg {
            std::string_view text;
            std::size_t next;
        };

        std::optional<Seg> next_seg(std::string_view s, std::size_t pos) {
            std::size_t i = pos;
            while (i < s.size() && s[i] == '/') {
                ++i;
            }
            if (i >= s.size()) {
                return std::nullopt;
            }
            const std::size_t start = i;
            while (i < s.size() && s[i] != '/') {
                ++i;
            }
            return Seg{s.substr(start, i - start), i};
        }

        /// `[...]` 的扫描结果; 未闭合时调用方按字面量 `[` 处理
        struct ClassScan {
            bool hit;
            std::size_t next;
        };

        std::optional<ClassScan> scan_class(std::string_view p, std::size_t start, char32_t cp) {
            std::size_t i = start + 1;
            if (i >= p.size()) {
                return std::nullopt;
            }
            bool negated = false;
            if (p[i] == '!') {
                negated = true;
                ++i;
            }
            bool hit = false;
            if (i < p.size() && p[i] == ']') {
                // 紧邻的 `]` 作为首个字面量
                if (cp == U']') {
                    hit = true;
                }
                ++i;
            }
            while (i < p.size() && p[i] != ']') {
                const Cp low = next_cp(p, i);
                i = low.next;
                if (i + 1 < p.size() && p[i] == '-' && p[i + 1] != ']') {
                    const Cp high = next_cp(p, i + 1);
                    if (low.cp <= cp && cp <= high.cp) {
                        hit = true;
                    }
                    i = high.next;
                } else if (low.cp == cp) {
                    hit = true;
                }
            }
            if (i >= p.size()) {
                return std::nullopt;
            }
            return ClassScan{hit != negated, i + 1};
        }

        bool seg_match(std::string_view p, std::size_t pi, std::string_view s, std::size_t si);

        /// 单段通配: `*` 任意序列, `?` 单个字符, `[...]` 单字符集合
        bool segment_matches(std::string_view pattern, std::string_view text) {
            return seg_match(pattern, 0, text, 0);
        }

        bool seg_match(std::string_view p, std::size_t pi, std::string_view s, std::size_t si) {
            std::size_t i = pi;
            std::size_t j = si;
            while (i < p.size()) {
                switch (p[i]) {
                    case '*': {
                        while (i < p.size() && p[i] == '*') {
                            ++i;
                        }
                        if (i == p.size()) {
                            return true;
                        }
                        std::size_t k = j;
                        while (true) {
                            if (seg_match(p, i, s, k)) {
                                return true;
                            }
                            if (k >= s.size()) {
                                return false;
                            }
                            k = next_cp(s, k).next;
                        }
                    }
                    case '?':
                        if (j >= s.size()) {
                            return false;
                        }
                        j = next_cp(s, j).next;
                        ++i;
                        break;
                    case '[': {
                        const auto scan = scan_class(p, i, j < s.size() ? next_cp(s, j).cp : 0);
                        if (scan.has_value()) {
                            if (j >= s.size() || !scan->hit) {
                                return false;
                            }
                            i = scan->next;
                            j = next_cp(s, j).next;
                        } else {
                            // 未闭合的 `[` 按字面量
                            if (j >= s.size() || next_cp(s, j).cp != U'[') {
                                return false;
                            }
                            ++i;
                            j = next_cp(s, j).next;
                        }
                        break;
                    }
                    default: {
                        const Cp pc = next_cp(p, i);
                        if (j >= s.size()) {
                            return false;
                        }
                        const Cp sc = next_cp(s, j);
                        if (pc.cp != sc.cp) {
                            return false;
                        }
                        i = pc.next;
                        j = sc.next;
                        break;
                    }
                }
            }
            return j == s.size();
        }

        /// 递归比对路径段与模式段; `**` 枚举零到多段
        bool match_segs(std::string_view path, std::size_t ppos, std::string_view pattern, std::size_t gpos) {
            const auto pseg = next_seg(pattern, gpos);
            if (!pseg.has_value()) {
                return !next_seg(path, ppos).has_value();
            }
            if (pseg->text == "**") {
                if (!next_seg(pattern, pseg->next).has_value()) {
                    // 尾段 `/**` 表示「目录下至少一个条目」
                    return next_seg(path, ppos).has_value();
                }
                std::size_t pos = ppos;
                while (true) {
                    if (match_segs(path, pos, pattern, pseg->next)) {
                        return true;
                    }
                    const auto rest = next_seg(path, pos);
                    if (!rest.has_value()) {
                        return false;
                    }
                    pos = rest->next;
                }
            }
            const auto sseg = next_seg(path, ppos);
            if (!sseg.has_value() || !segment_matches(pseg->text, sseg->text)) {
                return false;
            }
            return match_segs(path, sseg->next, pattern, pseg->next);
        }

    }// namespace

    bool matches(std::string_view path, std::string_view pattern) {
        if (path.empty() || pattern.empty()) {
            return false;
        }
        return match_segs(path, 0, pattern, 0);
    }

    bool matches_any(std::string_view path, const std::vector<std::string_view> &patterns) {
        for (const auto &pattern : patterns) {
            if (matches(path, pattern)) {
                return true;
            }
        }
        return false;
    }

}// namespace globmatch
