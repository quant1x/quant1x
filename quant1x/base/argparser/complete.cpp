// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>
#include <vector>

#include "complete.hpp"
#include "parser.hpp"
#include "spec.hpp"

namespace argparser {
    namespace {

        /// 一个命令在状态机里的位置: (子命令链, 命令)。
        struct Node {
            std::vector<std::string> chain;
            const Parser            *parser = nullptr;
        };

        /// 用空格拼一段词。
        std::string join(const std::vector<std::string> &items) {
            std::string text;
            for (std::size_t index = 0; index < items.size(); ++index) {
                if (index != 0) {
                    text.push_back(' ');
                }
                text += items[index];
            }
            return text;
        }

        /// 含空白字符。
        bool has_space(const std::string &text) {
            return std::any_of(
                text.begin(), text.end(), [](char ch) { return std::isspace(static_cast<unsigned char>(ch)) != 0; });
        }

        /// 按前序收集 (子命令链, 命令); 链为空的是顶层命令。
        void walk(const Parser &parser, std::vector<std::string> &chain, std::vector<Node> &nodes) {
            nodes.push_back(Node{chain, &parser});
            const SubParsers *subs = parser.subparsers();
            if (subs == nullptr) {
                return;
            }
            const std::vector<Command> commands = subs->commands();
            for (const Command &command : commands) {
                chain.push_back(command.name);
                walk(*command.parser, chain, nodes);
                chain.pop_back();
            }
        }

        /// 祖先命令上标记为 `global` 的参数: 子命令里同样可用, 也进候选。
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

        /// 一个命令的候选词: 选项名、子命令名、位置参数的 `choices`, 按登记顺序去重。
        std::vector<std::string> words(const Parser &parser) {
            std::vector<std::string>            collected;
            const std::vector<const Argument *> own       = parser.arguments();
            const std::vector<const Argument *> inherited = global_arguments(parser);
            for (const Argument *argument : own) {
                if (!argument->hide) {
                    collected.insert(collected.end(), argument->names.begin(), argument->names.end());
                }
            }
            for (const Argument *argument : inherited) {
                if (!argument->hide) {
                    collected.insert(collected.end(), argument->names.begin(), argument->names.end());
                }
            }
            if (const SubParsers *subs = parser.subparsers()) {
                for (const Command &command : subs->commands()) {
                    collected.push_back(command.name);
                }
            }
            for (const Argument *argument : own) {
                if (!argument->is_positional()) {
                    continue;
                }
                for (const Value &value : argument->choice_values()) {
                    collected.push_back(value.text());
                }
            }
            std::vector<std::string> unique;
            for (const std::string &word : collected) {
                if (word.empty() || has_space(word) || std::find(unique.begin(), unique.end(), word) != unique.end()) {
                    continue;
                }
                unique.push_back(word);
            }
            return unique;
        }

