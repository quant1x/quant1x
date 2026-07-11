package argparse

import (
	"errors"
	"fmt"
	"math"
	"os"
	"reflect"
	"sort"
	"strconv"
	"strings"
)

const reprMaxContainerSize = 5

// reprAny mirrors argparse::details::repr used for default-value formatting.
// Notably, containers are rendered as: {a b c...z} (space-separated, no commas).
func reprAny(val any) string {
	if val == nil {
		return "<not representable>"
	}

	// Unwrap pointers/interfaces.
	rv := reflect.ValueOf(val)
	for rv.IsValid() && (rv.Kind() == reflect.Pointer || rv.Kind() == reflect.Interface) {
		if rv.IsNil() {
			return "<not representable>"
		}
		rv = rv.Elem()
	}
	if !rv.IsValid() {
		return "<not representable>"
	}

	switch rv.Kind() {
	case reflect.Bool:
		if rv.Bool() {
			return "true"
		}
		return "false"
	case reflect.String:
		return "\"" + rv.String() + "\""
	case reflect.Int8:
		return string(rune(byte(int8(rv.Int()))))
	case reflect.Uint8:
		return string(rune(byte(rv.Uint())))
	case reflect.Int, reflect.Int16, reflect.Int32, reflect.Int64:
		return strconv.FormatInt(rv.Int(), 10)
	case reflect.Uint, reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Uintptr:
		return strconv.FormatUint(rv.Uint(), 10)
	case reflect.Float32:
		return floatReprCpp(rv.Float(), 32)
	case reflect.Float64:
		return floatReprCpp(rv.Float(), 64)
	case reflect.Slice, reflect.Array:
		// Treat byte slice as container, not string.
		n := rv.Len()
		var sb strings.Builder
		sb.WriteString("{")
		if n > 1 {
			sb.WriteString(reprAny(rv.Index(0).Interface()))
			limit := n
			if limit > reprMaxContainerSize {
				limit = reprMaxContainerSize
			}
			// Print elements 2..(limit-1), last is printed separately.
			for i := 1; i < limit-1; i++ {
				sb.WriteString(" ")
				sb.WriteString(reprAny(rv.Index(i).Interface()))
			}
			if n <= reprMaxContainerSize {
				sb.WriteString(" ")
			} else {
				sb.WriteString("...")
			}
		}
		if n > 0 {
			sb.WriteString(reprAny(rv.Index(n - 1).Interface()))
		}
		sb.WriteString("}")
		return sb.String()
	default:
		return "<not representable>"
	}
}

func floatReprCpp(f float64, bits int) string {
	if math.IsNaN(f) {
		return "nan"
	}
	if math.IsInf(f, 1) {
		return "inf"
	}
	if math.IsInf(f, -1) {
		return "-inf"
	}
	// C++ ostringstream default format is defaultfloat with precision 6
	// (significant digits).
	s := strconv.FormatFloat(f, 'g', 6, bits)
	// Go may produce "-0"; C++ streams typically show "0".
	if s == "-0" || s == "-0.0" {
		return "0"
	}
	return s
}

type NArgsPattern int

const (
	NArgsOptional NArgsPattern = iota
	NArgsAny
	NArgsAtLeastOne
)

type nArgsRange struct {
	min uint64
	max uint64
}

func newNArgsRange(min, max uint64) nArgsRange {
	if min > max {
		panic("Range of number of arguments is invalid")
	}
	return nArgsRange{min: min, max: max}
}

func (r nArgsRange) contains(v uint64) bool { return v >= r.min && v <= r.max }
func (r nArgsRange) isExact() bool          { return r.min == r.max }
func (r nArgsRange) isRightBounded() bool   { return r.max < math.MaxUint64 }

type valuedAction func(string) (any, error)
type voidAction func(string) error

type actionVariant struct {
	valued valuedAction
	voided voidAction
}

// Argument represents a command line argument (aligned to argparse.hpp semantics).
type Argument struct {
	names    []string
	usedName string

	help    string
	metavar string

	defaultValue     any
	defaultValueStr  *string
	defaultValueRepr string

	implicitValue any

	choices []string

	actions       []actionVariant
	defaultAction actionVariant

	values  []any
	numArgs nArgsRange

	acceptsOptionalLikeValue bool
	isOptional               bool
	isRequired               bool
	isRepeatable             bool
	isUsed                   bool
	isHidden                 bool

	prefixChars         string
	usageNewlineCounter int
	groupIdx            uint64
}

// Help sets help text.
func (a *Argument) Help(text string) *Argument {
	a.help = text
	return a
}

// Metavar sets metavar.
func (a *Argument) Metavar(v string) *Argument {
	a.metavar = v
	return a
}

// Required marks required.
func (a *Argument) Required() *Argument {
	a.isRequired = true
	return a
}

// Append allows repeatable usage (like .append() in argparse.hpp).
func (a *Argument) Append() *Argument {
	a.isRepeatable = true
	return a
}

// Hidden hides from help/usage.
func (a *Argument) Hidden() *Argument {
	a.isHidden = true
	return a
}

// Nargs sets nargs exact.
func (a *Argument) Nargs(n int) *Argument {
	if n < 0 {
		// keep current
		return a
	}
	a.numArgs = newNArgsRange(uint64(n), uint64(n))
	return a
}

// NargsRange sets nargs min/max.
func (a *Argument) NargsRange(min, max int) *Argument {
	if min < 0 || max < 0 {
		return a
	}
	a.numArgs = newNArgsRange(uint64(min), uint64(max))
	return a
}

// NargsPattern sets nargs per pattern.
func (a *Argument) NargsPattern(p NArgsPattern) *Argument {
	switch p {
	case NArgsOptional:
		a.numArgs = newNArgsRange(0, 1)
	case NArgsAny:
		a.numArgs = newNArgsRange(0, math.MaxUint64)
	case NArgsAtLeastOne:
		a.numArgs = newNArgsRange(1, math.MaxUint64)
	}
	return a
}

// Remaining mirrors argparse.hpp remaining(): accepts optional-like values and consumes any.
func (a *Argument) Remaining() *Argument {
	a.acceptsOptionalLikeValue = true
	return a.NargsPattern(NArgsAny)
}

// Choices sets allowed values.
func (a *Argument) Choices(choices ...string) *Argument {
	if len(choices) == 0 {
		panic("Zero choices provided")
	}
	a.choices = append([]string{}, choices...)
	return a
}

// ChoicesInt adds integer choices (base-10), matching argparse.hpp choices() accepting integers.
func (a *Argument) ChoicesInt(choices ...int) *Argument {
	if len(choices) == 0 {
		panic("Zero choices provided")
	}
	out := make([]string, 0, len(choices))
	for _, c := range choices {
		out = append(out, strconv.FormatInt(int64(c), 10))
	}
	a.choices = out
	return a
}

func (a *Argument) ChoicesInt64(choices ...int64) *Argument {
	if len(choices) == 0 {
		panic("Zero choices provided")
	}
	out := make([]string, 0, len(choices))
	for _, c := range choices {
		out = append(out, strconv.FormatInt(c, 10))
	}
	a.choices = out
	return a
}

func (a *Argument) ChoicesUint(choices ...uint) *Argument {
	if len(choices) == 0 {
		panic("Zero choices provided")
	}
	out := make([]string, 0, len(choices))
	for _, c := range choices {
		out = append(out, strconv.FormatUint(uint64(c), 10))
	}
	a.choices = out
	return a
}

func (a *Argument) ChoicesUint64(choices ...uint64) *Argument {
	if len(choices) == 0 {
		panic("Zero choices provided")
	}
	out := make([]string, 0, len(choices))
	for _, c := range choices {
		out = append(out, strconv.FormatUint(c, 10))
	}
	a.choices = out
	return a
}

// DefaultValue sets a default value and adjusts nargs min to 0.
func (a *Argument) DefaultValue(v any) *Argument {
	a.numArgs = newNArgsRange(0, a.numArgs.max)
	a.defaultValue = v
	a.defaultValueRepr = reprAny(v)
	// Only string/integer defaults participate in string-based choices validation.
	a.defaultValueStr = nil
	switch vv := v.(type) {
	case string:
		a.defaultValueStr = &vv
	case int:
		s := strconv.FormatInt(int64(vv), 10)
		a.defaultValueStr = &s
	case int8:
		s := strconv.FormatInt(int64(vv), 10)
		a.defaultValueStr = &s
	case int16:
		s := strconv.FormatInt(int64(vv), 10)
		a.defaultValueStr = &s
	case int32:
		s := strconv.FormatInt(int64(vv), 10)
		a.defaultValueStr = &s
	case int64:
		s := strconv.FormatInt(vv, 10)
		a.defaultValueStr = &s
	case uint:
		s := strconv.FormatUint(uint64(vv), 10)
		a.defaultValueStr = &s
	case uint8:
		s := strconv.FormatUint(uint64(vv), 10)
		a.defaultValueStr = &s
	case uint16:
		s := strconv.FormatUint(uint64(vv), 10)
		a.defaultValueStr = &s
	case uint32:
		s := strconv.FormatUint(uint64(vv), 10)
		a.defaultValueStr = &s
	case uint64:
		s := strconv.FormatUint(vv, 10)
		a.defaultValueStr = &s
	}
	return a
}

