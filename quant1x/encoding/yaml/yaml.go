// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

// Package yaml 在 gopkg.in/yaml.v3 的基础上支持结构体 tag 中的 default 默认值,
// 并提供与 yaml.v3 一致的 Node 别名, 供类型实现 UnmarshalYAML。
//
// 实现移植自 github.com/quant1x/gox v1.27.1 的 encoding/yaml 包 (Apache-2.0, 版权归 Quant1X Team)。
package yaml

import (
	"encoding"
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"strconv"
	"strings"
	"time"

	"gopkg.in/yaml.v3"
)

const (
	// defaultTagName 结构体tag中默认值的关键字
	defaultTagName = "default"
)

var (
	// errYamlOutput 输出参数错误
	errYamlOutput = errors.New("yaml: out must be a non-nil pointer")
	// errYamlNotStruct 不是结构体指针
	errYamlNotStruct = errors.New("yaml: out must be a pointer of struct")
)

// Node yaml文档的节点类型, 与gopkg.in/yaml.v3的Node一致
//
//	自定义解析的类型, 可以用它实现UnmarshalYAML, 例如:
//	func (this *TradingSession) UnmarshalYAML(node *yaml.Node) error
type Node = yaml.Node

// Unmarshal 解析yaml文本
//
//	在标准库gopkg.in/yaml.v3的基础上, 保留结构体tag中default的用法, 口径如下:
//	1) yaml中出现的字段, 以yaml的值为准, 即便是显式的零值, 例如 total: 0;
//	2) yaml中没有出现的字段, 取default标签的值;
//	3) 嵌套结构、切片元素、map的值, 逐层按上面两条处理.
func Unmarshal(data []byte, out any) error {
	var document yaml.Node
	if err := yaml.Unmarshal(data, &document); err != nil {
		return err
	}
	if document.Kind == 0 || len(document.Content) == 0 {
		// 空文档, 不做处理
		return nil
	}
	v := reflect.ValueOf(out)
	if v.Kind() != reflect.Pointer || v.IsNil() {
		return errYamlOutput
	}
	root := document.Content[0]
	// 先按标准库解析, 解析结果为yaml中实际配置的值
	if err := root.Decode(out); err != nil {
		return err
	}
	// 再按节点补齐yaml中没有配置的字段的默认值
	applyDefaults(root, v.Elem())
	return nil
}

// Marshal 编码为yaml文本, 与gopkg.in/yaml.v3一致
func Marshal(in any) ([]byte, error) {
	return yaml.Marshal(in)
}

// applyDefaults 依据node中实际出现的字段, 填充未出现字段的默认值
func applyDefaults(node *yaml.Node, v reflect.Value) {
	if !v.IsValid() || !v.CanSet() {
		return
	}
	for v.Kind() == reflect.Pointer {
		if v.IsNil() {
			return
		}
		v = v.Elem()
	}
	switch v.Kind() {
	case reflect.Struct:
		applyStructDefaults(node, v)
	case reflect.Slice, reflect.Array:
		applySliceDefaults(node, v)
	case reflect.Map:
		applyMapDefaults(node, v)
	}
}

// applyStructDefaults 填充结构体中未配置字段的默认值
func applyStructDefaults(node *yaml.Node, v reflect.Value) {
	t := v.Type()
	fields := mappingFields(node)
	for i := 0; i < t.NumField(); i++ {
		ft := t.Field(i)
		if ft.Anonymous && ft.Type.Kind() == reflect.Struct {
			// 内嵌结构体, 字段在yaml中是平铺的
			applyDefaults(node, v.Field(i))
			continue
		}
		if ft.PkgPath != "" {
			// 未导出的字段, 跳过
			continue
		}
		name := yamlFieldName(ft)
		if len(name) == 0 {
			continue
		}
		fv := v.Field(i)
		if child, ok := fields[name]; ok {
			// yaml中配置了该字段, 以yaml的值为准, 继续向下一层查找未配置的字段
			applyDefaults(child, fv)
			continue
		}
		// yaml中没有配置, 取default标签的值
		_ = setDefault(fv, ft.Tag.Get(defaultTagName))
	}
}

// applySliceDefaults 填充切片元素中未配置字段的默认值
func applySliceDefaults(node *yaml.Node, v reflect.Value) {
	if node == nil || node.Kind != yaml.SequenceNode {
		return
	}
	for i := 0; i < v.Len() && i < len(node.Content); i++ {
		applyDefaults(node.Content[i], v.Index(i))
	}
}

