# -*- coding: utf-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: AGPL-3.0-only

"""pre-commit 校验：拦截混进本次提交内容的 BOM。

仓库一律 UTF-8 无 BOM —— 规定见 AGENTS.md 的「文件编码与行尾约定」。这东西肉眼
不可见，Markdown 预览里往往也不显示，靠事后自查容易漏，故在提交这道口子上堵住。

**为什么只查新增行，不扫整个文件**：CHANGELOG.md 现存 17 处行首 U+FEFF，是历史
事实，已明确不予追溯修正。若钩子扫全文件，那么每改动一次该文件都会被拦下 ——
规矩一旦扰民，最后的结果只会是被 `--no-verify` 绕过去。故这里只检查**本次提交
新引入的行**：既往的不良留在原地不动，新的一个也不许进来。

**为什么不查 CRLF**：行尾已由 `.gitattributes` 的 `text eol=lf` 兜住 —— 实测带
CRLF 的文件 add 进暂存区时 git 就归一成了 LF，并打印 `CRLF will be replaced by
LF`。钩子重复那一层只是徒增啰嗦，故不管。

**如何取新增行**：`git diff --cached -U0 -- <文件>`，取以 `+` 开头且非 `+++`
文件头的那些行。附带的好处是：二进制文件 git 只输出 `Binary files ... differ`
而不产生 `+` 行，故无需另行判断是否二进制。

**例外**：`.bat` / `.cmd` 整体豁免 —— 它们是 Windows 批处理，形态自成一格。

用法：
    python text_check.py
返回 0 通过；非 0 拒绝（信息输出到 stderr）。
"""
import subprocess
import sys

BOM = b"\xef\xbb\xbf"

# Windows 批处理整体豁免, 见 AGENTS.md 的文件编码与行尾约定
EXEMPT_SUFFIXES = (".bat", ".cmd")


def _git(args, binary=False):
    """执行 git 子命令。失败即抛, 让钩子明确报错而不是静默放过。"""
    proc = subprocess.run(
        ["git"] + args, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    if proc.returncode != 0:
        raise RuntimeError(
            "git %s 失败: %s" % (" ".join(args), proc.stderr.decode("utf-8", "replace"))
        )
    return proc.stdout if binary else proc.stdout.decode("utf-8", "replace")


def staged_files():
    """本次待提交(新增/复制/修改)的文件列表; -z 分隔, 文件名带空格也拆不开。"""
    out = _git(["diff", "--cached", "--name-only", "-z", "--diff-filter=ACM"])
    return [f for f in out.split("\0") if f]


def added_lines(path):
    """本次提交在该文件里新增的行(字节), 去掉 diff 的 `+` 号与 `+++` 文件头。"""
    raw = _git(["diff", "--cached", "-U0", "--", path], binary=True)
    return [
        line[1:]
        for line in raw.split(b"\n")
        if line.startswith(b"+") and not line.startswith(b"+++")
    ]


def scan(path):
    """本次新增行里的 BOM 数。"""
    return sum(line.count(BOM) for line in added_lines(path))


def main():
    offenders = []
    for path in staged_files():
        if path.lower().endswith(EXEMPT_SUFFIXES):
            continue
        bom = scan(path)
        if bom:
            offenders.append((path, bom))

    if not offenders:
        return 0

    sys.stderr.write("pre-commit 校验未通过：下列文件**本次新增的行**混进了 BOM\n\n")
    for path, bom in offenders:
        sys.stderr.write("  %s  BOM=%d\n" % (path, bom))
    sys.stderr.write(
        "\nU+FEFF 多半是某次编辑把自带 BOM 的一段文本拼接进来的 —— 它肉眼看不见,\n"
        "所见只是 `- ` 与正文之间多了个位置。修法见 AGENTS.md 的「文件编码与行尾约定」。\n"
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
