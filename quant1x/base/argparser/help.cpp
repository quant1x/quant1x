// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "error.hpp"
#include "help.hpp"
#include "parser.hpp"
#include "spec.hpp"

namespace argparser {
    namespace {

        constexpr std::string_view RESET        = "\x1b[0m";
        constexpr std::string_view HEADING_CODE = "\x1b[1m\x1b[4m";
        constexpr std::string_view BOLD_CODE    = "\x1b[1m";
        constexpr std::string_view LITERAL_CODE = "\x1b[32m";
        constexpr std::string_view ERROR_CODE   = "\x1b[31m";
        constexpr std::string_view WARNING_CODE = "\x1b[33m";

        /// 表格行: 左列 (选项 / 命令 / 位置参数) 与右列说明。
        using Row = std::pair<std::string, std::string>;

        /// 可见宽度: 码点个数, 与 Python 的 `len()` 同口径。
        std::size_t width_of(std::string_view text) {
            std::size_t total = 0;
            for (unsigned char byte : text) {
                if ((byte & 0xC0) != 0x80) {
                    ++total;
                }
            }
            return total;
        }

        /// 折行只认 ASCII 空白。
        bool is_space(char byte) {
            return byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n' || byte == '\v' || byte == '\f';
        }

        std::string spaces(std::size_t count) {
            std::string blanks;
            blanks.assign(count, ' ');
            return blanks;
        }

        /// 单段文字按宽度折行; `width` 是**含缩进在内**的整行宽度。
        std::vector<std::string> wrap_one(std::string_view   paragraph,
                                          std::size_t        width,
                                          const std::string &initial_indent,
                                          const std::string &subsequent_indent) {
            const std::size_t        limit = width < 20 ? 20 : width;
            std::vector<std::string> lines;
            std::string              line   = initial_indent;
            bool                     filled = false;
            std::size_t              index  = 0;
            while (index < paragraph.size()) {
                while (index < paragraph.size() && is_space(paragraph[index])) {
                    ++index;
                }
                const std::size_t start = index;
                while (index < paragraph.size() && !is_space(paragraph[index])) {
                    ++index;
                }
                if (start == index) {
                    break;
                }
                const std::string_view word = paragraph.substr(start, index - start);
                if (filled && width_of(line) + 1 + width_of(word) > limit) {
                    lines.push_back(line);
                    line   = subsequent_indent;
                    filled = false;
                }
                if (filled) {
                    line.push_back(' ');
                }
                line.append(word);
                filled = true;
            }
            if (filled) {
                lines.push_back(line);
            } else {
                lines.emplace_back();
            }
            return lines;
        }

        /// 按段落换行; 段间保留一个空行位。
        std::vector<std::string> wrap_paragraphs(std::string_view   text,
                                                 std::size_t        width,
                                                 const std::string &indent,
                                                 const std::string *first_indent = nullptr) {
            std::vector<std::string> lines;
            std::size_t              start = 0;
            std::size_t              index = 0;
            bool                     first = true;
            while (true) {
                if (index == text.size() || text[index] == '\n') {
                    const std::string head    = first && first_indent != nullptr ? *first_indent : indent;
                    const auto        wrapped = wrap_one(text.substr(start, index - start), width, head, indent);
                    lines.insert(lines.end(), wrapped.begin(), wrapped.end());
                    first = false;
                    if (index == text.size()) {
                        break;
                    }
                    start = index + 1;
                }
                ++index;
            }
            return lines;
        }

        /// 一行说明文字: 帮助正文 + 默认值 / 环境变量 / 可选值。
        std::string argument_text(const Argument &argument, bool long_help) {
            std::string        text;
            const std::string *preferred = long_help && argument.long_help.has_value() ? &argument.long_help.value()
                                                                                       : nullptr;
            if (preferred != nullptr && !preferred->empty()) {
                text = *preferred;
            } else if (argument.help.has_value()) {
                text = argument.help.value();
            }
            std::vector<std::string> extras;
            const bool shows_default = argument.action == Action::STORE || argument.action == Action::APPEND ||
                                       argument.action == Action::EXTEND;
            if (shows_default && !argument.hide_default) {
                const Value *value = argument.default_value.value();
                if (value != nullptr && !value->is_null()) {
                    extras.push_back("[default: " + value->text() + "]");
                }
            }
            if (argument.env.has_value()) {
                extras.push_back("[env: " + argument.env.value() + "]");
            }
            const std::vector<Value> choices = argument.choice_values();
            if (!choices.empty()) {
                extras.push_back("[possible values: " + format_items(choices) + "]");
            }
            if (!extras.empty()) {
                text += ' ';
                for (std::size_t index = 0; index < extras.size(); ++index) {
                    if (index != 0) {
                        text += ' ';
                    }
                    text += extras[index];
                }
                const auto head = text.find_first_not_of(" \t\r\n");
                const auto tail = text.find_last_not_of(" \t\r\n");
                text            = head == std::string::npos ? std::string{} : text.substr(head, tail - head + 1);
            }
            if (long_help) {
                std::string notes;
                for (const Value &value : choices) {
                    const std::string note = argument.choice_help(value);
                    if (note.empty()) {
                        continue;
                    }
                    if (!notes.empty()) {
                        notes += '\n';
                    }
                    notes += "- " + value.text() + ": " + note;
                }
                if (!notes.empty()) {
                    text = text.empty() ? notes : text + "\n" + notes;
                }
            }
            return text;
        }