// ImplicitValue sets implicit value and forces nargs(0).
func (a *Argument) ImplicitValue(v any) *Argument {
	a.implicitValue = v
	a.numArgs = newNArgsRange(0, 0)
	return a
}

// Flag is shorthand for default false + implicit true + nargs(0).
func (a *Argument) Flag() *Argument {
	a.DefaultValue(false)
	a.ImplicitValue(true)
	a.Nargs(0)
	return a
}

// Action adds a valued action. The returned value is stored into Argument values.
func (a *Argument) Action(f func(string) (any, error)) *Argument {
	a.actions = append(a.actions, actionVariant{valued: f})
	return a
}

// ActionVoid adds a void action.
func (a *Argument) ActionVoid(f func(string) error) *Argument {
	a.actions = append(a.actions, actionVariant{voided: f})
	return a
}

// StoreInto is a compatibility API: binds an action that writes into dest.
func (a *Argument) StoreInto(dest any) *Argument {
	// Mimic argparse.hpp store_into semantics: initialize dest with default if present.
	if a.defaultValue != nil {
		_ = assignDefault(dest, a.defaultValue)
	}

	// If bool and neither default nor implicit exist, treat as flag.
	if _, ok := dest.(*bool); ok {
		if a.defaultValue == nil && a.implicitValue == nil {
			a.Flag()
		}
		// store_into(bool) sets var true when used.
		a.Action(func(_ string) (any, error) {
			*(dest.(*bool)) = true
			return *(dest.(*bool)), nil
		})
		return a
	}

	// slices: append behavior, but only allowed if repeatable or nargs > 1
	if _, ok := dest.(*[]string); ok {
		a.Action(func(s string) (any, error) {
			v := dest.(*[]string)
			if !a.isUsed {
				// mimic argparse.hpp: clear on first actual use
				*v = (*v)[:0]
				// store_into(vector<...>) flips used on first element
				a.isUsed = true
			}
			*v = append(*v, s)
			return s, nil
		})
		return a
	}

	if _, ok := dest.(*[]int); ok {
		a.Action(func(s string) (any, error) {
			vi := dest.(*[]int)
			if !a.isUsed {
				*vi = (*vi)[:0]
				a.isUsed = true
			}
			n64, err := parseIntDecStrictBitSize(s, strconv.IntSize)
			if err != nil {
				return nil, err
			}
			n := int(n64)
			*vi = append(*vi, n)
			return n, nil
		})
		return a
	}

	// scalar types
	switch dest.(type) {
	case *string:
		a.Action(func(s string) (any, error) {
			*(dest.(*string)) = s
			return *(dest.(*string)), nil
		})
	case *int:
		a.Action(func(s string) (any, error) {
			n64, err := parseIntDecStrictBitSize(s, strconv.IntSize)
			if err != nil {
				return nil, err
			}
			n := int(n64)
			*(dest.(*int)) = n
			return *(dest.(*int)), nil
		})
	case *float64:
		a.Action(func(s string) (any, error) {
			f, err := parseFloatGeneral(s, 64)
			if err != nil {
				return nil, err
			}
			*(dest.(*float64)) = f
			return *(dest.(*float64)), nil
		})
	default:
		a.Action(func(s string) (any, error) {
			// best-effort reflection assignment
			if err := assignString(dest, s); err != nil {
				return nil, err
			}
			return s, nil
		})
	}
	return a
}

// Scan configures a numeric parser action similar to argparse.hpp scan<'x', T>().
// It stores values as int64/uint64/float64, which can later be converted via GetInto().
func (a *Argument) Scan(shape rune) *Argument {
	s := byte(shape)
	// Integer shapes
	if s == 'd' || s == 'i' {
		a.Action(func(raw string) (any, error) {
			v, err := parseIntShape(raw, s)
			if err != nil {
				return nil, err
			}
			return v, nil
		})
		return a
	}
	if s == 'u' || s == 'b' || s == 'o' || s == 'x' || s == 'X' {
		a.Action(func(raw string) (any, error) {
			v, err := parseUintShape(raw, s)
			if err != nil {
				return nil, err
			}
			return v, nil
		})
		return a
	}
	// Float shapes
	if s == 'a' || s == 'A' || s == 'e' || s == 'E' || s == 'f' || s == 'F' || s == 'g' || s == 'G' {
		a.Action(func(raw string) (any, error) {
			v, err := parseFloatShape(raw, s)
			if err != nil {
				return nil, err
			}
			return v, nil
		})
		return a
	}
	panic("No scan specification")
}

func hasHexPrefix(s string) bool {
	return strings.HasPrefix(s, "0x") || strings.HasPrefix(s, "0X")
}

func hasBinaryPrefix(s string) bool {
	return strings.HasPrefix(s, "0b") || strings.HasPrefix(s, "0B")
}

func isASCIISpace(b byte) bool {
	switch b {
	case ' ', '\t', '\n', '\r', '\v', '\f':
		return true
	default:
		return false
	}
}

func hasLeadingSpaceOrPlus(s string) bool {
	if s == "" {
		return false
	}
	return isASCIISpace(s[0]) || s[0] == '+'
}

func hasDecimalTrailingJunkSigned(s string) bool {
	if s == "" {
		return false
	}
	i := 0
	if s[0] == '-' {
		i = 1
	}
	if i >= len(s) {
		return false
	}
	if s[i] < '0' || s[i] > '9' {
		return false
	}
	for ; i < len(s); i++ {
		if s[i] < '0' || s[i] > '9' {
			return true
		}
	}
	return false
}

func hasDecimalTrailingJunkUnsigned(s string) bool {
	if s == "" {
		return false
	}
	if s[0] < '0' || s[0] > '9' {
		return false
	}
	for i := 0; i < len(s); i++ {
		if s[i] < '0' || s[i] > '9' {
			return true
		}
	}
	return false
}

