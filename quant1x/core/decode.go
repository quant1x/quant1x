package core

import (
	"fmt"
	"reflect"
	"strings"

	"gopkg.in/yaml.v3"
)

// DecodeTo 将 src（通常是 map[string]any / []any / 基础类型）解码到 dst（通常是 *Struct）。
//
// 采用 yaml 的 marshal/unmarshal 作为桥接，适合从动态 map 转成强类型结构体。
func DecodeTo(dst any, src any) error {
	if dst == nil {
		return fmt.Errorf("DecodeTo: dst is nil")
	}
	dstVal := reflect.ValueOf(dst)
	if dstVal.Kind() != reflect.Pointer || dstVal.IsNil() {
		return fmt.Errorf("DecodeTo: dst must be non-nil pointer")
	}

	// 直接赋值：src 是基础类型，dst 也是指向基础类型的指针
	dstElem := dstVal.Elem()
	srcVal := reflect.ValueOf(src)
	if isBasicKind(dstElem.Kind()) && isBasicKind(srcVal.Kind()) {
		if srcVal.Type().AssignableTo(dstElem.Type()) {
			dstElem.Set(srcVal)
			return nil
		}
		if srcVal.Type().ConvertibleTo(dstElem.Type()) {
			dstElem.Set(srcVal.Convert(dstElem.Type()))
			return nil
		}
		return fmt.Errorf("DecodeTo: cannot assign %v to %v", srcVal.Type(), dstElem.Type())
	}

	// 其它情况仍走 marshal/unmarshal，但先剔除 src 中的显式 null，以免覆盖默认值
	cleaned := pruneNil(src)
	data, err := yaml.Marshal(cleaned)
	if err != nil {
		return fmt.Errorf("DecodeTo: marshal: %w", err)
	}
	if err := yaml.Unmarshal(data, dst); err != nil {
		return fmt.Errorf("DecodeTo: unmarshal: %w", err)
	}
	return nil
}

func isBasicKind(k reflect.Kind) bool {
	switch k {
	case reflect.Bool, reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64,
		reflect.Uint, reflect.Uint8, reflect.Uint16, reflect.Uint32, reflect.Uint64,
		reflect.Float32, reflect.Float64, reflect.String:
		return true
	default:
		return false
	}
}

// pruneNil recursively removes nil values from maps and slices so that
// explicit nulls in configuration won't overwrite pre-set defaults.
func pruneNil(v any) any {
	switch t := v.(type) {
	case map[string]any:
		out := make(map[string]any, len(t))
		for k, val := range t {
			if val == nil {
				continue
			}
			out[k] = pruneNil(val)
		}
		return out
	case map[any]any:
		out := make(map[any]any, len(t))
		for k, val := range t {
			if val == nil {
				continue
			}
			out[k] = pruneNil(val)
		}
		return out
	case []any:
		out := make([]any, 0, len(t))
		for _, it := range t {
			if it == nil {
				continue
			}
			out = append(out, pruneNil(it))
		}
		return out
	default:
		return v
	}
}

// LookupConfig 从 GetConfigMapRef() 中按路径查找值。
//
// path 支持用 '.' 分隔的多级 key，例如："engine.mysql"。
func LookupConfig(path string) (any, bool) {
	m := GetConfigMapRef()
	if path == "" {
		return m, true
	}
	parts := strings.Split(path, ".")
	var current any = m
	for _, p := range parts {
		p = strings.TrimSpace(p)
		if p == "" {
			return nil, false
		}
		if current == nil {
			return nil, false
		}
		switch node := current.(type) {
		case map[string]any:
			v, ok := node[p]
			if !ok {
				return nil, false
			}
			current = v
		case map[any]any:
			v, ok := node[p]
			if !ok {
				return nil, false
			}
			current = v
		default:
			return nil, false
		}
	}
	return current, true
}

// DecodeConfig 按 path 查找配置段，并解码到 dst。
//
// - 如果目标结构体字段带有 `default:"..."`，会在解码后调用 ApplyDefaults(dst) 填充零值默认值。
// - 如果配置文件中配置项存在且有效，则覆盖默认值；如果不存在，则仅使用默认值。
func DecodeConfig(path string, dst any) error {
	// Apply defaults first so target has defaults even if path is missing.
	if err := ApplyDefaults(dst); err != nil {
		return fmt.Errorf("DecodeConfig: apply defaults: %w", err)
	}

	src, ok := LookupConfig(path)
	if !ok {
		// Path not found, use defaults only
		return nil
	}

	// Decode into a temporary object (which has defaults applied) so failures
	// don't leave `dst` partially modified. Only copy back on success.
	dstVal := reflect.ValueOf(dst)
	if dstVal.Kind() != reflect.Pointer || dstVal.IsNil() {
		return fmt.Errorf("DecodeConfig: dst must be non-nil pointer")
	}
	tmp := reflect.New(dstVal.Elem().Type()).Interface()
	// initialize tmp with dst's defaults to avoid calling ApplyDefaults twice
	tmpValInit := reflect.ValueOf(tmp)
	tmpValInit.Elem().Set(dstVal.Elem())

	if err := DecodeTo(tmp, src); err != nil {
		return err
	}

	// copy tmp -> dst
	tmpVal := reflect.ValueOf(tmp)
	dstVal.Elem().Set(tmpVal.Elem())
	return nil
}