// applyMapDefaults 填充map值中未配置字段的默认值
func applyMapDefaults(node *yaml.Node, v reflect.Value) {
	if node == nil || node.Kind != yaml.MappingNode || v.Len() == 0 {
		return
	}
	fields := mappingFields(node)
	for _, key := range v.MapKeys() {
		child, ok := fields[fmt.Sprintf("%v", key.Interface())]
		if !ok {
			continue
		}
		// map的值不可寻址, 复制一份处理完再写回
		value := v.MapIndex(key)
		copied := reflect.New(value.Type()).Elem()
		copied.Set(value)
		applyDefaults(child, copied)
		v.SetMapIndex(key, copied)
	}
}

// mappingFields 取mapping节点的字段表, 关键字->值节点
func mappingFields(node *yaml.Node) map[string]*yaml.Node {
	if node == nil || node.Kind != yaml.MappingNode {
		return nil
	}
	fields := make(map[string]*yaml.Node, len(node.Content)/2)
	for i := 0; i+1 < len(node.Content); i += 2 {
		fields[node.Content[i].Value] = node.Content[i+1]
	}
	return fields
}

// yamlFieldName 取字段在yaml中的关键字, 空字符串代表该字段不参与yaml解析
func yamlFieldName(field reflect.StructField) string {
	tag := field.Tag.Get("yaml")
	if tag == "-" {
		return ""
	}
	if index := strings.IndexByte(tag, ','); index >= 0 {
		tag = tag[:index]
	}
	if len(tag) > 0 {
		return tag
	}
	return strings.ToLower(field.Name)
}

// setDefaults 填充结构体中每个字段的default标签值
func setDefaults(ptr any) error {
	v := reflect.ValueOf(ptr)
	if v.Kind() != reflect.Pointer || v.IsNil() {
		return errYamlNotStruct
	}
	v = v.Elem()
	if v.Kind() != reflect.Struct {
		return errYamlNotStruct
	}
	t := v.Type()
	for i := 0; i < t.NumField(); i++ {
		ft := t.Field(i)
		if defaultVal := ft.Tag.Get(defaultTagName); defaultVal != "-" {
			if err := setDefault(v.Field(i), defaultVal); err != nil {
				return err
			}
		}
	}
	return nil
}

