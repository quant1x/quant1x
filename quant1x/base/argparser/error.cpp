// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "error.hpp"

namespace argparser {

    namespace {

        /// 把 UTF-8 文本拆成码点; 非法字节按单字节处理。
        std::vector<char32_t> codepoints(std::string_view text) {
            std::vector<char32_t> result;
            result.reserve(text.size());
            std::size_t index = 0;
            while (index < text.size()) {
                const auto  lead  = static_cast<unsigned char>(text[index]);
                std::size_t width = 1;
                char32_t    value = lead;
                if ((lead & 0xE0) == 0xC0) {
                    width = 2;
                    value = lead & 0x1F;
                } else if ((lead & 0xF0) == 0xE0) {
                    width = 3;
                    value = lead & 0x0F;
                } else if ((lead & 0xF8) == 0xF0) {
                    width = 4;
                    value = lead & 0x07;
                }
                if (index + width > text.size()) {
                    width = 1;
                    value = lead;
                } else {
                    for (std::size_t offset = 1; offset < width; ++offset) {
                        value = (value << 6) | (static_cast<unsigned char>(text[index + offset]) & 0x3F);
                    }
                }
                result.push_back(value);
                index += width;
            }
            return result;
        }

    }  // namespace

    std::string_view error_kind_name(ErrorKind kind) {
        switch (kind) {
            case ErrorKind::UNKNOWN_ARGUMENT:
                return "unknown_argument";
            case ErrorKind::AMBIGUOUS_ARGUMENT:
                return "ambiguous_argument";
            case ErrorKind::MISSING_VALUE:
                return "missing_value";
            case ErrorKind::INVALID_VALUE:
                return "invalid_value";
            case ErrorKind::MISSING_REQUIRED:
                return "missing_required";
            case ErrorKind::CONFLICT:
                return "conflict";
            case ErrorKind::OTHER:
                break;
        }
        return "other";
    }

    std::string UsageError::render() const {
        std::string head = "error: " + message_;
        if (!reason_.empty()) {
            head += ": " + reason_;
        }
        std::string body = head;
        for (const std::string &line : lines_) {
            body += "\n  " + line;
        }
        if (!tips_.empty()) {
            body += '\n';
            for (const std::string &tip : tips_) {
                body += "\n  tip: " + tip;
            }
        }
        if (!usage_.empty()) {
            body += "\n\nUsage: " + usage_;
        }
        if (!hint_.empty()) {
            body += "\n\n" + hint_;
        }
        return body;
    }

    int exit_code(const ParseError &error) {
        if (const auto *usage = std::get_if<UsageError>(&error)) {
            return argparser::UsageError::exit_code();
        }
        if (std::holds_alternative<HelpRequested>(error) || std::holds_alternative<VersionRequested>(error)) {
            return 0;
        }
        return 1;
    }

    std::string render(const ParseError &error) {
        if (const auto *usage = std::get_if<UsageError>(&error)) {
            return usage->render();
        }
        if (const auto *help = std::get_if<HelpRequested>(&error)) {
            return help->long_form ? "--help" : "-h";
        }
        if (const auto *version = std::get_if<VersionRequested>(&error)) {
            return version->text;
        }
        return std::get<DefinitionError>(error).message;
    }

    double jaro(std::string_view left, std::string_view right) {
        if (left == right) {
            return 1.0;
        }
        const std::vector<char32_t> left_points  = codepoints(left);
        const std::vector<char32_t> right_points = codepoints(right);
        const auto                  left_len     = left_points.size();
        const auto                  right_len    = right_points.size();
        if (left_len == 0 || right_len == 0) {
            return 0.0;
        }
        const std::size_t longest = std::max(left_len, right_len);
        const std::size_t window  = longest / 2 > 0 ? longest / 2 - 1 : 0;
        std::vector<bool> left_hit(left_len, false);
        std::vector<bool> right_hit(right_len, false);
        std::size_t       matches = 0;
        for (std::size_t index = 0; index < left_len; ++index) {
            const std::size_t start = index > window ? index - window : 0;
            const std::size_t stop  = std::min(index + window + 1, right_len);
            for (std::size_t position = start; position < stop; ++position) {
                if (!right_hit[position] && right_points[position] == left_points[index]) {
                    left_hit[index]     = true;
                    right_hit[position] = true;
                    ++matches;
                    break;
                }
            }
        }
        if (matches == 0) {
            return 0.0;
        }
        std::size_t transpositions = 0;
        std::size_t cursor         = 0;
        for (std::size_t index = 0; index < left_len; ++index) {
            if (!left_hit[index]) {
                continue;
            }
            while (!right_hit[cursor]) {
                ++cursor;
            }
            if (left_points[index] != right_points[cursor]) {
                ++transpositions;
            }
            ++cursor;
        }
        transpositions /= 2;
        const auto left_size  = static_cast<double>(left_len);
        const auto right_size = static_cast<double>(right_len);
        const auto match_size = static_cast<double>(matches);
        const auto transposed = static_cast<double>(matches - transpositions);
        return (match_size / left_size + match_size / right_size + transposed / match_size) / 3.0;
    }

    double jaro_winkler(std::string_view left, std::string_view right) {
        const double                score        = jaro(left, right);
        const std::vector<char32_t> left_points  = codepoints(left);
        const std::vector<char32_t> right_points = codepoints(right);
        const std::size_t           shared       = std::min(left_points.size(), right_points.size());
        std::size_t                 prefix       = 0;
        while (prefix < shared && prefix < 4 && left_points[prefix] == right_points[prefix]) {
            ++prefix;
        }
        return score + static_cast<double>(prefix) * 0.1 * (1.0 - score);
    }

    std::optional<std::string> best_match(std::string_view word, const std::vector<std::string> &candidates) {
        return best_match_at(word, candidates, MATCH_THRESHOLD);
    }

    std::optional<std::string>
    best_match_at(std::string_view word, const std::vector<std::string> &candidates, double threshold) {
        std::optional<std::string> best;
        double                     best_score = threshold;
        for (const std::string &candidate : candidates) {
            if (candidate.empty() || candidate == word) {
                continue;
            }
            const double score = jaro_winkler(word, candidate);
            if (score > best_score) {
                best       = candidate;
                best_score = score;
            }
        }
        return best;
    }

}  // namespace argparser
