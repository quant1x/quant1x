package core

import "testing"

type decodeSub struct {
	A int    `yaml:"a" default:"7"`
	B string `yaml:"b"`
}

type decodeRoot struct {
	Sub decodeSub `yaml:"sub"`
}

func TestDecodeTo_MapToStruct(t *testing.T) {
	src := map[string]any{
		"a": 1,
		"b": "x",
	}
	var dst decodeSub
	if err := DecodeTo(&dst, src); err != nil {
		t.Fatalf("DecodeTo error: %v", err)
	}
	if dst.A != 1 || dst.B != "x" {
		t.Fatalf("unexpected dst: %+v", dst)
	}
}

func TestLookupConfig_AndDecodeConfig(t *testing.T) {
	// Trigger loading of config
	_ = GetConfigMapRef()
	// Now set our test config
	cacheCfg.ConfigMap = map[string]any{
		"root": map[string]any{
			"sub": map[string]any{
				"b": "hello",
			},
		},
	}

	var out decodeSub
	if err := DecodeConfig("root.sub", &out); err != nil {
		t.Fatalf("DecodeConfig error: %v", err)
	}
	// A is missing -> should be defaulted by ApplyDefaults
	if out.A != 7 || out.B != "hello" {
		t.Fatalf("unexpected out: %+v", out)
	}
}

func TestDecodeTo_BasicTypes(t *testing.T) {
	var b bool
	if err := DecodeTo(&b, true); err != nil {
		t.Fatalf("bool: %v", err)
	}
	if b != true {
		t.Fatalf("bool: want true, got %v", b)
	}

	var i int
	if err := DecodeTo(&i, 42); err != nil {
		t.Fatalf("int: %v", err)
	}
	if i != 42 {
		t.Fatalf("int: want 42, got %v", i)
	}

	var f float64
	if err := DecodeTo(&f, 3.14); err != nil {
		t.Fatalf("float64: %v", err)
	}
	if f != 3.14 {
		t.Fatalf("float64: want 3.14, got %v", f)
	}

	var s string
	if err := DecodeTo(&s, "hello"); err != nil {
		t.Fatalf("string: %v", err)
	}
	if s != "hello" {
		t.Fatalf("string: want 'hello', got %v", s)
	}

	// 类型可转换
	var i64 int64
	if err := DecodeTo(&i64, 123); err != nil {
		t.Fatalf("int64: %v", err)
	}
	if i64 != 123 {
		t.Fatalf("int64: want 123, got %v", i64)
	}
}
