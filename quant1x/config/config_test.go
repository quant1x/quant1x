package config

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"

	"github.com/quant1x/quant1x/quant1x/std"
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

func TestCacheId(t *testing.T) {
	tests := []struct {
		name     string
		input    string
		expected string
	}{
		// A股 - 上交所
		{
			name:     "SSE stock with prefix",
			input:    "sh600000",
			expected: "sh600000",
		},
		{
			name:     "SSE stock with suffix",
			input:    "600000.sh",
			expected: "sh600000",
		},
		{
			name:     "SSE stock pure code",
			input:    "600000",
			expected: "sh600000",
		},
		{
			name:     "SSE ETF",
			input:    "sh510300",
			expected: "sh510300",
		},
		{
			name:     "SSE Index",
			input:    "000001",
			expected: "sh000001",
		},

		// A股 - 深交所
		{
			name:     "SZSE stock with prefix",
			input:    "sz000001",
			expected: "sz000001",
		},
		{
			name:     "SZSE stock with suffix",
			input:    "000001.sz",
			expected: "sz000001",
		},
		{
			name:     "SZSE stock pure code",
			input:    "000001",
			expected: "sz000001",
		},
		{
			name:     "SZSE ETF",
			input:    "sz159919",
			expected: "sz159919",
		},

		// A股 - 北交所
		{
			name:     "BSE stock with prefix",
			input:    "bj832566",
			expected: "bj832566",
		},
		{
			name:     "BSE stock pure code",
			input:    "832566",
			expected: "bj832566",
		},

		// 港股
		{
			name:     "HKEX with prefix",
			input:    "hk00700",
			expected: "hk00700",
		},
		{
			name:     "HKEX with suffix",
			input:    "00700.hk",
			expected: "hk00700",
		},
		{
			name:     "HKEX pure code",
			input:    "00700",
			expected: "hk00700",
		},
		{
			name:     "HKEX pure code without leading zero",
			input:    "700",
			expected: "hk00700",
		},

		// 美股
		{
			name:     "US stock lowercase",
			input:    "aapl",
			expected: "us.aapl",
		},
		{
			name:     "US stock uppercase",
			input:    "AAPL",
			expected: "us.aapl",
		},
		{
			name:     "US stock mixed case",
			input:    "AaPl",
			expected: "us.aapl",
		},
		{
			name:     "US stock with prefix",
			input:    "usmsft",
			expected: "us.msft",
		},
		{
			name:     "US stock with suffix",
			input:    "MSFT.us",
			expected: "us.msft",
		},

		// 边界情况 - 空字符串
		{
			name:     "empty string",
			input:    "",
			expected: "",
		},

		// 边界情况 - 空格
		{
			name:     "whitespace only",
			input:    "   ",
			expected: "",
		},

		// 边界情况 - 带空格的输入
		{
			name:     "SSE code with whitespace",
			input:    " 600000 ",
			expected: "sh600000",
		},

		// 无法识别的代码
		{
			name:     "unrecognized short code",
			input:    "123",
			expected: "",
		},
		{
			name:     "unrecognized mixed alphanumeric",
			input:    "a1b2",
			expected: "",
		},

		// 特殊的A股代码
		{
			name:     "SSE bond",
			input:    "sh010107",
			expected: "sh010107",
		},
		{
			name:     "SZSE bond",
			input:    "sz123456",
			expected: "sz123456",
		},
		{
			name:     "SSE stock B",
			input:    "sh900901",
			expected: "sh900901",
		},

		// 板块指数
		{
			name:     "SSE block index",
			input:    "880001",
			expected: "sh880001",
		},
		{
			name:     "SZSE block index",
			input:    "399001",
			expected: "sz399001",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got := CacheId(tt.input)
			if got != tt.expected {
				t.Errorf("CacheId(%q) = %q, want %q", tt.input, got, tt.expected)
			}
		})
	}
}
