# -*- coding: utf-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: AGPL-3.0-only

"""commit-msg 校验：提交信息编码完整性检查（拒绝乱码/损坏文本）。

Windows 下中文提交信息若经 GBK 命令行/控制台传递，会被错误转码并作为
“合法 UTF-8 的乱码文本”写入提交对象（历史上有 daa14be / 8e49a85 两例）。
本脚本只做两类判断，全部基于编码事实，不维护任何“乱码特征字”黑名单：

一、硬性检查（编码损坏的直接证据）
   - 必须整体是合法 UTF-8；
   - 不含 NUL 字节、替换符 U+FFFD、私有区字符（PUA）。
     这些都是正常输入不可能出现、只会由编码损坏产生的字符。

二、round-trip 反推（乱码检测主逻辑）
   乱码的产生链是确定的：
       原始 UTF-8 字节流 → 被按编码 E 误读（得到“乱码字符”）
       → 再以 UTF-8 存储（得到我们看到的文本）
   因此把待检文本按 E 重新编码、再按 UTF-8 解码，若还原出的文本
   “与原文不同且以汉字为主”，即可确认它是 E 误读的产物：
       text.encode(E).decode('utf-8') → 原始中文
   CANDIDATES 列出常见误读链（GB18030/CP1252/Latin-1/Shift_JIS/
   EUC-KR/KOI8-R 等）。将来出现“UTF-8 被当作编码 X 读取”的新形态，
   只需在 CANDIDATES 增补一行，无需维护字符表。

   误报分析（为何“还原出汉字”是强信号）：
   - 正常中文文本按 GBK/GB18030 整体编码后，字节流几乎必然无法构成
     合法 UTF-8（单字双字节落点不在 UTF-8 合法区间内，序列连续性亦
     不成立），解码必然失败；即便以 errors='replace' 宽容扫描，GBK
     字节偶合出的 UTF-8 片段也只会落在 Latin 区（2 字节 UTF-8 编码
     范围 U+0080..U+07FF 不含汉字），汉字必须由 3 字节 UTF-8 编码，
     这种连续偶合几乎不可能成串出现。因此“还原文本含 >= 2 个汉字”
     基本只可能来自真正的乱码。

   字节丢失处理：误读链若带丢字节（历史两例即如此），还原会以
   errors='replace' 进行，丢失处产生 U+FFFD。此时用 “FFFD 数 <= 还原
   汉字数” 约束：真乱码还原后汉字是主体、FFFD 只是零星丢失位；
   正常文本偶合还原出的汉字极少而 FFFD 占绝对多数，二者可明确区分。

用法：
    python commit_msg_check.py <commit-msg-file>
    python commit_msg_check.py --debug <commit-msg-file>   # 诊断模式
返回 0 通过；非 0 拒绝（信息输出到 stderr）。

--debug 模式：对每个 CANDIDATE 打印 round-trip 诊断（encode 结果、
解码模式、还原汉字数 / FFFD 数、是否命中），便于排查“新出现的乱码
形态应归入哪条误读链”。
"""
import re
import sys

# 误读链候选，按仓库环境（Windows/中文）常见度排序；每条注明典型场景，
# 日后按“实际观测到的新乱码形态”增补即可：
#   gb18030   Windows 简体中文环境把 UTF-8 字节流按 GBK/CP936 读取后再以
#             UTF-8 存储 —— 本仓库历史乱码（daa14be / 8e49a85）即此链；
#             GB18030 是 GBK 的超集，覆盖其全部单/双字节形态
#   cp1252    Windows 拉丁环境按 CP1252 读取 UTF-8（“Ã©…” 型 mojibake）
#   latin-1   逐字节无损映射的 ISO-8859-1，覆盖 cp1252 未包含的 0x80-0x9F 形态
#   shift_jis 日文环境按 Shift_JIS/CP932 读取 UTF-8
#   euc_kr    韩文环境按 EUC-KR 读取 UTF-8
#   koi8_r    俄文/西里尔环境按 KOI8-R 读取 UTF-8
CANDIDATES = [
    ("gb18030", "UTF-8 被按 GBK/GB18030 误读"),
    ("cp1252", "UTF-8 被按 CP1252 误读"),
    ("latin-1", "UTF-8 被按 Latin-1(ISO-8859-1) 误读"),
    ("shift_jis", "UTF-8 被按 Shift_JIS/CP932 误读"),
    ("euc_kr", "UTF-8 被按 EUC-KR 误读"),
    ("koi8_r", "UTF-8 被按 KOI8-R 误读"),
]