        /// 脚本里的函数名: 非标识符字符一律换成下划线。
        std::string function(std::string_view name) {
            std::string text = "_argparser_";
            for (const char ch : name) {
                const bool identifier = ch == '_' || (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
                                        (ch >= 'A' && ch <= 'Z');
                text.push_back(identifier ? ch : '_');
            }
            return text;
        }

        /// 状态机里的每条边: (键, 命中后的命令链文本)。键形如 `|alpha`、`alpha|beta`。
        std::vector<std::pair<std::string, std::string>> transitions(const std::vector<Node> &nodes) {
            std::vector<std::pair<std::string, std::string>> edges;
            for (const Node &node : nodes) {
                if (node.chain.empty()) {
                    continue;
                }
                std::vector<std::string> head(node.chain.begin(), node.chain.end() - 1);
                edges.emplace_back(join(head) + "|" + node.chain.back(), join(node.chain));
            }
            return edges;
        }

        /// fish 的单选项拼法: `-h` → `-s h`, `--help` → `-l help`; 其余形态不补。
        std::optional<std::string> fish_flag(const std::string &flag) {
            if (flag.size() > 2 && flag.rfind("--", 0) == 0) {
                return "-l " + flag.substr(2);
            }
            if (flag.size() == 2 && flag.front() == '-') {
                return std::string("-s ") + flag.back();
            }
            return std::nullopt;
        }

        /// 把说明文字放进单引号: 反斜杠与单引号各转义一层。
        std::string quote(const std::string &text) {
            std::string out;
            for (const char ch : text) {
                if (ch == '\\' || ch == '\'') {
                    out.push_back('\\');
                }
                out.push_back(ch);
            }
            return out;
        }

        std::string bash(const std::string &name, const std::vector<Node> &nodes) {
            const std::string function_name = function(name);
            std::string       out           = function_name + "() {\n    local cur cmd word\n";
            out += "    cur=\"${COMP_WORDS[COMP_CWORD]}\"\n    cmd=\"\"\n";
            const std::vector<std::pair<std::string, std::string>> edges = transitions(nodes);
            if (!edges.empty()) {
                out += "    for word in \"${COMP_WORDS[@]:1:COMP_CWORD-1}\"; do\n";
                out += "        case \"$cmd|$word\" in\n";
                for (const auto &[key, chain] : edges) {
                    out += "            \"" + key + "\") cmd=\"" + chain + "\" ;;\n";
                }
                out += "        esac\n    done\n";
            }
            out += "    case \"$cmd\" in\n";
            for (const Node &node : nodes) {
                out += "        \"" + join(node.chain) + "\") COMPREPLY=( $(compgen -W \"" + join(words(*node.parser)) +
                       "\" -- \"$cur\") ) ;;\n";
            }
            out += "    esac\n}\n";
            out += "complete -F " + function_name + " " + name + "\n";
            return out;
        }

        std::string zsh(const std::string &name, const std::vector<Node> &nodes) {
            const std::string function_name = function(name);
            std::string       out           = "#compdef " + name + "\n\n" + function_name +
                                              "() {\n    local -a candidates\n    local cmd=\"\" word\n";
            const std::vector<std::pair<std::string, std::string>> edges = transitions(nodes);
            if (!edges.empty()) {
                out += "    for word in ${words[2,CURRENT-1]}; do\n";
                out += "        case \"$cmd|$word\" in\n";
                for (const auto &[key, chain] : edges) {
                    out += "            \"" + key + "\") cmd=\"" + chain + "\" ;;\n";
                }
                out += "        esac\n    done\n";
            }
            out += "    case \"$cmd\" in\n";
            for (const Node &node : nodes) {
                out += "        \"" + join(node.chain) + "\") candidates=(" + join(words(*node.parser)) + ") ;;\n";
            }
            out += "    esac\n    compadd -- \"${candidates[@]}\"\n}\n\n";
            out += function_name + " \"$@\"\n";
            return out;
        }

        std::string fish(const std::string &name, const std::vector<Node> &nodes) {
            std::string out;
            for (const Node &node : nodes) {
                std::string condition;
                if (!node.chain.empty()) {
                    condition = " -n '__fish_seen_subcommand_from " + node.chain.back() + "'";
                }
                for (const Argument *argument : node.parser->arguments()) {
                    if (argument->hide || argument->is_positional()) {
                        continue;
                    }
                    std::vector<std::string> spelled;
                    for (const std::string &flag : argument->names) {
                        if (std::optional<std::string> word = fish_flag(flag)) {
                            spelled.push_back(*word);
                        }
                    }
                    if (spelled.empty()) {
                        continue;
                    }
                    std::string detail;
                    if (argument->help && !argument->help->empty()) {
                        detail = " -d '" + quote(*argument->help) + "'";
                    }
                    out += "complete -c " + name + condition + " " + join(spelled) + detail + "\n";
                }
                if (const SubParsers *subs = node.parser->subparsers()) {
                    for (const Command &command : subs->commands()) {
                        std::string detail;
                        if (command.parser->about() && !command.parser->about()->empty()) {
                            detail = " -d '" + quote(*command.parser->about()) + "'";
                        }
                        out += "complete -c " + name + condition + " -f -a " + command.name + detail + "\n";
                    }
                }
                std::vector<std::string> spelled;
                for (const Argument *argument : node.parser->arguments()) {
                    if (!argument->is_positional()) {
                        continue;
                    }
                    for (const Value &value : argument->choice_values()) {
                        const std::string text = value.text();
                        if (!has_space(text)) {
                            spelled.push_back(text);
                        }
                    }
                }
                if (!spelled.empty()) {
                    out += "complete -c " + name + condition + " -f -a '" + join(spelled) + "'\n";
                }
            }
            return out;
        }

    }  // namespace

    std::string complete(const Parser &parser, std::string_view shell, std::optional<std::string> prog) {
        if (std::find(SHELLS.begin(), SHELLS.end(), shell) == SHELLS.end()) {
            throw std::invalid_argument("不支持的 shell: " + std::string(shell));
        }
        const std::string        name = prog ? *prog : parser.prog();
        std::vector<Node>        nodes;
        std::vector<std::string> chain;
        walk(parser, chain, nodes);
        if (shell == "bash") {
            return bash(name, nodes);
        }
        if (shell == "zsh") {
            return zsh(name, nodes);
        }
        return fish(name, nodes);
    }

}  // namespace argparser
