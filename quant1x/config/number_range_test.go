package config

import (
	"testing"

	"gopkg.in/yaml.v3"
)

func TestParseScalarRange(t *testing.T) {
	var r NumberRange
	r.ParseFromString("0.382~2.800")
	if r.Min != 0.382 {
		t.Fatalf("unexpected min: %v", r.Min)
	}
	if r.Max != 2.8 {
		t.Fatalf("unexpected max: %v", r.Max)
	}

	r.ParseFromString("80~")
	if r.Min != 80 {
		t.Fatalf("unexpected min for '80~': %v", r.Min)
	}

	r.ParseFromString("~2.5")
	if r.Max != 2.5 {
		t.Fatalf("unexpected max for '~2.5': %v", r.Max)
	}

	r.ParseFromString("5")
	if r.Min != 5 {
		t.Fatalf("unexpected min for '5': %v", r.Min)
	}
}

func TestUnmarshalYAMLScalar(t *testing.T) {
	var r NumberRange
	src := `
safety: "38.2~61.80"
`
	var cfg map[string]NumberRange
	if err := yaml.Unmarshal([]byte(src), &cfg); err != nil {
		t.Fatalf("yaml unmarshal error: %v", err)
	}
	r = cfg["safety"]
	if r.Min == 0 {
		t.Fatalf("expected non-zero min from scalar YAML")
	}
}

func TestUnmarshalYAMLMap(t *testing.T) {
	var r NumberRange
	src := `
val:
    min: 1
    max: 2
`
	var cfg map[string]NumberRange
	if err := yaml.Unmarshal([]byte(src), &cfg); err != nil {
		t.Fatalf("yaml unmarshal error: %v", err)
	}
	r = cfg["val"]
	if r.Min != 1 || r.Max != 2 {
		t.Fatalf("unexpected map-unmarshal values: %v", r)
	}
}

func TestValidateSpecialZeroZero(t *testing.T) {
	r := NumberRange{Min: 0, Max: 0}
	if !r.Validate(123.45) {
		t.Fatalf("validate should be true when Min==0 && Max==0")
	}
}