# 汉字集合（CJK 扩展 A + 统一表意 + 兼容表意）—— 还原文本“汉字量”判断用
_HAN_RE = re.compile(r"[\u3400-\u4dbf\u4e00-\u9fff\uf900-\ufaff]")


def _roundtrip(enc, text):
    """单候选 round-trip 反推。encode 失败（该编码不可能误读出此文本）返回 None。"""
    try:
        raw = text.encode(enc)
    except UnicodeEncodeError:
        return None
    try:
        recovered = raw.decode("utf-8")
        mode = "strict"
        loss = 0
    except UnicodeDecodeError:
        recovered = raw.decode("utf-8", errors="replace")
        mode = "replace"
        loss = recovered.count("\ufffd")
    return {
        "recovered": recovered,
        "mode": mode,
        "loss": loss,
        "han": len(_HAN_RE.findall(recovered)),
    }


def detect_mojibake(text):
    """命中返回 (encoding, label, recovered)，否则 None。"""
    for enc, label in CANDIDATES:
        r = _roundtrip(enc, text)
        if r is None or r["recovered"] == text:
            continue
        if r["han"] >= 2 and r["loss"] <= r["han"]:
            return enc, label, r["recovered"]
    return None


def _clean_preview(text, limit=72):
    out = []
    for ch in text:
        if ch == "\ufffd":
            continue
        o = ord(ch)
        if o < 32 and ch not in "\t\n":
            continue
        out.append(ch)
    return "".join(out)[:limit]


def _debug_report(text, valid_utf8):
    sys.stderr.write("commit-msg 校验 --debug：\n")
    if not valid_utf8:
        sys.stderr.write("  文本非合法 UTF-8，round-trip 检测跳过（硬性检查已拒绝）\n")
        return
    for enc, label in CANDIDATES:
        r = _roundtrip(enc, text)
        if r is None:
            sys.stderr.write("  %-9s %s\n" % (enc, "encode 失败——不可能为该编码误读产物"))
            continue
        same = r["recovered"] == text
        hit = (not same and r["han"] >= 2 and r["loss"] <= r["han"])
        verdict = "命中" if hit else ("还原=原文" if same else "未命中")
        sys.stderr.write(
            "  %-9s %s decode=%s han=%d loss=%d %s\n"
            % (enc, r["mode"], verdict, r["han"], r["loss"], label)
        )
        if hit:
            sys.stderr.write("      反推还原：%s\n" % _clean_preview(r["recovered"]))


def check(path, debug=False):
    with open(path, "rb") as f:
        raw = f.read()
    problems = []

    if b"\x00" in raw:
        problems.append("包含 NUL 字节")

    try:
        text = raw.decode("utf-8")
        valid_utf8 = True
    except UnicodeDecodeError as e:
        problems.append("不是合法 UTF-8：%s" % e)
        text = raw.decode("utf-8", errors="replace")
        valid_utf8 = False

    if "\ufffd" in text:
        problems.append("含替换符 U+FFFD（编码损坏标记）")

    if any("\ue000" <= ch <= "\uf8ff" for ch in text):
        problems.append("含私有区字符 U+E000..U+F8FF（编码损坏迹象）")

    if debug:
        _debug_report(text, valid_utf8)

    if valid_utf8:
        hit = detect_mojibake(text)
        if hit:
            enc, label, recovered = hit
            preview = _clean_preview(recovered)
            problems.append(
                "%s：round-trip 反推可还原为「%s%s」"
                % (label, preview, "…" if len(recovered) > 72 else "")
            )

    if problems:
        for p in problems:
            sys.stderr.write("commit-msg 校验未通过：%s\n" % p)
        sys.stderr.write(
            "提示：Windows 下提交中文信息请走 UTF-8 文件方式，例如\n"
            "  git commit -F .git/commit-msg.tmp\n"
            "不要把中文直接写在命令行 -m 参数里。\n"
        )
        return 1
    return 0


if __name__ == "__main__":
    args = sys.argv[1:]
    debug = False
    if args and args[0] == "--debug":
        debug = True
        args = args[1:]
    if len(args) < 1:
        sys.stderr.write("usage: commit_msg_check.py [--debug] <commit-msg-file>\n")
        sys.exit(2)
    sys.exit(check(args[0], debug=debug))
