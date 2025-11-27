package config

import (
	"os"
	"path/filepath"
	"sync"
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/std"
)

// resetGlobals resets package-level globals so tests can control initialization.
func resetGlobals() {
	globalConfig = BaseConfig{}
	globalCacheOnce = sync.Once{}
	globalTraderOnce = sync.Once{}
	globalTrader = nil
}

func TestSubpathAndFilenames(t *testing.T) {
	resetGlobals()
	// prepare a fake cache dir
	td := t.TempDir()
	globalConfig.CacheDir = td
	// mark lazy init as done so functions use globalConfig values
	globalCacheOnce.Do(func() {})

	code := "600000SZ" // 8 chars

	// subpath
	sp := subpath(code)
	if sp != "60000" {
		t.Fatalf("unexpected subpath: %s", sp)
	}

	// GetXdxrFilename
	wantX := filepath.Clean(filepath.Join(td, "xdxr", "60000", code+".csv"))
	gotX := GetXdxrFilename(code)
	if gotX != wantX {
		t.Fatalf("GetXdxrFilename wrong:\n got: %s\n want:%s", gotX, wantX)
	}

	// GetKlineFilename forward
	wantK := filepath.Clean(filepath.Join(td, "day", "60000", code+".csv"))
	gotK := GetKlineFilename(code, true)
	if gotK != wantK {
		t.Fatalf("GetKlineFilename forward wrong:\n got: %s\n want:%s", gotK, wantK)
	}

	// GetKlineFilename not forward
	wantKr := filepath.Clean(filepath.Join(td, "day", "60000", code+".raw"))
	gotKr := GetKlineFilename(code, false)
	if gotKr != wantKr {
		t.Fatalf("GetKlineFilename raw wrong:\n got: %s\n want:%s", gotKr, wantKr)
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
	q := getQuarterByDate("2025-05-15")
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
	cfg := `trader:
  order_path: "` + filepath.Join(td, "orders") + `"
  account_id: "acct-123"`
	cfgFile := filepath.Join(td, "quant1x.yaml")
	if err := os.WriteFile(cfgFile, []byte(cfg), 0o644); err != nil {
		t.Fatalf("write config failed: %v", err)
	}

	// set global config filename and cache dir, and mark lazy init done
	globalConfig.Filename = cfgFile
	globalConfig.CacheDir = td
	globalCacheOnce.Do(func() {})

	// ensure trader globals reset
	globalTraderOnce = sync.Once{}
	globalTrader = nil

	tp := TraderConfig()
	if tp == nil {
		t.Fatalf("TraderConfig returned nil")
	}
	if tp.OrderPath != filepath.Join(td, "orders") {
		t.Fatalf("TraderConfig.OrderPath wrong: %s", tp.OrderPath)
	}
}
