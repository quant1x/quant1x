package config

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

// resetGlobals resets package-level globals so tests can control initialization.
func resetGlobals() {
	// reset trader globals only
	globalTraderOnce = sync.Once{}
	globalTrader = nil
}

func TestFilenames(t *testing.T) {
	resetGlobals()
	code := "600000SZ" // 8 chars

	// GetXdxrFilename: 应该包含 xdxr 目录并以 <cacheid>.csv 结尾
	gotX := GetXdxrFilename(code)
	gotXSlash := strings.ReplaceAll(gotX, "\\", "/")
	if !strings.Contains(gotXSlash, "xdxr") || !strings.HasSuffix(gotXSlash, CacheIdPath(code)+".csv") {
		t.Fatalf("GetXdxrFilename wrong: %s", gotX)
	}

	// GetKlineFilename forward: 包含 day 目录并以 .csv 结尾
	gotK := GetKlineFilename(code, true)
	gotKSlash := strings.ReplaceAll(gotK, "\\", "/")
	if !strings.Contains(gotKSlash, "day") || !strings.HasSuffix(gotKSlash, CacheIdPath(code)+".csv") {
		t.Fatalf("GetKlineFilename forward wrong: %s", gotK)
	}

	// GetKlineFilename not forward: 包含 day 目录并以 .raw 结尾
	gotKr := GetKlineFilename(code, false)
	gotKrSlash := strings.ReplaceAll(gotKr, "\\", "/")
	if !strings.Contains(gotKrSlash, "day") || !strings.HasSuffix(gotKrSlash, CacheIdPath(code)+".raw") {
		t.Fatalf("GetKlineFilename raw wrong: %s", gotKr)
	}
}

func TestCacheIdPath(t *testing.T) {
	resetGlobals()
	// test CacheIdPath using code containing market suffix
	code := "600000SZ" // DetectMarket lowercases internally and treats trailing 'sz'
	got := CacheIdPath(code)
	// expected: prefix (cacheId without last 3) + "/" + cacheId
	// CacheId("600000SZ") -> "sz600000"; prefix = first len-3 = first 5 = "sz600"
	want := "sz600/sz600000"
	if got != want {
		t.Fatalf("CacheIdPath wrong: got=%s want=%s", got, want)
	}
}

func TestQuarterAndExpandHome(t *testing.T) {
	resetGlobals()
	q, _, _ := std.GetQuarterByDate("2025-05-15", 0)
	if q != "2025Q2" {
		t.Fatalf("quarter wrong: %s", q)
	}

	// expandHome for ~/ should equal user's home dir
	home, _ := std.ExpandUser("~")
	if home == "" {
		t.Fatalf("expandHome returned empty")
	}
}

func TestTraderConfigLoad(t *testing.T) {
	resetGlobals()
	td := t.TempDir()
	// write a small quant1x.yaml with trader section
	// Escape backslashes for YAML string
	orderPath := filepath.Join(td, "orders")
	yamlOrderPath := strings.ReplaceAll(orderPath, "\\", "\\\\")
	cfg := `trader:
  order_path: "` + yamlOrderPath + `"
  account_id: "acct-123"`
	cfgFile := filepath.Join(td, "quant1x.yaml")
	if err := os.WriteFile(cfgFile, []byte(cfg), 0o644); err != nil {
		t.Fatalf("write config failed: %v", err)
	}

	// directly load trader config from YAML file (no package-level overrides)
	tp := loadTraderConfigFromYAML(cfgFile)
	if tp.OrderPath != filepath.Join(td, "orders") {
		t.Fatalf("TraderConfig.OrderPath wrong: %s", tp.OrderPath)
	}
}
