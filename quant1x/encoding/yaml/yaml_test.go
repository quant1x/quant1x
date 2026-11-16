// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package yaml

import (
	"strings"
	"testing"
	"time"
)

// sampleConfig 覆盖标量、切片、嵌套结构、map 与内嵌结构体的默认值场景
type sampleConfig struct {
	Name    string            `yaml:"name" default:"demo"`
	Total   int               `yaml:"total" default:"3"`
	Ratio   float64           `yaml:"ratio" default:"0.5"`
	Enabled bool              `yaml:"enabled" default:"true"`
	Timeout time.Duration     `yaml:"timeout" default:"5s"`
	Ignore  []string          `yaml:"ignore" default:"[\"sh68\",\"bj\"]"`
	Extras  map[string]string `yaml:"extras" default:"{\"k\":\"v\"}"`

	Nested sampleNested            `yaml:"nested"`
	Items  []sampleNested          `yaml:"items"`
	Rules  map[string]sampleNested `yaml:"rules"`

	inner string `yaml:"inner" default:"hidden"`
	Skip  string `yaml:"-" default:"skipped"`
}

type sampleNested struct {
	Level int    `yaml:"level" default:"7"`
	Label string `yaml:"label" default:"nested"`
}

func TestUnmarshalAppliesDefaults(t *testing.T) {
	var cfg sampleConfig
	if err := Unmarshal([]byte("{}"), &cfg); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if cfg.Name != "demo" || cfg.Total != 3 || cfg.Ratio != 0.5 || !cfg.Enabled {
		t.Errorf("标量默认值未生效: %+v", cfg)
	}
	if cfg.Timeout != 5*time.Second {
		t.Errorf("time.Duration 默认值未生效: %v", cfg.Timeout)
	}
	if len(cfg.Ignore) != 2 || cfg.Ignore[0] != "sh68" || cfg.Ignore[1] != "bj" {
		t.Errorf("切片默认值未生效: %v", cfg.Ignore)
	}
	if cfg.Extras["k"] != "v" {
		t.Errorf("map 默认值未生效: %v", cfg.Extras)
	}
	if cfg.Nested.Level != 7 || cfg.Nested.Label != "nested" {
		t.Errorf("嵌套结构默认值未生效: %+v", cfg.Nested)
	}
	if cfg.inner != "" {
		t.Errorf("未导出字段不应参与默认值填充: %q", cfg.inner)
	}
	if cfg.Skip != "" {
		t.Errorf("yaml:\"-\" 字段不应参与默认值填充: %q", cfg.Skip)
	}
}

func TestUnmarshalExplicitValueWins(t *testing.T) {
	// yaml 中显式出现的零值以 yaml 为准, 不被 default 覆盖
	src := "total: 0\nenabled: false\nname: \"\"\n"
	var cfg sampleConfig
	if err := Unmarshal([]byte(src), &cfg); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if cfg.Total != 0 || cfg.Enabled || cfg.Name != "" {
		t.Errorf("显式零值被默认值覆盖: %+v", cfg)
	}
	// 未出现的字段仍然取默认值
	if cfg.Ratio != 0.5 || cfg.Nested.Level != 7 {
		t.Errorf("未出现的字段未取默认值: %+v", cfg)
	}
}

func TestUnmarshalNestedSliceAndMap(t *testing.T) {
	src := strings.Join([]string{
		"nested:",
		"  level: 1",
		"items:",
		"  - level: 2",
		"rules:",
		"  r1:",
		"    label: rule",
	}, "\n")
	var cfg sampleConfig
	if err := Unmarshal([]byte(src), &cfg); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if cfg.Nested.Level != 1 || cfg.Nested.Label != "nested" {
		t.Errorf("嵌套结构逐层默认值不正确: %+v", cfg.Nested)
	}
	if len(cfg.Items) != 1 || cfg.Items[0].Level != 2 || cfg.Items[0].Label != "nested" {
		t.Errorf("切片元素默认值不正确: %+v", cfg.Items)
	}
	if cfg.Rules["r1"].Label != "rule" || cfg.Rules["r1"].Level != 7 {
		t.Errorf("map 值默认值不正确: %+v", cfg.Rules)
	}
}

func TestUnmarshalEmptyDocument(t *testing.T) {
	cfg := sampleConfig{Name: "kept"}
	for _, src := range []string{"", "# 只有注释\n"} {
		if err := Unmarshal([]byte(src), &cfg); err != nil {
			t.Errorf("空文档不应报错: %v", err)
		}
		if cfg.Name != "kept" {
			t.Errorf("空文档不应改写目标: %q", cfg.Name)
		}
	}
}

func TestUnmarshalInvalidOutput(t *testing.T) {
	var cfg sampleConfig
	if err := Unmarshal([]byte("{}"), cfg); err != errYamlOutput {
		t.Errorf("非指针输出应返回 errYamlOutput, 实际: %v", err)
	}
	var nilCfg *sampleConfig
	if err := Unmarshal([]byte("{}"), nilCfg); err != errYamlOutput {
		t.Errorf("nil 指针输出应返回 errYamlOutput, 实际: %v", err)
	}
}

// textValue 实现 encoding.TextUnmarshaler, 默认值走字段自己的解析
type textValue string

func (v *textValue) UnmarshalText(data []byte) error {
	*v = textValue("text:" + string(data))
	return nil
}

func TestUnmarshalTextUnmarshalerDefault(t *testing.T) {
	var cfg struct {
		Value textValue `yaml:"value" default:"abc"`
	}
	if err := Unmarshal([]byte("{}"), &cfg); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if cfg.Value != "text:abc" {
		t.Errorf("TextUnmarshaler 默认值未生效: %q", cfg.Value)
	}
}

// nodeValue 实现 yaml.Unmarshaler, 依赖 Node 别名与 yaml.v3 节点一致
type nodeValue struct {
	raw string
}

func (v *nodeValue) UnmarshalYAML(node *Node) error {
	v.raw = node.Value
	return nil
}

func TestUnmarshalNodeAlias(t *testing.T) {
	var cfg struct {
		Value nodeValue `yaml:"value" default:"fallback"`
	}
	if err := Unmarshal([]byte("value: hello\n"), &cfg); err != nil {
		t.Fatalf("Unmarshal: %v", err)
	}
	if cfg.Value.raw != "hello" {
		t.Errorf("Node 别名未按 yaml.v3 语义传给 UnmarshalYAML: %q", cfg.Value.raw)
	}
}

func TestMarshal(t *testing.T) {
	cfg := sampleConfig{Name: "demo", Total: 1}
	data, err := Marshal(cfg)
	if err != nil {
		t.Fatalf("Marshal: %v", err)
	}
	if !strings.Contains(string(data), "name: demo") || !strings.Contains(string(data), "total: 1") {
		t.Errorf("Marshal 输出不符合预期: %s", data)
	}
	var round sampleConfig
	if err := Unmarshal(data, &round); err != nil {
		t.Fatalf("Marshal 后 Unmarshal: %v", err)
	}
	if round.Name != "demo" || round.Total != 1 {
		t.Errorf("往返后取值不一致: %+v", round)
	}
	// Marshal 会把零值字段一并写出, 再解析时属于「yaml 中显式出现」, 因此不再套默认值
	if round.Nested.Level != 0 || round.Ratio != 0 {
		t.Errorf("往返后不应重新套用默认值: %+v", round)
	}
}
