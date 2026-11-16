// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package base

import (
	"embed"
	"os"
	"testing"
)

//go:embed *
var resources embed.FS

func TestEmbed(t *testing.T) {
	filename := "embed.go"

	file, err := OpenEmbed(resources, filename)
	if err != nil {
		t.Fatalf("OpenEmbed(%q) 失败: %v", filename, err)
	}
	if _, err := file.Stat(); err != nil {
		t.Errorf("Stat(%q) 失败: %v", filename, err)
	}
	if err := file.Close(); err != nil {
		t.Errorf("Close(%q) 失败: %v", filename, err)
	}

	// 导出目标用 .gitignore 收录的扩展名, 避免用例在源码目录留下可入库的文件
	target := "embed_test.out"
	t.Cleanup(func() { _ = os.Remove(target) })
	if err := Export(resources, filename, target); err != nil {
		t.Fatalf("Export(%q, %q) 失败: %v", filename, target, err)
	}
	if _, err := os.Stat(target); err != nil {
		t.Errorf("导出文件不存在: %v", err)
	}
}
