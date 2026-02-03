package std

import (
	"os"
	"os/user"
	"path/filepath"
	"testing"
)

// patchEnv 临时修改或取消环境变量，并返回恢复原值的函数
//
// 参数:
//
//	key - 要修改的环境变量名
//	value - 要设置的新值，空字符串表示取消该变量
//
// 返回值:
//
//	返回一个函数，调用该函数可将环境变量恢复为原值
func patchEnv(key, value string) func() {
	bck := os.Getenv(key)
	deferFunc := func() {
		os.Setenv(key, bck)
	}

	if value != "" {
		os.Setenv(key, value)
	} else {
		os.Unsetenv(key)
	}

	return deferFunc
}

func BenchmarkHomeDir(b *testing.B) {
	// We do this for any "warmups"
	for i := 0; i < 10; i++ {
		HomeDir()
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		HomeDir()
	}
}

// TestHomeDir 测试HomeDir函数是否正确返回当前用户的主目录路径
//
// 测试场景包括：
//  1. 正常情况下的主目录获取
//  2. 禁用缓存后的主目录获取
//  3. 环境变量HOME为空时的主目录获取
func TestHomeDir(t *testing.T) {
	u, err := user.Current()
	if err != nil {
		t.Fatalf("err: %s", err)
	}

	dir, err := HomeDir()
	if err != nil {
		t.Fatalf("err: %s", err)
	}

	if u.HomeDir != dir {
		t.Fatalf("%#v != %#v", u.HomeDir, dir)
	}

	DisableCache = true
	defer func() { DisableCache = false }()
	defer patchEnv("HOME", "")()
	dir, err = HomeDir()
	if err != nil {
		t.Fatalf("err: %s", err)
	}

	if u.HomeDir != dir {
		t.Fatalf("%#v != %#v", u.HomeDir, dir)
	}
}

// TestExpandUser 测试 ExpandUser 函数的功能
//
// 测试用例包括：
//   - 普通路径处理
//   - 用户主目录(~)扩展
//   - 空字符串处理
//   - 无效用户目录(~foo)处理
//   - 环境变量 QUANT1X_HOME 和 GOX_HOME 的优先级测试
func TestExpandUser(t *testing.T) {
	u, err := user.Current()
	if err != nil {
		t.Fatalf("err: %s", err)
	}

	cases := []struct {
		Input  string
		Output string
		Err    bool
	}{
		{
			"/foo",
			"/foo",
			false,
		},

		{
			"~/foo",
			filepath.Join(u.HomeDir, "foo"),
			false,
		},

		{
			"",
			"",
			false,
		},

		{
			"~",
			u.HomeDir,
			false,
		},

		{
			"~foo/foo",
			"",
			true,
		},
	}

	for _, tc := range cases {
		actual, err := ExpandUser(tc.Input)
		if (err != nil) != tc.Err {
			t.Fatalf("Input: %#v\n\nErr: %s", tc.Input, err)
		}

		if actual != tc.Output {
			t.Fatalf("Input: %#v\n\nOutput: %#v", tc.Input, actual)
		}
	}

	DisableCache = true
	defer func() { DisableCache = false }()
	defer patchEnv("QUANT1X_HOME", "/custom/q1x")()
	defer patchEnv("GOX_HOME", "/custom/path/")()

	// Test QUANT1X_HOME priority
	expected := filepath.Join("/", "custom", "q1x", "foo/bar")
	actual, err := ExpandUser("~/foo/bar")
	if err != nil {
		t.Errorf("No error is expected, got: %v", err)
	} else if actual != expected {
		t.Errorf("Expected: %v; actual: %v", expected, actual)
	}

	// Test GOX_HOME priority (when QUANT1X_HOME is unset)
	os.Unsetenv("QUANT1X_HOME")
	expected = filepath.Join("/", "custom", "path", "foo/bar")
	actual, err = ExpandUser("~/foo/bar")

	if err != nil {
		t.Errorf("No error is expected, got: %v", err)
	} else if actual != expected {
		t.Errorf("Expected: %v; actual: %v", expected, actual)
	}
}