func parseIntDecStrictBitSize(s string, bitSize int) (int64, error) {
	if s == "" || hasLeadingSpaceOrPlus(s) {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	v, err := strconv.ParseInt(s, 10, bitSize)
	if err == nil {
		return v, nil
	}
	var numErr *strconv.NumError
	if errors.As(err, &numErr) {
		if numErr.Err == strconv.ErrRange {
			return 0, fmt.Errorf("'%s' not representable", s)
		}
	}
	if hasDecimalTrailingJunkSigned(s) {
		return 0, fmt.Errorf("pattern '%s' does not match to the end", s)
	}
	return 0, fmt.Errorf("pattern '%s' not found", s)
}

func parseUintDecStrictBitSize(s string, bitSize int) (uint64, error) {
	if s == "" || hasLeadingSpaceOrPlus(s) || strings.HasPrefix(s, "-") {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	v, err := strconv.ParseUint(s, 10, bitSize)
	if err == nil {
		return v, nil
	}
	var numErr *strconv.NumError
	if errors.As(err, &numErr) {
		if numErr.Err == strconv.ErrRange {
			return 0, fmt.Errorf("'%s' not representable", s)
		}
	}
	if hasDecimalTrailingJunkUnsigned(s) {
		return 0, fmt.Errorf("pattern '%s' does not match to the end", s)
	}
	return 0, fmt.Errorf("pattern '%s' not found", s)
}

func doStrtodStrict(s string, bitSize int) (float64, error) {
	if s == "" || hasLeadingSpaceOrPlus(s) {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	v, err := strconv.ParseFloat(s, bitSize)
	if err == nil {
		return v, nil
	}
	var numErr *strconv.NumError
	if errors.As(err, &numErr) {
		if numErr.Err == strconv.ErrRange {
			return 0, fmt.Errorf("'%s' not representable", s)
		}
	}
	if n := floatPrefixLen(s); n > 0 {
		if n < len(s) {
			return 0, fmt.Errorf("pattern '%s' does not match to the end", s)
		}
	}
	return 0, fmt.Errorf("pattern '%s' not found", s)
}

func floatPrefixLen(s string) int {
	if s == "" {
		return 0
	}
	i := 0
	if s[0] == '-' {
		i = 1
		if i >= len(s) {
			return 0
		}
	}
	rest := s[i:]
	if len(rest) >= 8 && strings.EqualFold(rest[:8], "infinity") {
		return i + 8
	}
	if len(rest) >= 3 && strings.EqualFold(rest[:3], "inf") {
		return i + 3
	}
	if len(rest) >= 3 && strings.EqualFold(rest[:3], "nan") {
		return i + 3
	}

	startDigits := i
	for i < len(s) && s[i] >= '0' && s[i] <= '9' {
		i++
	}
	digitsBefore := i - startDigits

	digitsAfter := 0
	if i < len(s) && s[i] == '.' {
		i++
		startAfter := i
		for i < len(s) && s[i] >= '0' && s[i] <= '9' {
			i++
		}
		digitsAfter = i - startAfter
	}

	if digitsBefore+digitsAfter == 0 {
		return 0
	}

	// Optional exponent. Only consume it if it is well-formed.
	if i < len(s) && (s[i] == 'e' || s[i] == 'E') {
		j := i + 1
		if j < len(s) && (s[j] == '+' || s[j] == '-') {
			j++
		}
		expStart := j
		for j < len(s) && s[j] >= '0' && s[j] <= '9' {
			j++
		}
		if j > expStart {
			i = j
		}
	}

	return i
}

func digitVal(b byte) int {
	switch {
	case b >= '0' && b <= '9':
		return int(b - '0')
	case b >= 'a' && b <= 'f':
		return int(b-'a') + 10
	case b >= 'A' && b <= 'F':
		return int(b-'A') + 10
	default:
		return -1
	}
}

func parseIntBaseStrictBitSize(s string, base int, bitSize int) (int64, error) {
	if s == "" || hasLeadingSpaceOrPlus(s) {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	i := 0
	if s[0] == '-' {
		i = 1
		if i >= len(s) {
			return 0, fmt.Errorf("pattern '%s' not found", s)
		}
	}
	start := i
	for i < len(s) {
		v := digitVal(s[i])
		if v < 0 || v >= base {
			break
		}
		i++
	}
	if i == start {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	if i != len(s) {
		return 0, fmt.Errorf("pattern '%s' does not match to the end", s)
	}
	v, err := strconv.ParseInt(s, base, bitSize)
	if err == nil {
		return v, nil
	}
	var numErr *strconv.NumError
	if errors.As(err, &numErr) {
		if numErr.Err == strconv.ErrRange {
			return 0, fmt.Errorf("'%s' not representable", s)
		}
	}
	return 0, fmt.Errorf("pattern '%s' not found", s)
}

func parseUintBaseStrictBitSize(s string, base int, bitSize int) (uint64, error) {
	if s == "" || hasLeadingSpaceOrPlus(s) || strings.HasPrefix(s, "-") {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	i := 0
	start := 0
	for i < len(s) {
		v := digitVal(s[i])
		if v < 0 || v >= base {
			break
		}
		i++
	}
	if i == start {
		return 0, fmt.Errorf("pattern '%s' not found", s)
	}
	if i != len(s) {
		return 0, fmt.Errorf("pattern '%s' does not match to the end", s)
	}
	v, err := strconv.ParseUint(s, base, bitSize)
	if err == nil {
		return v, nil
	}
	var numErr *strconv.NumError
	if errors.As(err, &numErr) {
		if numErr.Err == strconv.ErrRange {
			return 0, fmt.Errorf("'%s' not representable", s)
		}
	}
	return 0, fmt.Errorf("pattern '%s' not found", s)
}

// parseFloatGeneral mirrors argparse.hpp chars_format::general restriction: no hexfloat/binfloat.
func parseFloatGeneral(s string, bitSize int) (float64, error) {
	if hasHexPrefix(s) {
		return 0, fmt.Errorf("chars_format::general does not parse hexfloat")
	}
	if hasBinaryPrefix(s) {
		return 0, fmt.Errorf("chars_format::general does not parse binfloat")
	}
	v, err := doStrtodStrict(s, bitSize)
	if err != nil {
		return 0, fmt.Errorf("Failed to parse '%s' as number: %s", s, err.Error())
	}
	return v, nil
}

func parseIntShape(s string, shape byte) (int64, error) {
	switch shape {
	case 'd':
		v, err := parseIntDecStrictBitSize(s, 64)
		if err != nil {
			return 0, err
		}
		return v, nil
	case 'i':
		// Auto base: hex (0x), binary (0b), octal (leading 0), else decimal.
		if hasHexPrefix(s) {
			rest := s[2:]
			v, err := parseIntBaseStrictBitSize(rest, 16, 64)
			if err != nil {
				return 0, fmt.Errorf("Failed to parse '%s' as hexadecimal: %s", s, err.Error())
			}
			return v, nil
		}
		if hasBinaryPrefix(s) {
			rest := s[2:]
			v, err := parseIntBaseStrictBitSize(rest, 2, 64)
			if err != nil {
				return 0, fmt.Errorf("Failed to parse '%s' as binary: %s", s, err.Error())
			}
			return v, nil
		}
		if strings.HasPrefix(s, "0") {
			v, err := parseIntBaseStrictBitSize(s, 8, 64)
			if err != nil {
				return 0, fmt.Errorf("Failed to parse '%s' as octal: %s", s, err.Error())
			}
			return v, nil
		}
		v, err := parseIntDecStrictBitSize(s, 64)
		if err != nil {
			return 0, fmt.Errorf("Failed to parse '%s' as decimal integer: %s", s, err.Error())
		}
		return v, nil
	default:
		return 0, fmt.Errorf("No scan specification")
	}
}

func parseUintShape(s string, shape byte) (uint64, error) {
	switch shape {
	case 'u':
		v, err := parseUintDecStrictBitSize(s, 64)
		if err != nil {
			return 0, err
		}
		return v, nil
	case 'b':
		if !hasBinaryPrefix(s) {
			return 0, fmt.Errorf("pattern not found")
		}
		rest := s[2:]
		v, err := parseUintBaseStrictBitSize(rest, 2, 64)
		if err != nil {
			return 0, err
		}
		return v, nil
	case 'o':
		v, err := parseUintBaseStrictBitSize(s, 8, 64)
		if err != nil {
			return 0, err
		}
		return v, nil
	case 'x', 'X':
		raw := s
		if hasHexPrefix(s) {
			rest := s[2:]
			v, err := parseUintBaseStrictBitSize(rest, 16, 64)
			if err != nil {
				return 0, fmt.Errorf("Failed to parse '%s' as hexadecimal: %s", raw, err.Error())
			}
			return v, nil
		}
		v, err := parseUintBaseStrictBitSize(s, 16, 64)
		if err != nil {
			return 0, fmt.Errorf("Failed to parse '%s' as hexadecimal: %s", raw, err.Error())
		}
		return v, nil
	default:
		return 0, fmt.Errorf("No scan specification")
	}
}

func parseFloatShape(s string, shape byte) (float64, error) {
	switch shape {
	case 'g', 'G':
		return parseFloatGeneral(s, 64)
	case 'e', 'E':
		if hasHexPrefix(s) {
			return 0, fmt.Errorf("chars_format::scientific does not parse hexfloat")
		}
		if hasBinaryPrefix(s) {
			return 0, fmt.Errorf("chars_format::scientific does not parse binfloat")
		}
		if !strings.ContainsAny(s, "eE") {
			return 0, fmt.Errorf("chars_format::scientific requires exponent part")
		}
		v, err := doStrtodStrict(s, 64)
		if err != nil {
			return 0, fmt.Errorf("Failed to parse '%s' as scientific notation: %s", s, err.Error())
		}
		return v, nil
	case 'f', 'F':
		if hasHexPrefix(s) {
			return 0, fmt.Errorf("chars_format::fixed does not parse hexfloat")
		}
		if hasBinaryPrefix(s) {
			return 0, fmt.Errorf("chars_format::fixed does not parse binfloat")
		}
		if strings.ContainsAny(s, "eE") {
			return 0, fmt.Errorf("chars_format::fixed does not parse exponent part")
		}
		v, err := doStrtodStrict(s, 64)
		if err != nil {
			return 0, fmt.Errorf("Failed to parse '%s' as fixed notation: %s", s, err.Error())
		}
		return v, nil
	case 'a', 'A':
		if !hasHexPrefix(s) {
			return 0, fmt.Errorf("chars_format::hex parses hexfloat")
		}
		if hasBinaryPrefix(s) {
			return 0, fmt.Errorf("chars_format::hex does not parse binfloat")
		}
		v, err := doStrtodStrict(s, 64)
		if err != nil {
			return 0, fmt.Errorf("Failed to parse '%s' as hexadecimal: %s", s, err.Error())
		}
		return v, nil
	default:
		return 0, fmt.Errorf("No scan specification")
	}
}

func assignDefault(dest any, def any) error {
	rv := reflect.ValueOf(dest)
	if rv.Kind() != reflect.Pointer || rv.IsNil() {
		return errors.New("dest must be a non-nil pointer")
	}
	dv := reflect.ValueOf(def)
	if dv.IsValid() {
		if dv.Type().AssignableTo(rv.Elem().Type()) {
			rv.Elem().Set(dv)
			return nil
		}
		if dv.Type().ConvertibleTo(rv.Elem().Type()) {
			rv.Elem().Set(dv.Convert(rv.Elem().Type()))
			return nil
		}
	}
	return nil
}

func assignString(dest any, s string) error {
	rv := reflect.ValueOf(dest)
	if rv.Kind() != reflect.Pointer || rv.IsNil() {
		return errors.New("dest must be a non-nil pointer")
	}
	switch rv.Elem().Kind() {
	case reflect.String:
		rv.Elem().SetString(s)
		return nil
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		bitSize := int(rv.Elem().Type().Bits())
		n, err := parseIntDecStrictBitSize(s, bitSize)
		if err != nil {
			return err
		}
		rv.Elem().SetInt(n)
		return nil
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		bitSize := int(rv.Elem().Type().Bits())
		n, err := parseUintDecStrictBitSize(s, bitSize)
		if err != nil {
			return err
		}
		rv.Elem().SetUint(n)
		return nil
	case reflect.Float32, reflect.Float64:
		bitSize := int(rv.Elem().Type().Bits())
		f, err := parseFloatGeneral(s, bitSize)
		if err != nil {
			return err
		}
		rv.Elem().SetFloat(f)
		return nil
	case reflect.Bool:
		b, err := strconv.ParseBool(s)
		if err != nil {
			return err
		}
		rv.Elem().SetBool(b)
		return nil
	default:
		return fmt.Errorf("unsupported destination type: %s", rv.Elem().Kind())
	}
}

func (a *Argument) initDefaults() {
	if a.defaultAction.valued == nil && a.defaultAction.voided == nil {
		a.defaultAction = actionVariant{valued: func(s string) (any, error) { return s, nil }}
	}
}

func (a *Argument) validate() error {
	if a.isOptional {
		if !a.isUsed && a.defaultValue == nil && a.isRequired {
			return fmt.Errorf("%s: required.", a.names[0])
		}
		if a.isUsed && a.isRequired && len(a.values) == 0 {
			return fmt.Errorf("%s: no value provided.", a.usedName)
		}
	} else {
		if !a.numArgs.contains(uint64(len(a.values))) && a.defaultValue == nil {
			used := a.usedName
			if used == "" {
				used = a.names[0]
			}
			expected := ""
			if a.numArgs.isExact() {
				expected = fmt.Sprintf("%d", a.numArgs.min)
			} else if a.numArgs.isRightBounded() {
				expected = fmt.Sprintf("%d to %d", a.numArgs.min, a.numArgs.max)
			} else {
				expected = fmt.Sprintf("%d or more", a.numArgs.min)
			}
			return fmt.Errorf("%s: %s argument(s) expected. %d provided.", used, expected, len(a.values))
		}
	}
	if len(a.choices) > 0 && a.defaultValue != nil {
		if a.defaultValueStr != nil {
			ok := false
			for _, c := range a.choices {
				if c == *a.defaultValueStr {
					ok = true
					break
				}
			}
			if !ok {
				return fmt.Errorf("Invalid default value %s - allowed options: {%s}", a.defaultValueRepr, strings.Join(a.choices, ", "))
			}
		}
	}
	return nil
}

func (a *Argument) isValueInChoices(v string) bool {
	for _, c := range a.choices {
		if c == v {
			return true
		}
	}
	return false
}

func (a *Argument) consume(tokens []string, start, end int, usedName string, dryRun bool) (int, error) {
	if !a.isRepeatable && a.isUsed {
		return start, fmt.Errorf("Duplicate argument %s", usedName)
	}
	a.usedName = usedName

	// Pre-count choices passed options.
	passed := uint64(0)
	if len(a.choices) > 0 {
		maxN := a.numArgs.max
		minN := a.numArgs.min
		for i := start; i < end; i++ {
			if a.isValueInChoices(tokens[i]) {
				passed++
				continue
			}
			if passed >= minN && passed <= maxN {
				break
			}
			return start, fmt.Errorf("Invalid argument %q - allowed options: {%s}", tokens[i], strings.Join(a.choices, ", "))
		}
	}

	numArgsMaxU := a.numArgs.max
	if len(a.choices) > 0 {
		numArgsMaxU = passed
	}
	numArgsMinU := a.numArgs.min

	if numArgsMaxU == 0 {
		if !dryRun {
			a.values = append(a.values, a.implicitValue)
			a.initDefaults()
			if len(a.actions) > 0 {
				for _, act := range a.actions {
					if act.valued != nil {
						_, _ = act.valued("")
					} else if act.voided != nil {
						_ = act.voided("")
					}
				}
			} else {
				if a.defaultAction.valued != nil {
					_, _ = a.defaultAction.valued("")
				} else if a.defaultAction.voided != nil {
					_ = a.defaultAction.voided("")
				}
			}
			a.isUsed = true
		}
		return start, nil
	}

	distU := uint64(end - start)
	if distU >= numArgsMinU {
		newEnd := end
		// Only cap end if max is bounded and smaller than available.
		if numArgsMaxU != math.MaxUint64 && numArgsMaxU < distU {
			newEnd = start + int(numArgsMaxU)
		}
		if !a.acceptsOptionalLikeValue {
			// stop at next optional-like
			for i := start; i < newEnd; i++ {
				if !isPositional(tokens[i], a.prefixChars) {
					newEnd = i
					break
				}
			}
			if uint64(newEnd-start) < numArgsMinU {
				return start, fmt.Errorf("Too few arguments for '%s'.", usedName)
			}
		}

		if !dryRun {
			a.initDefaults()
			if len(a.actions) > 0 {
				for _, act := range a.actions {
					if act.valued != nil {
						for i := start; i < newEnd; i++ {
							v, err := act.valued(tokens[i])
							if err != nil {
								return start, err
							}
							a.values = append(a.values, v)
						}
					} else if act.voided != nil {
						for i := start; i < newEnd; i++ {
							if err := act.voided(tokens[i]); err != nil {
								return start, err
							}
						}
						if a.defaultValue == nil && !a.acceptsOptionalLikeValue {
							// keep size aligned
							for len(a.values) < (newEnd - start) {
								a.values = append(a.values, nil)
							}
						}
					}
				}
			} else {
				if a.defaultAction.valued != nil {
					for i := start; i < newEnd; i++ {
						v, err := a.defaultAction.valued(tokens[i])
						if err != nil {
							return start, err
						}
						a.values = append(a.values, v)
					}
				} else if a.defaultAction.voided != nil {
					for i := start; i < newEnd; i++ {
						if err := a.defaultAction.voided(tokens[i]); err != nil {
							return start, err
						}
					}
				}
			}
			a.isUsed = true
		}
		return newEnd, nil
	}

	if a.defaultValue != nil {
		if !dryRun {
			a.isUsed = true
		}
		return start, nil
	}
	return start, fmt.Errorf("Too few arguments for '%s'.", usedName)
}

// MutuallyExclusiveGroup matches argparse.hpp group semantics.
type MutuallyExclusiveGroup struct {
	parent   *ArgumentParser
	required bool
	elements []*Argument
}

func (g *MutuallyExclusiveGroup) AddArgument(names ...string) *Argument {
	arg := g.parent.AddArgument(names...)
	g.elements = append(g.elements, arg)
	arg.usageNewlineCounter = g.parent.usageNewlineCounter
	arg.groupIdx = uint64(len(g.parent.groupNames))
	return arg
}

// ArgumentParser is the main struct for parsing command line arguments.
type ArgumentParser struct {
	name        string
	version     string
	description string
	epilog      string

	exitOnDefaultArguments bool

	prefixChars string
	assignChars string

	isParsed bool

	positional []*Argument
	optional   []*Argument
	argMap     map[string]*Argument

	parserPath string

	subparsers    []*ArgumentParser
	subparserMap  map[string]*ArgumentParser
	subparserUsed map[string]bool
	suppress      bool

	mutexGroups       []*MutuallyExclusiveGroup
	usageMaxLineWidth uint64
	usageBreakOnMutex bool

	usageNewlineCounter int
	groupNames          []string
}

// NewArgumentParser creates a new ArgumentParser (compatible ctor).
func NewArgumentParser(name string, version ...string) *ArgumentParser {
	ver := ""
	if len(version) > 0 {
		ver = version[0]
	}
	p := &ArgumentParser{
		name:                   name,
		version:                ver,
		exitOnDefaultArguments: true,
		prefixChars:            "-",
		assignChars:            "=",
		positional:             make([]*Argument, 0),
		optional:               make([]*Argument, 0),
		argMap:                 make(map[string]*Argument),
		subparsers:             make([]*ArgumentParser, 0),
		subparserMap:           make(map[string]*ArgumentParser),
		subparserUsed:          make(map[string]bool),
		mutexGroups:            make([]*MutuallyExclusiveGroup, 0),
		groupNames:             make([]string, 0),
		usageMaxLineWidth:      math.MaxUint64,
		usageBreakOnMutex:      false,
	}

	// Add default help/version arguments like argparse.hpp.
	p.AddArgument("-h", "--help").
		ActionVoid(func(_ string) error {
			fmt.Fprint(os.Stdout, p.FormatHelp())
			if p.exitOnDefaultArguments {
				os.Exit(0)
			}
			return nil
		}).
		DefaultValue(false).
		Help("shows help message and exits").
		ImplicitValue(true).
		Nargs(0)

	if p.version != "" {
		p.AddArgument("-v", "--version").
			ActionVoid(func(_ string) error {
				fmt.Fprintln(os.Stdout, p.version)
				if p.exitOnDefaultArguments {
					os.Exit(0)
				}
				return nil
			}).
			DefaultValue(false).
			Help("prints version information and exits").
			ImplicitValue(true).
			Nargs(0)
	}

	return p
}

func (p *ArgumentParser) AddDescription(desc string) { p.description = desc }
func (p *ArgumentParser) AddEpilog(epilog string)    { p.epilog = epilog }

func (p *ArgumentParser) SetPrefixChars(prefix string) *ArgumentParser {
	p.prefixChars = prefix
	// argparse.hpp stores prefix chars as a view referenced by Arguments;
	// in Go we propagate it to existing Arguments to keep consume() semantics aligned.
	for _, a := range p.positional {
		a.prefixChars = prefix
	}
	for _, a := range p.optional {
		a.prefixChars = prefix
	}
	return p
}

func (p *ArgumentParser) SetAssignChars(assign string) *ArgumentParser {
	p.assignChars = assign
	return p
}

// SetUsageMaxLineWidth sets the maximum usage line width for wrapping.
// A width of 0 disables wrapping (single-line usage), matching argparse.hpp.
func (p *ArgumentParser) SetUsageMaxLineWidth(w uint64) *ArgumentParser {
	if w == 0 {
		p.usageMaxLineWidth = math.MaxUint64
		return p
	}
	p.usageMaxLineWidth = w
	return p
}

// SetUsageBreakOnMutex forces a line break before/after mutex groups when wrapping.
func (p *ArgumentParser) SetUsageBreakOnMutex(v bool) *ArgumentParser {
	p.usageBreakOnMutex = v
	return p
}

func (p *ArgumentParser) AddGroup(name string) *ArgumentParser {
	p.groupNames = append(p.groupNames, name)
	return p
}

func (p *ArgumentParser) AddUsageNewline() *ArgumentParser {
	p.usageNewlineCounter++
	return p
}

func (p *ArgumentParser) AddMutuallyExclusiveGroup(required bool) *MutuallyExclusiveGroup {
	g := &MutuallyExclusiveGroup{parent: p, required: required, elements: []*Argument{}}
	p.mutexGroups = append(p.mutexGroups, g)
	return g
}

// AddArgument adds an argument to the parser.
func (p *ArgumentParser) AddArgument(names ...string) *Argument {
	a := &Argument{
		names:       append([]string{}, names...),
		prefixChars: p.prefixChars,
		numArgs:     newNArgsRange(1, 1),
	}
	// argparse.hpp sorts names by length, then lexicographically.
	sort.Slice(a.names, func(i, j int) bool {
		li, lj := len(a.names[i]), len(a.names[j])
		if li == lj {
			return a.names[i] < a.names[j]
		}
		return li < lj
	})
	// Determine optional/positional based on first name.
	a.isOptional = !isPositional(a.names[0], p.prefixChars)
	if a.isOptional {
		p.optional = append(p.optional, a)
	} else {
		p.positional = append(p.positional, a)
	}
	a.usageNewlineCounter = p.usageNewlineCounter
	a.groupIdx = uint64(len(p.groupNames))
	for _, n := range a.names {
		p.argMap[n] = a
	}
	return a
}

func (p *ArgumentParser) AddSubparser(sub *ArgumentParser) {
	sub.parserPath = strings.TrimSpace(p.name + " " + sub.name)
	p.subparsers = append(p.subparsers, sub)
	p.subparserMap[sub.name] = sub
	p.subparserUsed[sub.name] = false
}

// IsSubcommandUsed checks if a subcommand was used.
func (p *ArgumentParser) IsSubcommandUsed(name string) bool {
	return p.subparserUsed[name]
}

func (p *ArgumentParser) IsUsed(argName string) bool {
	a, err := p.At(argName)
	if err != nil {
		return false
	}
	return a.isUsed
}

func (p *ArgumentParser) At(name string) (*Argument, error) {
	if a, ok := p.argMap[name]; ok {
		return a, nil
	}
	// Try prefix fix-up like argparse.hpp operator[]
	if len(name) > 0 && !p.isValidPrefixChar(name[0]) {
		prefix := string(p.prefixChars[0])
		if a, ok := p.argMap[prefix+name]; ok {
			return a, nil
		}
		if a, ok := p.argMap[prefix+prefix+name]; ok {
			return a, nil
		}
	}
	return nil, fmt.Errorf("No such argument: %s", name)
}

// Get returns the parsed (or default) value for an argument.
// This is the Go equivalent of argparse.hpp get<T>(), but returns `any`.
func (p *ArgumentParser) Get(name string) (any, error) {
	if !p.isParsed {
		return nil, fmt.Errorf("Nothing parsed, no arguments are available.")
	}
	a, err := p.At(name)
	if err != nil {
		return nil, err
	}
	return a.Get()
}

// GetInto writes the parsed (or default) value into dest.
// For slice destinations, it fills the slice from all consumed values.
func (p *ArgumentParser) GetInto(name string, dest any) error {
	if !p.isParsed {
		return fmt.Errorf("Nothing parsed, no arguments are available.")
	}
	a, err := p.At(name)
	if err != nil {
		return err
	}
	return a.GetInto(dest)
}

// Present returns (value,true) if user supplied values; (nil,false) otherwise.
// It errors if the argument has a default value.
func (p *ArgumentParser) Present(name string) (any, bool, error) {
	a, err := p.At(name)
	if err != nil {
		return nil, false, err
	}
	return a.Present()
}

// PresentInto writes the parsed value into dest and returns true if present.
// It errors if the argument has a default value.
func (p *ArgumentParser) PresentInto(name string, dest any) (bool, error) {
	a, err := p.At(name)
	if err != nil {
		return false, err
	}
	return a.PresentInto(dest)
}

func (p *ArgumentParser) isValidPrefixChar(c byte) bool {
	return strings.ContainsRune(p.prefixChars, rune(c))
}

// ParseArgs parses args; aligned to argparse.hpp: args[0] is program name.
func (p *ArgumentParser) ParseArgs(args []string) error {
	_, err := p.parseInternal(args, false)
	if err != nil {
		return err
	}
	// Validate all arguments
	for _, a := range p.argMap {
		if err := a.validate(); err != nil {
			return err
		}
	}
	// Validate mutex groups
	for _, g := range p.mutexGroups {
		used := false
		var usedArg *Argument
		for _, a := range g.elements {
			if !used && a.isUsed {
				used = true
				usedArg = a
			} else if used && a.isUsed {
				return fmt.Errorf("Argument '%s' not allowed with '%s'", a.getUsageFull(), usedArg.getUsageFull())
			}
		}
		if !used && g.required {
			parts := make([]string, 0, len(g.elements))
			for _, a := range g.elements {
				parts = append(parts, fmt.Sprintf("'%s'", a.getUsageFull()))
			}
			if len(parts) == 0 {
				continue
			}
			if len(parts) == 1 {
				return fmt.Errorf("One of the arguments %s is required", parts[0])
			}
			return fmt.Errorf("One of the arguments %s is required", strings.Join(parts[:len(parts)-1], " or ")+" or "+parts[len(parts)-1])
		}
	}
	p.isParsed = true
	return nil
}

// ParseArgsOS parses the current process arguments from os.Args.
// This is the Go equivalent of calling parse_args(argc, argv) in C++ main().
func (p *ArgumentParser) ParseArgsOS() error {
	return p.ParseArgs(os.Args)
}

// ParseKnownArgs parses known args and returns unknown args.
func (p *ArgumentParser) ParseKnownArgs(args []string) ([]string, error) {
	unknown, err := p.parseInternal(args, true)
	if err != nil {
		return nil, err
	}
	for _, a := range p.argMap {
		if err := a.validate(); err != nil {
			return nil, err
		}
	}
	p.isParsed = true
	return unknown, nil
}

// ParseKnownArgsOS parses os.Args and returns unknown arguments.
func (p *ArgumentParser) ParseKnownArgsOS() ([]string, error) {
	return p.ParseKnownArgs(os.Args)
}

func (p *ArgumentParser) preprocessArguments(raw []string) []string {
	args := make([]string, 0, len(raw))
	for _, arg := range raw {
		argumentStartsWithPrefixChars := func(a string) bool {
			if a == "" {
				return false
			}
			legalPrefix := func(c byte) bool { return p.isValidPrefixChar(c) }
			windowsStyle := legalPrefix('/')
			if windowsStyle {
				return legalPrefix(a[0])
			}
			if len(a) > 1 {
				return legalPrefix(a[0]) && legalPrefix(a[1])
			}
			return false
		}

		assignPos := strings.IndexAny(arg, p.assignChars)
		if _, ok := p.argMap[arg]; !ok && argumentStartsWithPrefixChars(arg) && assignPos != -1 {
			optName := arg[:assignPos]
			if _, ok2 := p.argMap[optName]; ok2 {
				args = append(args, optName)
				args = append(args, arg[assignPos+1:])
				continue
			}
		}
		args = append(args, arg)
	}
	return args
}

func (p *ArgumentParser) parseInternal(raw []string, known bool) ([]string, error) {
	arguments := p.preprocessArguments(raw)
	if p.name == "" && len(arguments) > 0 {
		p.name = arguments[0]
	}
	if len(arguments) == 0 {
		return nil, nil
	}
	unknown := []string{}
	end := len(arguments)
	posIdx := 0
	for i := 1; i < end; {
		cur := arguments[i]
		if isPositional(cur, p.prefixChars) {
			if posIdx >= len(p.positional) {
				// subparsers
				if sub, ok := p.subparserMap[cur]; ok {
					p.isParsed = true
					p.subparserUsed[cur] = true
					// Remaining args list starts at subcommand token.
					if known {
						return sub.parseInternal(arguments[i:], true)
					}
					_, err := sub.parseInternal(arguments[i:], false)
					return nil, err
				}
				if known {
					unknown = append(unknown, cur)
					i++
					continue
				}
				if len(p.positional) == 0 {
					// Ask if user meant a subcommand.
					if len(p.subparserMap) > 0 {
						keys := make([]string, 0, len(p.subparserMap))
						for k := range p.subparserMap {
							keys = append(keys, k)
						}
						sort.Strings(keys)
						most := mostSimilarString(keys, cur)
						return nil, fmt.Errorf("Failed to parse '%s', did you mean '%s'", cur, most)
					}

					// Ask if user meant a value-taking optional argument.
					if len(p.optional) > 0 {
						for _, opt := range p.optional {
							if opt == nil {
								continue
							}
							// Not a flag (requires a value) and not used.
							if opt.implicitValue == nil && !opt.isUsed {
								return nil, fmt.Errorf("Zero positional arguments expected, did you mean %s", opt.getUsageFull())
							}
						}
						return nil, fmt.Errorf("Zero positional arguments expected")
					}

					return nil, fmt.Errorf("Zero positional arguments expected")
				}
				return nil, fmt.Errorf("Maximum number of positional arguments exceeded, failed to parse '%s'", cur)
			}

			arg := p.positional[posIdx]
			posIdx++

			// Special case: <pos1>... <pos2>
			if arg.numArgs.min == 1 && arg.numArgs.max == math.MaxUint64 &&
				posIdx < len(p.positional) && posIdx == len(p.positional)-1 {
				nextArg := p.positional[posIdx]
				if nextArg.numArgs.min == 1 && nextArg.numArgs.max == 1 {
					if i+1 < end {
						// consume last token for nextArg
						_, err := nextArg.consume(arguments, end-1, end, "", false)
						if err != nil {
							return nil, err
						}
						end = end - 1
					} else {
						return nil, fmt.Errorf("Missing %s", nextArg.names[0])
					}
				}
			}
			newI, err := arg.consume(arguments, i, end, "", false)
			if err != nil {
				return nil, err
			}
			i = newI
			continue
		}

		if a, ok := p.argMap[cur]; ok {
			newI, err := a.consume(arguments, i+1, end, cur, false)
			if err != nil {
				return nil, err
			}
			i = newI
			continue
		}

		// compound short options -abc
		if len(cur) > 1 && p.isValidPrefixChar(cur[0]) && !p.isValidPrefixChar(cur[1]) {
			i++
			okAll := true
			for j := 1; j < len(cur); j++ {
				hypothetical := "-" + string(cur[j])
				if a2, ok2 := p.argMap[hypothetical]; ok2 {
					newI, err := a2.consume(arguments, i, end, hypothetical, false)
					if err != nil {
						return nil, err
					}
					i = newI
				} else {
					okAll = false
					if known {
						unknown = append(unknown, cur)
						break
					}
					return nil, fmt.Errorf("Unknown argument: %s", cur)
				}
			}
			if okAll {
				continue
			}
			continue
		}

		if known {
			unknown = append(unknown, cur)
			i++
			continue
		}
		return nil, fmt.Errorf("Unknown argument: %s", cur)
	}
	p.isParsed = true
	return unknown, nil
}

func levenshteinDistance(s1, s2 string) int {
	// DP implementation aligned to argparse.hpp details::get_levenshtein_distance.
	// Uses bytes (ASCII-ish CLI tokens) like the C++ char-wise algorithm.
	b1 := []byte(s1)
	b2 := []byte(s2)
	rows := len(b1) + 1
	cols := len(b2) + 1
	dp := make([][]int, rows)
	for i := 0; i < rows; i++ {
		dp[i] = make([]int, cols)
	}
	for i := 0; i < rows; i++ {
		dp[i][0] = i
	}
	for j := 0; j < cols; j++ {
		dp[0][j] = j
	}
	for i := 1; i < rows; i++ {
		for j := 1; j < cols; j++ {
			if b1[i-1] == b2[j-1] {
				dp[i][j] = dp[i-1][j-1]
				continue
			}
			a := dp[i-1][j]
			b := dp[i][j-1]
			c := dp[i-1][j-1]
			m := a
			if b < m {
				m = b
			}
			if c < m {
				m = c
			}
			dp[i][j] = 1 + m
		}
	}
	return dp[len(b1)][len(b2)]
}

func mostSimilarString(sortedCandidates []string, input string) string {
	most := ""
	minDist := int(^uint(0) >> 1)
	for _, cand := range sortedCandidates {
		d := levenshteinDistance(cand, input)
		if d < minDist {
			minDist = d
			most = cand
		}
	}
	return most
}

func (p *ArgumentParser) parserNameForUsage() string {
	if strings.TrimSpace(p.parserPath) != "" {
		return p.parserPath
	}
	return p.name
}

func (p *ArgumentParser) subcommandNamesSorted() []string {
	keys := make([]string, 0, len(p.subparserMap))
	for k, sp := range p.subparserMap {
		if sp == nil || sp.suppress {
			continue
		}
		keys = append(keys, k)
	}
	sort.Strings(keys)
	return keys
}

func (p *ArgumentParser) hasVisiblePositional() bool {
	for _, a := range p.positional {
		if a != nil && !a.isHidden {
			return true
		}
	}
	return false
}

func (p *ArgumentParser) hasVisibleOptional(groupIdx uint64) bool {
	for _, a := range p.optional {
		if a == nil || a.isHidden {
			continue
		}
		if a.groupIdx == groupIdx {
			return true
		}
	}
	return false
}

func (p *ArgumentParser) getBelongingMutex(a *Argument) *MutuallyExclusiveGroup {
	if a == nil {
		return nil
	}
	for _, g := range p.mutexGroups {
		for _, e := range g.elements {
			if e == a {
				return g
			}
		}
	}
	return nil
}

func padRight(s string, width int) string {
	if width <= 0 {
		return s
	}
	if len(s) >= width {
		return s
	}
	return s + strings.Repeat(" ", width-len(s))
}

func (p *ArgumentParser) longestArgumentWidth() int {
	maxLen := 0
	for _, a := range p.positional {
		if a == nil {
			continue
		}
		if l := a.getArgumentsLength(); l > maxLen {
			maxLen = l
		}
	}
	for _, a := range p.optional {
		if a == nil {
			continue
		}
		if l := a.getArgumentsLength(); l > maxLen {
			maxLen = l
		}
	}
	for k := range p.subparserMap {
		if len(k) > maxLen {
			maxLen = len(k)
		}
	}
	return maxLen
}

// Usage renders the usage string in a style aligned to argparse.hpp.
func (p *ArgumentParser) Usage() string {
	var stream strings.Builder

	curline := "Usage: "
	curline += strings.TrimSpace(p.parserNameForUsage())
	if strings.TrimSpace(curline) == "Usage:" {
		curline = "Usage: " + strings.TrimSpace(p.name)
	}

	indentSize := len(curline)
	multilineUsage := p.usageMaxLineWidth < math.MaxUint64

	dealWithOptionsOfGroup := func(groupIdx uint64) bool {
		foundOptions := false
		var curMutex *MutuallyExclusiveGroup
		usageNewlineCounter := -1
		for _, argument := range p.optional {
			if argument == nil || argument.isHidden {
				continue
			}
			if multilineUsage {
				if argument.groupIdx != groupIdx {
					continue
				}
				if usageNewlineCounter != int(argument.usageNewlineCounter) {
					if usageNewlineCounter >= 0 {
						if len(curline) > indentSize {
							stream.WriteString(curline)
							stream.WriteString("\n")
							curline = strings.Repeat(" ", indentSize)
						}
					}
					usageNewlineCounter = int(argument.usageNewlineCounter)
				}
			}

			foundOptions = true
			argInlineUsage := argument.inlineUsage()
			argMutex := p.getBelongingMutex(argument)

			if curMutex != nil && argMutex == nil {
				curline += "]"
				if p.usageBreakOnMutex {
					stream.WriteString(curline)
					stream.WriteString("\n")
					curline = strings.Repeat(" ", indentSize)
				}
			} else if curMutex == nil && argMutex != nil {
				if (p.usageBreakOnMutex && len(curline) > indentSize) ||
					(uint64(len(curline))+3+uint64(len(argInlineUsage)) > p.usageMaxLineWidth) {
					stream.WriteString(curline)
					stream.WriteString("\n")
					curline = strings.Repeat(" ", indentSize)
				}
				curline += " ["
			} else if curMutex != nil && argMutex != nil {
				if curMutex != argMutex {
					curline += "]"
					if p.usageBreakOnMutex ||
						(uint64(len(curline))+3+uint64(len(argInlineUsage)) > p.usageMaxLineWidth) {
						stream.WriteString(curline)
						stream.WriteString("\n")
						curline = strings.Repeat(" ", indentSize)
					}
					curline += " ["
				} else {
					curline += "|"
				}
			}

			curMutex = argMutex
			if len(curline) != indentSize &&
				(uint64(len(curline))+1+uint64(len(argInlineUsage)) > p.usageMaxLineWidth) {
				stream.WriteString(curline)
				stream.WriteString("\n")
				curline = strings.Repeat(" ", indentSize)
				curline += " "
			} else if curMutex == nil {
				curline += " "
			}
			curline += argInlineUsage
		}
		if curMutex != nil {
			curline += "]"
		}
		return foundOptions
	}

	foundOptions := dealWithOptionsOfGroup(0)

	if foundOptions && multilineUsage && len(p.positional) > 0 {
		stream.WriteString(curline)
		stream.WriteString("\n")
		curline = strings.Repeat(" ", indentSize)
	}

	for _, argument := range p.positional {
		if argument == nil || argument.isHidden {
			continue
		}
		posArg := argument.names[0]
		if strings.TrimSpace(argument.metavar) != "" {
			posArg = argument.metavar
		}
		if uint64(len(curline))+1+uint64(len(posArg)) > p.usageMaxLineWidth {
			stream.WriteString(curline)
			stream.WriteString("\n")
			curline = strings.Repeat(" ", indentSize)
		}
		curline += " "
		if argument.numArgs.min == 0 && !argument.numArgs.isRightBounded() {
			curline += "[" + posArg + "]..."
		} else if argument.numArgs.min == 1 && !argument.numArgs.isRightBounded() {
			curline += posArg + "..."
		} else {
			curline += posArg
		}
	}

	if multilineUsage {
		for i := 0; i < len(p.groupNames); i++ {
			stream.WriteString(curline)
			stream.WriteString("\n\n")
			stream.WriteString(p.groupNames[i])
			stream.WriteString(":\n")
			curline = strings.Repeat(" ", indentSize)
			dealWithOptionsOfGroup(uint64(i + 1))
		}
	}

	stream.WriteString(curline)

	if len(p.subparserMap) > 0 {
		subs := p.subcommandNamesSorted()
		stream.WriteString(" {")
		for i, s := range subs {
			if i == 0 {
				stream.WriteString(s)
			} else {
				stream.WriteString(",")
				stream.WriteString(s)
			}
		}
		stream.WriteString("}")
	}

	return stream.String()
}

// FormatHelp prints help text similarly to argparse.hpp operator<<.
func (p *ArgumentParser) FormatHelp() string {
	var sb strings.Builder
	width := p.longestArgumentWidth()
	hasVisiblePositional := p.hasVisiblePositional()

	sb.WriteString(p.Usage())
	sb.WriteString("\n\n")
	if p.description != "" {
		sb.WriteString(p.description)
		sb.WriteString("\n\n")
	}

	if hasVisiblePositional {
		sb.WriteString("Positional arguments:\n")
	}
	for _, a := range p.positional {
		if a == nil || a.isHidden {
			continue
		}
		sb.WriteString(a.renderForHelp(width))
	}

	if len(p.optional) > 0 {
		if hasVisiblePositional {
			sb.WriteString("\n")
		}
		sb.WriteString("Optional arguments:\n")
		for _, a := range p.optional {
			if a == nil || a.isHidden || a.groupIdx != 0 {
				continue
			}
			sb.WriteString(a.renderForHelp(width))
		}
	}

	for i, groupName := range p.groupNames {
		idx := uint64(i + 1)
		sb.WriteString("\n")
		sb.WriteString(groupName)
		sb.WriteString(" (detailed usage):\n")
		for _, a := range p.optional {
			if a == nil || a.isHidden || a.groupIdx != idx {
				continue
			}
			sb.WriteString(a.renderForHelp(width))
		}
	}

	subs := p.subcommandNamesSorted()
	if len(subs) > 0 {
		if len(p.positional) > 0 {
			sb.WriteString("\n")
		} else if len(p.optional) > 0 {
			sb.WriteString("\n")
		}
		sb.WriteString("Subcommands:\n")
		for _, k := range subs {
			sp := p.subparserMap[k]
			help := ""
			if sp != nil {
				help = sp.description
			}
			colw := width - 2
			if colw < 0 {
				colw = 0
			}
			sb.WriteString("  " + padRight(k, colw) + " " + help + "\n")
		}
	}

	if p.epilog != "" {
		sb.WriteString("\n")
		sb.WriteString(p.epilog)
		sb.WriteString("\n\n")
	}
	return sb.String()
}

func (a *Argument) getUsageFull() string {
	names := strings.Join(a.names, "/")
	mv := a.metavar
	if mv == "" {
		mv = "VAR"
	}
	if a.numArgs.max > 0 {
		if a.numArgs.max > 1 {
			return fmt.Sprintf("%s %s...", names, mv)
		}
		return fmt.Sprintf("%s %s", names, mv)
	}
	return names
}

func (r nArgsRange) reprForHelp() string {
	if r.isExact() {
		if r.min == 0 || r.min == 1 {
			return ""
		}
		return fmt.Sprintf("[nargs: %d] ", r.min)
	}
	if !r.isRightBounded() {
		return fmt.Sprintf("[nargs: %d or more] ", r.min)
	}
	return fmt.Sprintf("[nargs=%d..%d] ", r.min, r.max)
}

func (a *Argument) metavarOrDefault() string {
	if strings.TrimSpace(a.metavar) != "" {
		return a.metavar
	}
	return "VAR"
}

func (a *Argument) longestName() string {
	if a == nil || len(a.names) == 0 {
		return ""
	}
	longest := a.names[0]
	for _, s := range a.names {
		if len(s) > len(longest) {
			longest = s
		}
	}
	return longest
}

func (a *Argument) shouldShowMetavarInHelpName() bool {
	if a == nil {
		return false
	}
	if strings.TrimSpace(a.metavar) == "" {
		return false
	}
	if a.numArgs.min == 1 && a.numArgs.max == 1 {
		return true
	}
	if a.numArgs.min == a.numArgs.max && strings.Contains(a.metavar, "> <") {
		return true
	}
	return false
}

func (a *Argument) helpNameStream() string {
	if a == nil {
		return ""
	}
	// Mirrors argparse.hpp Argument::operator<< name_stream formatting.
	var sb strings.Builder
	sb.WriteString("  ")
	if !a.isOptional {
		if strings.TrimSpace(a.metavar) != "" {
			sb.WriteString(a.metavar)
		} else {
			sb.WriteString(strings.Join(a.names, " "))
		}
		return sb.String()
	}
	sb.WriteString(strings.Join(a.names, ", "))
	if a.shouldShowMetavarInHelpName() {
		sb.WriteString(" ")
		sb.WriteString(a.metavar)
	}
	return sb.String()
}

func (a *Argument) getArgumentsLength() int {
	// Mirrors argparse.hpp Argument::get_arguments_length().
	if a == nil {
		return 0
	}
	namesSize := 0
	for _, n := range a.names {
		namesSize += len(n)
	}

	if !a.isOptional {
		if strings.TrimSpace(a.metavar) != "" {
			return 2 + len(a.metavar)
		}
		// indent + names size + spaces between names
		if len(a.names) == 0 {
			return 2
		}
		return 2 + namesSize + (len(a.names) - 1)
	}
	// option: names size + ", " separators
	sz := namesSize
	if len(a.names) > 1 {
		sz += 2 * (len(a.names) - 1)
	}
	if strings.TrimSpace(a.metavar) != "" && a.numArgs.min == 1 && a.numArgs.max == 1 {
		sz += 1 + len(a.metavar)
	}
	return sz + 2
}

// formatNameForHelp/usageMetavarSuffix were superseded by helpNameStream()
// and inlineUsage() for closer argparse.hpp parity.

func (a *Argument) inlineUsage() string {
	if a == nil {
		return ""
	}
	var sb strings.Builder
	if a.isOptional {
		if !a.isRequired {
			sb.WriteString("[")
		}
		sb.WriteString(a.longestName())
		metavar := a.metavarOrDefault()
		if a.numArgs.max > 0 {
			sb.WriteString(" ")
			sb.WriteString(metavar)
			if a.numArgs.max > 1 && !strings.Contains(a.metavar, "> <") {
				sb.WriteString("...")
			}
		}
		if !a.isRequired {
			sb.WriteString("]")
		}
	} else {
		// positional usage is handled in Usage() to exactly match argparse.hpp.
		posArg := a.names[0]
		if strings.TrimSpace(a.metavar) != "" {
			posArg = a.metavar
		}
		sb.WriteString(posArg)
	}
	usage := sb.String()
	if a.isRepeatable {
		usage += "..."
	}
	return usage
}

func (a *Argument) renderForHelp(width int) string {
	if a == nil {
		return ""
	}
	nameStream := a.helpNameStream()
	namePadding := strings.Repeat(" ", len(nameStream))
	hspace := "  "

	var sb strings.Builder
	// First line: padded name + hspace + help (possibly empty)
	sb.WriteString(padRight(nameStream, width))
	sb.WriteString(hspace)

	helpView := a.help
	firstLine := true
	prev := 0
	for {
		pos := strings.IndexByte(helpView[prev:], '\n')
		if pos == -1 {
			break
		}
		pos = prev + pos
		line := helpView[prev : pos+1]
		if firstLine {
			sb.WriteString(line)
			firstLine = false
		} else {
			sb.WriteString(padRight(namePadding, width))
			sb.WriteString(hspace)
			sb.WriteString(line)
		}
		prev = pos + 1
		if prev >= len(helpView) {
			break
		}
	}
	if firstLine {
		// No embedded newlines
		sb.WriteString(helpView)
	} else {
		leftover := ""
		if prev < len(helpView) {
			leftover = helpView[prev:]
		}
		if leftover != "" {
			sb.WriteString(padRight(namePadding, width))
			sb.WriteString(hspace)
			sb.WriteString(leftover)
		}
	}

	// print nargs spec
	if a.help != "" {
		sb.WriteString(" ")
	}
	sb.WriteString(a.numArgs.reprForHelp())

	addSpace := false
	if a.defaultValue != nil && !(a.numArgs.min == 0 && a.numArgs.max == 0) {
		sb.WriteString("[default: ")
		sb.WriteString(a.defaultValueRepr)
		sb.WriteString("]")
		addSpace = true
	} else if a.isRequired {
		sb.WriteString("[required]")
		addSpace = true
	}
	if a.isRepeatable {
		if addSpace {
			sb.WriteString(" ")
		}
		sb.WriteString("[may be repeated]")
	}
	sb.WriteString("\n")
	return sb.String()
}

// Get returns the stored value (or default) as `any`.
func (a *Argument) Get() (any, error) {
	if len(a.values) > 0 {
		return a.values[0], nil
	}
	if a.defaultValue != nil {
		return a.defaultValue, nil
	}
	return nil, fmt.Errorf("No value provided for '%s'.", a.names[len(a.names)-1])
}

// Values returns a copy of stored values.
func (a *Argument) Values() []any {
	out := make([]any, len(a.values))
	copy(out, a.values)
	return out
}

// GetInto writes the value(s) into dest. If dest is pointer-to-slice, fills slice.
func (a *Argument) GetInto(dest any) error {
	if dest == nil {
		return fmt.Errorf("dest must be non-nil")
	}
	rv := reflect.ValueOf(dest)
	if rv.Kind() != reflect.Pointer || rv.IsNil() {
		return fmt.Errorf("dest must be a non-nil pointer")
	}
	// slice destination: build from all values
	if rv.Elem().Kind() == reflect.Slice {
		sliceT := rv.Elem().Type()
		out := reflect.MakeSlice(sliceT, 0, len(a.values))
		for _, v := range a.values {
			if v == nil {
				continue
			}
			elem := reflect.New(sliceT.Elem()).Elem()
			if err := assignAny(elem, v); err != nil {
				return err
			}
			out = reflect.Append(out, elem)
		}
		rv.Elem().Set(out)
		return nil
	}

	// scalar: pick first stored, else default
	var v any
	if len(a.values) > 0 {
		v = a.values[0]
	} else {
		v = a.defaultValue
	}
	if v == nil {
		return fmt.Errorf("No value provided for '%s'.", a.names[len(a.names)-1])
	}
	return assignAny(rv.Elem(), v)
}

// Present returns (value,true) if present. Errors if default exists.
func (a *Argument) Present() (any, bool, error) {
	if a.defaultValue != nil {
		return nil, false, fmt.Errorf("Argument with default value always presents")
	}
	if len(a.values) == 0 {
		return nil, false, nil
	}
	return a.values[0], true, nil
}

// PresentInto writes to dest if present and returns true, errors if default exists.
func (a *Argument) PresentInto(dest any) (bool, error) {
	if a.defaultValue != nil {
		return false, fmt.Errorf("Argument with default value always presents")
	}
	if len(a.values) == 0 {
		return false, nil
	}
	return true, a.GetInto(dest)
}

func assignAny(dst reflect.Value, v any) error {
	vv := reflect.ValueOf(v)
	if !vv.IsValid() {
		return fmt.Errorf("bad cast")
	}
	if vv.Type().AssignableTo(dst.Type()) {
		dst.Set(vv)
		return nil
	}
	// Try parsing from string
	if vv.Kind() == reflect.String {
		parsed, err := parseStringToType(vv.String(), dst.Type())
		if err != nil {
			return err
		}
		dst.Set(parsed)
		return nil
	}
	if converted, ok, err := convertNumericWithRange(vv, dst.Type()); ok {
		if err != nil {
			return err
		}
		dst.Set(converted)
		return nil
	}
	if vv.Type().ConvertibleTo(dst.Type()) {
		dst.Set(vv.Convert(dst.Type()))
		return nil
	}
	return fmt.Errorf("bad cast")
}

func isIntKind(k reflect.Kind) bool {
	switch k {
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		return true
	default:
		return false
	}
}

func isUintKind(k reflect.Kind) bool {
	switch k {
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		return true
	default:
		return false
	}
}

func isFloatKind(k reflect.Kind) bool {
	switch k {
	case reflect.Float32, reflect.Float64:
		return true
	default:
		return false
	}
}

func convertNumericWithRange(v reflect.Value, dstType reflect.Type) (reflect.Value, bool, error) {
	srcKind := v.Kind()
	dstKind := dstType.Kind()

	if isIntKind(srcKind) && isIntKind(dstKind) {
		bits := int(dstType.Bits())
		val := v.Int()
		// bits is 0 only for non-int kinds; safe here.
		min := -(int64(1) << (bits - 1))
		max := (int64(1) << (bits - 1)) - 1
		if val < min || val > max {
			return reflect.Value{}, true, &strconv.NumError{Func: "convert", Num: fmt.Sprint(v.Interface()), Err: strconv.ErrRange}
		}
		out := reflect.New(dstType).Elem()
		out.SetInt(val)
		return out, true, nil
	}

	if isUintKind(srcKind) && isUintKind(dstKind) {
		bits := int(dstType.Bits())
		val := v.Uint()
		var max uint64
		if bits >= 64 {
			max = ^uint64(0)
		} else {
			max = (uint64(1) << bits) - 1
		}
		if val > max {
			return reflect.Value{}, true, &strconv.NumError{Func: "convert", Num: fmt.Sprint(v.Interface()), Err: strconv.ErrRange}
		}
		out := reflect.New(dstType).Elem()
		out.SetUint(val)
		return out, true, nil
	}

	if isFloatKind(srcKind) && isFloatKind(dstKind) {
		bits := int(dstType.Bits())
		val := v.Float()
		if bits == 32 {
			if !math.IsInf(val, 0) && math.Abs(val) > math.MaxFloat32 {
				return reflect.Value{}, true, &strconv.NumError{Func: "convert", Num: fmt.Sprint(v.Interface()), Err: strconv.ErrRange}
			}
		}
		out := reflect.New(dstType).Elem()
		out.SetFloat(val)
		return out, true, nil
	}

	// If both are numeric but in different families, avoid silent truncation.
	if (isIntKind(srcKind) || isUintKind(srcKind) || isFloatKind(srcKind)) && (isIntKind(dstKind) || isUintKind(dstKind) || isFloatKind(dstKind)) {
		return reflect.Value{}, true, fmt.Errorf("bad cast")
	}

	return reflect.Value{}, false, nil
}

func parseStringToType(s string, t reflect.Type) (reflect.Value, error) {
	switch t.Kind() {
	case reflect.String:
		return reflect.ValueOf(s).Convert(t), nil
	case reflect.Bool:
		b, err := strconv.ParseBool(s)
		if err != nil {
			return reflect.Value{}, err
		}
		return reflect.ValueOf(b).Convert(t), nil
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		bitSize := int(t.Bits())
		n, err := parseIntDecStrictBitSize(s, bitSize)
		if err != nil {
			return reflect.Value{}, err
		}
		v := reflect.New(t).Elem()
		v.SetInt(n)
		return v, nil
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64:
		bitSize := int(t.Bits())
		n, err := parseUintDecStrictBitSize(s, bitSize)
		if err != nil {
			return reflect.Value{}, err
		}
		v := reflect.New(t).Elem()
		v.SetUint(n)
		return v, nil
	case reflect.Float32, reflect.Float64:
		bitSize := int(t.Bits())
		f, err := parseFloatGeneral(s, bitSize)
		if err != nil {
			return reflect.Value{}, err
		}
		v := reflect.New(t).Elem()
		v.SetFloat(f)
		return v, nil
	default:
		return reflect.Value{}, fmt.Errorf("unsupported destination type: %s", t.Kind())
	}
}

// isPositional matches argparse.hpp Argument::is_positional.
func isPositional(name string, prefixChars string) bool {
	if name == "" {
		return true
	}
	first := name[0]
	if strings.ContainsRune(prefixChars, rune(first)) {
		rest := name[1:]
		if rest == "" {
			return true
		}
		return isDecimalLiteral(rest)
	}
	return true
}

func isDecimalLiteral(s string) bool {
	// Port of argparse.hpp decimal-literal recognizer.
	if s == "" {
		return false
	}
	lookahead := func(ss string) (byte, bool) {
		if ss == "" {
			return 0, false
		}
		return ss[0], true
	}
	isDigit := func(c byte) bool { return c >= '0' && c <= '9' }
	consumeDigits := func(sd string) string {
		i := 0
		for i < len(sd) && isDigit(sd[i]) {
			i++
		}
		return sd[i:]
	}

	c, ok := lookahead(s)
	if !ok {
		return true
	}
	switch c {
	case '0':
		s = s[1:]
		if s == "" {
			return true
		}
		s = consumeDigits(s)
		// integer_part_consumed
	case '1', '2', '3', '4', '5', '6', '7', '8', '9':
		s = consumeDigits(s)
		if s == "" {
			return true
		}
	case '.':
		s = s[1:]
		goto postDecimalPoint
	default:
		return false
	}

	// integer_part_consumed
	if s == "" {
		return false
	}
	c, ok = lookahead(s)
	if !ok {
		return true
	}
	switch c {
	case '.':
		s = s[1:]
		if c2, ok2 := lookahead(s); ok2 && isDigit(c2) {
			goto postDecimalPoint
		}
		goto exponentPartOpt
	case 'e', 'E':
		s = s[1:]
		goto postE
	default:
		return false
	}

postDecimalPoint:
	if c2, ok2 := lookahead(s); ok2 && isDigit(c2) {
		s = consumeDigits(s)
		goto exponentPartOpt
	}
	return false

exponentPartOpt:
	if s == "" {
		return true
	}
	c, ok = lookahead(s)
	if !ok {
		return true
	}
	switch c {
	case 'e', 'E':
		s = s[1:]
		goto postE
	default:
		return false
	}

postE:
	if s == "" {
		return false
	}
	if c3, ok3 := lookahead(s); ok3 {
		if c3 == '-' || c3 == '+' {
			s = s[1:]
		}
	}
	if s == "" {
		return false
	}
	if c4, ok4 := lookahead(s); ok4 && isDigit(c4) {
		s = consumeDigits(s)
		return s == ""
	}
	return false
}