// setDefault 填充字段的默认值, defaultVal为空时, 只递归处理嵌套的结构
func setDefault(field reflect.Value, defaultVal string) error {
	if !field.CanSet() {
		return nil
	}
	if !shouldInitializeField(field, defaultVal) {
		return nil
	}
	isInitial := isInitialValue(field)
	if isInitial {
		if unmarshalByInterface(field, defaultVal) {
			return nil
		}
		switch field.Kind() {
		case reflect.Bool:
			if val, err := strconv.ParseBool(defaultVal); err == nil {
				field.Set(reflect.ValueOf(val).Convert(field.Type()))
			}
		case reflect.Int:
			if val, err := strconv.ParseInt(defaultVal, 0, strconv.IntSize); err == nil {
				field.Set(reflect.ValueOf(int(val)).Convert(field.Type()))
			}
		case reflect.Int8:
			if val, err := strconv.ParseInt(defaultVal, 0, 8); err == nil {
				field.Set(reflect.ValueOf(int8(val)).Convert(field.Type()))
			}
		case reflect.Int16:
			if val, err := strconv.ParseInt(defaultVal, 0, 16); err == nil {
				field.Set(reflect.ValueOf(int16(val)).Convert(field.Type()))
			}
		case reflect.Int32:
			if val, err := strconv.ParseInt(defaultVal, 0, 32); err == nil {
				field.Set(reflect.ValueOf(int32(val)).Convert(field.Type()))
			}
		case reflect.Int64:
			if val, err := time.ParseDuration(defaultVal); err == nil {
				field.Set(reflect.ValueOf(val).Convert(field.Type()))
			} else if val, err := strconv.ParseInt(defaultVal, 0, 64); err == nil {
				field.Set(reflect.ValueOf(val).Convert(field.Type()))
			}
		case reflect.Uint:
			if val, err := strconv.ParseUint(defaultVal, 0, strconv.IntSize); err == nil {
				field.Set(reflect.ValueOf(uint(val)).Convert(field.Type()))
			}
		case reflect.Uint8:
			if val, err := strconv.ParseUint(defaultVal, 0, 8); err == nil {
				field.Set(reflect.ValueOf(uint8(val)).Convert(field.Type()))
			}
		case reflect.Uint16:
			if val, err := strconv.ParseUint(defaultVal, 0, 16); err == nil {
				field.Set(reflect.ValueOf(uint16(val)).Convert(field.Type()))
			}
		case reflect.Uint32:
			if val, err := strconv.ParseUint(defaultVal, 0, 32); err == nil {
				field.Set(reflect.ValueOf(uint32(val)).Convert(field.Type()))
			}
		case reflect.Uint64:
			if val, err := strconv.ParseUint(defaultVal, 0, 64); err == nil {
				field.Set(reflect.ValueOf(val).Convert(field.Type()))
			}
		case reflect.Uintptr:
			if val, err := strconv.ParseUint(defaultVal, 0, strconv.IntSize); err == nil {
				field.Set(reflect.ValueOf(uintptr(val)).Convert(field.Type()))
			}
		case reflect.Float32:
			if val, err := strconv.ParseFloat(defaultVal, 32); err == nil {
				field.Set(reflect.ValueOf(float32(val)).Convert(field.Type()))
			}
		case reflect.Float64:
			if val, err := strconv.ParseFloat(defaultVal, 64); err == nil {
				field.Set(reflect.ValueOf(val).Convert(field.Type()))
			}
		case reflect.String:
			field.Set(reflect.ValueOf(defaultVal).Convert(field.Type()))
		case reflect.Slice:
			ref := reflect.New(field.Type())
			ref.Elem().Set(reflect.MakeSlice(field.Type(), 0, 0))
			if defaultVal != "" && defaultVal != "[]" {
				if err := json.Unmarshal([]byte(defaultVal), ref.Interface()); err != nil {
					return err
				}
			}
			field.Set(ref.Elem().Convert(field.Type()))
		case reflect.Map:
			ref := reflect.New(field.Type())
			ref.Elem().Set(reflect.MakeMap(field.Type()))
			if defaultVal != "" && defaultVal != "{}" {
				if err := json.Unmarshal([]byte(defaultVal), ref.Interface()); err != nil {
					return err
				}
			}
			field.Set(ref.Elem().Convert(field.Type()))
		case reflect.Struct:
			if defaultVal != "" && defaultVal != "{}" {
				if err := json.Unmarshal([]byte(defaultVal), field.Addr().Interface()); err != nil {
					return err
				}
			}
		case reflect.Pointer:
			field.Set(reflect.New(field.Type().Elem()))
		}
	}

	switch field.Kind() {
	case reflect.Pointer:
		if isInitial || field.Elem().Kind() == reflect.Struct {
			if err := setDefault(field.Elem(), defaultVal); err != nil {
				return err
			}
		}
	case reflect.Struct:
		if err := setDefaults(field.Addr().Interface()); err != nil {
			return err
		}
	case reflect.Slice:
		for j := 0; j < field.Len(); j++ {
			if err := setDefault(field.Index(j), defaultVal); err != nil {
				return err
			}
		}
	case reflect.Map:
		for _, key := range field.MapKeys() {
			value := field.MapIndex(key)
			switch value.Kind() {
			case reflect.Pointer:
				if value.Elem().Kind() == reflect.Struct || value.Elem().Kind() == reflect.Slice || value.Elem().Kind() == reflect.Map {
					if err := setDefault(value.Elem(), ""); err != nil {
						return err
					}
				}
			case reflect.Struct, reflect.Slice, reflect.Map:
				copied := reflect.New(value.Type()).Elem()
				copied.Set(value)
				if err := setDefault(copied, ""); err != nil {
					return err
				}
				field.SetMapIndex(key, copied)
			}
		}
	}
	return nil
}

// unmarshalByInterface 优先使用字段自己实现的文本和json反序列化接口
func unmarshalByInterface(field reflect.Value, defaultVal string) bool {
	if defaultVal == "" {
		return false
	}
	if asText, ok := field.Addr().Interface().(encoding.TextUnmarshaler); ok {
		if err := asText.UnmarshalText([]byte(defaultVal)); err == nil {
			return true
		}
	}
	if asJSON, ok := field.Addr().Interface().(json.Unmarshaler); ok && defaultVal != "{}" && defaultVal != "[]" {
		if err := asJSON.UnmarshalJSON([]byte(defaultVal)); err == nil {
			return true
		}
	}
	return false
}

// isInitialValue 字段是否是零值
func isInitialValue(field reflect.Value) bool {
	return reflect.DeepEqual(reflect.Zero(field.Type()).Interface(), field.Interface())
}

// shouldInitializeField 字段是否需要初始化
func shouldInitializeField(field reflect.Value, tag string) bool {
	switch field.Kind() {
	case reflect.Struct:
		return true
	case reflect.Pointer:
		if !field.IsNil() && field.Elem().Kind() == reflect.Struct {
			return true
		}
	case reflect.Slice:
		return field.Len() > 0 || tag != ""
	case reflect.Map:
		return field.Len() > 0 || tag != ""
	}
	return tag != ""
}
