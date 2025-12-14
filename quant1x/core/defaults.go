package core

import (
	"encoding"
	"fmt"
	"reflect"
	"strconv"
	"time"
)

// DefaultValue 默认值接口
type DefaultValue interface {
	Default() any
}

var (
	textUnmarshalerType = reflect.TypeOf((*encoding.TextUnmarshaler)(nil)).Elem()
	defaultValueType    = reflect.TypeOf((*DefaultValue)(nil)).Elem()
	durationType        = reflect.TypeOf(time.Duration(0))
)

// ApplyDefaults 给结构体零值字段填充默认值。
//
// 规则：
//   - 仅当字段为零值时才会应用 `default:"..."`。
//   - 递归处理嵌套结构体；对非 nil 的指针字段递归处理其指向的结构体。
//   - 支持基础类型(string/bool/int/uint/float)、time.Duration，以及实现 encoding.TextUnmarshaler 的类型(如 time.Time)。
//   - 支持字段类型实现 DefaultValue：当字段为零值且没有 default tag 时，会用 Default() 返回值赋值(可赋值/可转换时)。
//
// target 必须是可写的指针（通常是 *Struct）。
func ApplyDefaults(target any) error {
	if target == nil {
		return fmt.Errorf("ApplyDefaults: target is nil")
	}
	v := reflect.ValueOf(target)
	if v.Kind() != reflect.Pointer {
		return fmt.Errorf("ApplyDefaults: target must be a pointer, got %s", v.Kind())
	}
	if v.IsNil() {
		return fmt.Errorf("ApplyDefaults: target is nil pointer")
	}
	visited := map[uintptr]struct{}{}
	return applyDefaultsValue(v, visited, "")
}

func applyDefaultsValue(v reflect.Value, visited map[uintptr]struct{}, path string) error {
	// Unwrap pointers while tracking cycles
	for v.Kind() == reflect.Pointer {
		if v.IsNil() {
			return nil
		}
		ptr := v.Pointer()
		if ptr != 0 {
			if _, ok := visited[ptr]; ok {
				return nil
			}
			visited[ptr] = struct{}{}
		}
		v = v.Elem()
	}

	if v.Kind() != reflect.Struct {
		return nil
	}

	t := v.Type()
	for i := 0; i < v.NumField(); i++ {
		sf := t.Field(i)
		if sf.PkgPath != "" { // unexported
			continue
		}
		fv := v.Field(i)
		fieldPath := sf.Name
		if path != "" {
			fieldPath = path + "." + sf.Name
		}

		tag := sf.Tag.Get("default")
		if tag != "" && tag != "-" {
			if err := applyTagDefault(fv, tag, fieldPath); err != nil {
				return err
			}
		} else {
			if err := applyInterfaceDefault(fv); err != nil {
				return fmt.Errorf("ApplyDefaults: %s: %w", fieldPath, err)
			}
		}

		// Recurse into nested structs/pointers-to-structs (after applying defaults).
		switch fv.Kind() {
		case reflect.Struct:
			if fv.CanAddr() {
				if err := applyDefaultsValue(fv.Addr(), visited, fieldPath); err != nil {
					return err
				}
			}
		case reflect.Pointer:
			if !fv.IsNil() {
				if err := applyDefaultsValue(fv, visited, fieldPath); err != nil {
					return err
				}
			}
		}
	}

	return nil
}

func applyTagDefault(field reflect.Value, tag string, fieldPath string) error {
	if !field.IsValid() {
		return nil
	}
	if field.Kind() == reflect.Pointer {
		if field.IsNil() {
			// Only allocate pointer field when a tag is explicitly present.
			if !field.CanSet() {
				return nil
			}
			ptr := reflect.New(field.Type().Elem())
			if err := setFromString(ptr.Elem(), tag); err != nil {
				return fmt.Errorf("ApplyDefaults: %s: %w", fieldPath, err)
			}
			field.Set(ptr)
			return nil
		}
		// Non-nil pointer: apply only if underlying is zero
		if field.Elem().IsZero() {
			if err := setFromString(field.Elem(), tag); err != nil {
				return fmt.Errorf("ApplyDefaults: %s: %w", fieldPath, err)
			}
		}
		return nil
	}

	if field.IsZero() {
		if !field.CanSet() {
			return nil
		}
		if err := setFromString(field, tag); err != nil {
			return fmt.Errorf("ApplyDefaults: %s: %w", fieldPath, err)
		}
	}
	return nil
}

func applyInterfaceDefault(field reflect.Value) error {
	if !field.IsValid() {
		return nil
	}
	if !field.IsZero() {
		return nil
	}

	// Value receiver
	if field.Type().Implements(defaultValueType) {
		if !field.CanInterface() {
			return nil
		}
		dv := field.Interface().(DefaultValue).Default()
		return setFromAny(field, dv)
	}

	// Pointer receiver
	if field.Kind() != reflect.Pointer && field.CanAddr() && field.Addr().Type().Implements(defaultValueType) {
		dv := field.Addr().Interface().(DefaultValue).Default()
		return setFromAny(field, dv)
	}

	if field.Kind() == reflect.Pointer {
		// nil pointer that implements DefaultValue via element pointer receiver
		if field.IsNil() && field.CanSet() {
			ptr := reflect.New(field.Type().Elem())
			if ptr.Type().Implements(defaultValueType) {
				dv := ptr.Interface().(DefaultValue).Default()
				if err := setFromAny(ptr.Elem(), dv); err != nil {
					return err
				}
				field.Set(ptr)
			}
		}
	}

	return nil
}

func setFromAny(dst reflect.Value, anyValue any) error {
	if anyValue == nil {
		return nil
	}
	src := reflect.ValueOf(anyValue)
	if !dst.CanSet() {
		return nil
	}
	if src.Type().AssignableTo(dst.Type()) {
		dst.Set(src)
		return nil
	}
	if src.Type().ConvertibleTo(dst.Type()) {
		dst.Set(src.Convert(dst.Type()))
		return nil
	}
	return fmt.Errorf("cannot assign default value of type %s to %s", src.Type(), dst.Type())
}

func setFromString(dst reflect.Value, s string) error {
	// Prefer TextUnmarshaler if available
	if dst.CanAddr() {
		addr := dst.Addr()
		if addr.Type().Implements(textUnmarshalerType) {
			return addr.Interface().(encoding.TextUnmarshaler).UnmarshalText([]byte(s))
		}
	}

	// time.Duration
	if dst.Type() == durationType {
		d, err := time.ParseDuration(s)
		if err != nil {
			return err
		}
		dst.SetInt(int64(d))
		return nil
	}

	switch dst.Kind() {
	case reflect.String:
		dst.SetString(s)
		return nil
	case reflect.Bool:
		b, err := strconv.ParseBool(s)
		if err != nil {
			return err
		}
		dst.SetBool(b)
		return nil
	case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
		i, err := strconv.ParseInt(s, 0, dst.Type().Bits())
		if err != nil {
			return err
		}
		dst.SetInt(i)
		return nil
	case reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64, reflect.Uintptr:
		u, err := strconv.ParseUint(s, 0, dst.Type().Bits())
		if err != nil {
			return err
		}
		dst.SetUint(u)
		return nil
	case reflect.Float32, reflect.Float64:
		f, err := strconv.ParseFloat(s, dst.Type().Bits())
		if err != nil {
			return err
		}
		dst.SetFloat(f)
		return nil
	}

	return fmt.Errorf("unsupported default type %s", dst.Type())
}