        /// 祖先命令上标记为 `global` 的参数: 子命令里同样可用, 也出现在子命令帮助里。
        std::vector<const Argument *> global_arguments(const Parser &parser) {
            std::vector<const Argument *> collected;
            for (const Parser *parent = parser.parent(); parent != nullptr; parent = parent->parent()) {
                for (const Argument *argument : parent->arguments()) {
                    if (argument->global) {
                        collected.push_back(argument);
                    }
                }
            }
            std::reverse(collected.begin(), collected.end());
            return collected;
        }

        /// 本命令的选项 + 祖先命令下沉的 `global` 参数; 帮助 / 版本排最后 (与 clap 一致)。
        std::vector<const Argument *> visible_options(const Parser &parser) {
            std::vector<const Argument *> options;
            for (const Argument *argument : parser.arguments()) {
                if (!argument->is_positional() && !argument->hide) {
                    options.push_back(argument);
                }
            }
            for (const Argument *argument : global_arguments(parser)) {
                if (!argument->hide) {
                    options.push_back(argument);
                }
            }
            std::vector<const Argument *> ordered;
            ordered.reserve(options.size());
            for (const Argument *argument : options) {
                if (argument->action != Action::HELP && argument->action != Action::VERSION) {
                    ordered.push_back(argument);
                }
            }
            for (const Argument *argument : options) {
                if (argument->action == Action::HELP || argument->action == Action::VERSION) {
                    ordered.push_back(argument);
                }
            }
            return ordered;
        }

        /// `标题 + 两列表格`: 左列对齐, 右列折行并与右列首字符对齐。
        std::string table(const std::string                &title,
                          const std::vector<Row>           &rows,
                          std::size_t                       width,
                          const Style                      &style,
                          const std::optional<std::string> &description = std::nullopt) {
            if (rows.empty()) {
                return {};
            }
            std::size_t widest = 0;
            for (const Row &row : rows) {
                widest = std::max(widest, width_of(row.first));
            }
            const std::size_t        left_width = std::min(widest, std::max<std::size_t>(16, width / 3));
            const std::size_t        gutter     = 2;
            const std::size_t        base       = 2 + left_width + gutter;
            std::vector<std::string> lines{title};
            if (description.has_value() && !description->empty()) {
                const std::string indent  = "  ";
                const auto        wrapped = wrap_paragraphs(description.value(), width, indent, &indent);
                lines.insert(lines.end(), wrapped.begin(), wrapped.end());
                lines.emplace_back();
            }
            for (const Row &row : rows) {
                const std::string head = style.literal("  " + row.first);
                if (row.second.empty()) {
                    lines.push_back(head);
                    continue;
                }
                if (width_of(row.first) > left_width) {
                    lines.push_back(head);
                    const auto wrapped = wrap_paragraphs(row.second, width, spaces(4));
                    lines.insert(lines.end(), wrapped.begin(), wrapped.end());
                    continue;
                }
                const std::string indent = spaces(base);
                const auto        chunks = wrap_paragraphs(row.second, width, indent, &indent);
                const std::string pad    = spaces(base - 2 - width_of(row.first));
                const auto        start  = chunks.front().find_first_not_of(' ');
                lines.push_back(head + pad +
                                (start == std::string::npos ? std::string{} : chunks.front().substr(start)));
                lines.insert(lines.end(), chunks.begin() + 1, chunks.end());
            }
            std::string text;
            for (std::size_t index = 0; index < lines.size(); ++index) {
                if (index != 0) {
                    text += '\n';
                }
                text += lines[index];
            }
            return text;
        }

        /// 把折行结果拼回一段文本, 行间用一个换行。
        std::string join_lines(const std::vector<std::string> &lines) {
            std::string text;
            for (std::size_t index = 0; index < lines.size(); ++index) {
                if (index != 0) {
                    text += '\n';
                }
                text += lines[index];
            }
            return text;
        }

        /// 子命令章节的行: 名字 + 展示别名; 自动生成的 `help` 排在最后。
        std::vector<Row> command_rows(const Parser &parser) {
            std::vector<Row>  rows;
            const SubParsers *subparsers = parser.subparsers();
            if (subparsers == nullptr) {
                return rows;
            }
            for (const Command &command : subparsers->commands()) {
                std::string head = command.name;
                if (!command.aliases.empty()) {
                    head += ", " + format_names(command.aliases);
                }
                const std::optional<std::string> &about = command.parser != nullptr ? command.parser->about()
                                                                                    : std::nullopt;
                rows.emplace_back(head, about.value_or(""));
            }
            if (!subparsers->has_command("help")) {
                rows.emplace_back("help", "Print this message or the help of the given subcommand(s)");
            }
            return rows;
        }

    }  // namespace

