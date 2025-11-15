package config

import (
	"fmt"
	"math"
	"strconv"
	"strings"

	"gopkg.in/yaml.v3"
)

// NumberRange mirrors C++ numerics::number_range<T>
// - default construction -> Min = -Inf, Max = +Inf
// - supports scalar YAML like "0.382~2.800" and map forms {min: x, max: y}
// - Validate follows C++ semantics: if Min==0 && Max==0 => always true; else v >= Min && v < Max
type NumberRange struct {
	Min float64 `yaml:"min" json:"min"`
	Max float64 `yaml:"max" json:"max"`
}

// newNumberRangeDefault returns the default range (lowest..max)
func newNumberRangeDefault() NumberRange {
	return NumberRange{Min: math.Inf(-1), Max: math.Inf(1)}
}

// ParseFromString parses string forms similar to C++ number_range(string)
// Supported forms:
//   - "x~y"  => min=x, max=y
//   - "x~"   => min=x, max=+Inf
//   - "~y"   => min=-Inf, max=y
//   - "x"    => min=x, max=+Inf (single value interpreted as min)
//   - "~"    => default (min=-Inf, max=+Inf)
func (r *NumberRange) ParseFromString(s string) {
	*r = newNumberRangeDefault()
	text := strings.TrimSpace(s)
	if text == "" {
		return
	}
	pos := strings.IndexRune(text, '~')
	if pos == -1 {
		// no separator: treat as single min value
		if v, err := parseNumber(text); err == nil {
			r.Min = v
			r.Max = math.Inf(1)
		}
		return
	}
	smin := strings.TrimSpace(text[:pos])
	smax := strings.TrimSpace(text[pos+1:])
	if smin == "" && smax == "" {
		// both empty -> default
		return
	}
	if smin == "" {
		if v, err := parseNumber(smax); err == nil {
			r.Max = v
		}
		return
	}
	if smax == "" {
		if v, err := parseNumber(smin); err == nil {
			r.Min = v
			r.Max = math.Inf(1)
		}
		return
	}
	// both present
	if vmin, err := parseNumber(smin); err == nil {
		r.Min = vmin
	}
	if vmax, err := parseNumber(smax); err == nil {
		r.Max = vmax
	}
}

func parseNumber(text string) (float64, error) {
	return strconv.ParseFloat(text, 64)
}

// Validate follows C++ semantics: if Min==0 && Max==0 -> always true
func (r NumberRange) Validate(v float64) bool {
	if r.Min == 0 && r.Max == 0 {
		return true
	}
	return v >= r.Min && v < r.Max
}

func (r NumberRange) String() string {
	return fmt.Sprintf("{min:%v, max:%v}", r.Min, r.Max)
}

// UnmarshalYAML implements custom YAML parsing to accept scalar and map forms
func (r *NumberRange) UnmarshalYAML(value *yaml.Node) error {
	// default
	*r = newNumberRangeDefault()
	switch value.Kind {
	case yaml.ScalarNode:
		var s string
		if err := value.Decode(&s); err != nil {
			return err
		}
		r.ParseFromString(s)
		return nil
	case yaml.MappingNode:
		// decode into temporary map for keys min/max
		var m map[string]float64
		if err := value.Decode(&m); err == nil {
			if v, ok := m["min"]; ok {
				r.Min = v
			}
			if v, ok := m["max"]; ok {
				r.Max = v
			}
			return nil
		}
		// fallback: try decoding with interface to allow ints/other numeric types
		var mm map[string]interface{}
		if err := value.Decode(&mm); err != nil {
			return err
		}
		if v, ok := mm["min"]; ok {
			if fv, err := toFloat64(v); err == nil {
				r.Min = fv
			}
		}
		if v, ok := mm["max"]; ok {
			if fv, err := toFloat64(v); err == nil {
				r.Max = fv
			}
		}
		return nil
	default:
		return fmt.Errorf("unsupported YAML node for NumberRange: kind=%d", value.Kind)
	}
}

func toFloat64(v interface{}) (float64, error) {
	switch x := v.(type) {
	case int:
		return float64(x), nil
	case int64:
		return float64(x), nil
	case float32:
		return float64(x), nil
	case float64:
		return x, nil
	case string:
		return strconv.ParseFloat(strings.TrimSpace(x), 64)
	default:
		return 0, fmt.Errorf("unsupported numeric type")
	}
}
