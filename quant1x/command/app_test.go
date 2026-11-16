// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package command

import (
	"bytes"
	"errors"
	"testing"

	"github.com/quant1x/quant1x/quant1x/base/argparser"
	"github.com/quant1x/quant1x/quant1x/data"
)

// TestRootParserGlobalAndSubcommand 全局选项写在子命令之后也能落到同一个命名空间
func TestRootParserGlobalAndSubcommand(t *testing.T) {
	parser, err := newRootParser()
	if err != nil {
		t.Fatalf("newRootParser: %v", err)
	}
	namespace, err := parser.Parse([]string{
		"update", "--debug", "--cpu", "3", "--avx2",
		"--all", "--start", "2024-06-26",
	})
	if err != nil {
		t.Fatalf("Parse: %v", err)
	}

	if got := namespaceString(namespace, subcommandDest); got != updateCommand {
		t.Errorf("子命令落值 = %q, want %q", got, updateCommand)
	}
	if !namespaceBool(namespace, flagDebug) {
		t.Error("--debug 未落成 true")
	}
	if !namespaceBool(namespace, flagAvx2) {
		t.Error("--avx2 未落成 true")
	}
	if got := namespaceInt(namespace, flagCPU); got != 3 {
		t.Errorf("--cpu = %d, want 3", got)
	}
	if !namespaceBool(namespace, flagAll) {
		t.Error("--all 未落成 true")
	}
	if got := namespaceString(namespace, flagStartDate); got != "2024-06-26" {
		t.Errorf("--start = %q, want 2024-06-26", got)
	}
}

// TestRootParserDefaults 未给出的选项取默认值
func TestRootParserDefaults(t *testing.T) {
	parser, err := newRootParser()
	if err != nil {
		t.Fatalf("newRootParser: %v", err)
	}
	namespace, err := parser.Parse([]string{"update"})
	if err != nil {
		t.Fatalf("Parse: %v", err)
	}

	if namespaceBool(namespace, flagDebug) || namespaceBool(namespace, flagAvx2) || namespaceBool(namespace, flagAll) {
		t.Error("布尔开关默认应为 false")
	}
	if got := namespaceInt(namespace, flagCPU); got != cpuNum {
		t.Errorf("--cpu 默认 = %d, want %d", got, cpuNum)
	}
	wantDate := data.LastTradingDay(data.NowTimestamp()).OnlyDate()
	if got := namespaceString(namespace, flagStartDate); got != wantDate {
		t.Errorf("--start 默认 = %q, want %q", got, wantDate)
	}
	if got := namespaceString(namespace, flagEndDate); got != wantDate {
		t.Errorf("--end 默认 = %q, want %q", got, wantDate)
	}
	if got := namespaceString(namespace, flagBaseData); got != "" {
		t.Errorf("--base 默认 = %q, want 空串", got)
	}
}

// TestRootParserHelp 帮助请求由解析器抛出, 不进入子命令分发
func TestRootParserHelp(t *testing.T) {
	parser, err := newRootParser()
	if err != nil {
		t.Fatalf("newRootParser: %v", err)
	}
	_, failure := parser.Parse([]string{"--help"})
	var requested *argparser.HelpRequested
	if !errors.As(failure, &requested) {
		t.Fatalf("--help 应返回 HelpRequested, 实际: %v", failure)
	}

	// ParseOrExit 把帮助写到 out 并退 0
	var out, errOut bytes.Buffer
	namespace, code := parser.ParseOrExit([]string{"--help"}, &out, &errOut)
	if code != 0 || namespace != nil {
		t.Errorf("ParseOrExit(--help) = (%v, %d), want (nil, 0)", namespace, code)
	}
	if !bytes.Contains(out.Bytes(), []byte("Usage:")) {
		t.Errorf("帮助未打印 usage: %q", out.String())
	}
}

// TestRootParserUnknownOption 未知选项按用法错误退出 2
func TestRootParserUnknownOption(t *testing.T) {
	parser, err := newRootParser()
	if err != nil {
		t.Fatalf("newRootParser: %v", err)
	}
	var out, errOut bytes.Buffer
	namespace, code := parser.ParseOrExit([]string{"--no-such-flag"}, &out, &errOut)
	if code != 2 || namespace != nil {
		t.Errorf("ParseOrExit(未知选项) = (%v, %d), want (nil, 2)", namespace, code)
	}
	if errOut.Len() == 0 {
		t.Error("用法错误应写到 stderr")
	}
}