    std::string Style::wrap(std::string_view code, std::string_view text) const {
        if (!enabled_ || text.empty()) {
            return std::string(text);
        }
        std::string body(code);
        body.append(text);
        body.append(RESET);
        return body;
    }

    std::string Style::heading(std::string_view text) const {
        return wrap(HEADING_CODE, text);
    }

    std::string Style::bold(std::string_view text) const {
        return wrap(BOLD_CODE, text);
    }

    std::string Style::literal(std::string_view text) const {
        return wrap(LITERAL_CODE, text);
    }

    std::string Style::error(std::string_view text) const {
        return wrap(ERROR_CODE, text);
    }

    std::string Style::warning(std::string_view text) const {
        return wrap(WARNING_CODE, text);
    }

    std::string render_usage(const Parser &parser) {
        const Style style(parser.color_enabled());
        return style.heading("Usage:") + " " + parser.usage_spec();
    }

    std::string render_help(const Parser &parser, bool long_help) {
        const Style                       style(parser.color_enabled());
        const std::size_t                 width = parser.width();
        std::vector<std::string>          blocks;
        const std::optional<std::string> &about  = long_help ? parser.long_about() : parser.about();
        const std::optional<std::string> &chosen = about.has_value() && !about->empty() ? about : parser.about();
        if (chosen.has_value() && !chosen->empty()) {
            blocks.push_back(join_lines(wrap_paragraphs(chosen.value(), width, "")));
        }
        blocks.push_back(render_usage(parser));
        if (const SubParsers *subparsers = parser.subparsers(); subparsers != nullptr) {
            const std::string                title   = style.heading(subparsers->title() + ":");
            const std::optional<std::string> help    = subparsers->help().empty()
                                                           ? std::nullopt
                                                           : std::optional<std::string>(subparsers->help());
            std::string                      section = table(title, command_rows(parser), width, style, help);
            if (!section.empty()) {
                blocks.push_back(std::move(section));
            }
        }
        std::vector<Row> positionals;
        for (const Argument *argument : parser.arguments()) {
            if (!argument->is_positional() || argument->hide) {
                continue;
            }
            positionals.emplace_back(argument->help_names(), argument_text(*argument, long_help));
        }
        if (!positionals.empty()) {
            blocks.push_back(table(style.heading("Arguments:"), positionals, width, style));
        }
        const std::vector<const Argument *> options = visible_options(parser);
        std::vector<Row>                    default_options;
        for (const Argument *argument : options) {
            if (argument->group.has_value()) {
                bool registered = false;
                for (const GroupInfo &group : parser.groups()) {
                    if (group.title == argument->group.value()) {
                        registered = true;
                        break;
                    }
                }
                if (registered) {
                    continue;
                }
            }
            default_options.emplace_back(argument->help_names(), argument_text(*argument, long_help));
        }
        if (!default_options.empty()) {
            blocks.push_back(table(style.heading("Options:"), default_options, width, style));
        }
        for (const GroupInfo &group : parser.groups()) {
            std::vector<Row> grouped;
            for (const Argument *argument : options) {
                if (!argument->group.has_value() || argument->group.value() != group.title) {
                    continue;
                }
                grouped.emplace_back(argument->help_names(), argument_text(*argument, long_help));
            }
            if (grouped.empty()) {
                continue;
            }
            blocks.push_back(table(style.heading(group.title + ":"), grouped, width, style, group.description));
        }
        if (parser.epilog().has_value() && !parser.epilog()->empty()) {
            blocks.push_back(join_lines(wrap_paragraphs(parser.epilog().value(), width, "")));
        }
        std::string text;
        for (std::size_t index = 0; index < blocks.size(); ++index) {
            if (index != 0) {
                text += "\n\n";
            }
            text += blocks[index];
        }
        return text;
    }

    std::string render_error(const UsageError &error, bool color) {
        if (!color) {
            return error.render();
        }
        const Style style(true);
        std::string body = style.error("error:") + " " + error.message();
        if (!error.reason().empty()) {
            body += ": " + error.reason();
        }
        for (const std::string &line : error.lines()) {
            body += "\n  " + line;
        }
        if (!error.tips().empty()) {
            body += '\n';
            for (const std::string &tip : error.tips()) {
                body += "\n  " + style.warning("tip:") + " " + tip;
            }
        }
        if (!error.usage().empty()) {
            body += "\n\n" + style.heading("Usage:") + " " + error.usage();
        }
        if (!error.hint().empty()) {
            body += "\n\n" + error.hint();
        }
        return body;
    }

}  // namespace argparser
