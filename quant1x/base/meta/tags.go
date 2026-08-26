package meta

import (
	"fmt"
	"reflect"
	"strings"

	"github.com/quant1x/quant1x/quant1x/base"
)

const (
	StructDefaultTagNameOfTableHeader = "name"
)

var (
	// 结构体tag映射
	__mapStructTags = map[reflect.Type]map[int]string{}
	// 结构体作为TableView显示的表头
	__mapTableHeaders = map[reflect.Type][]string{}
)

// GetFieldTags 缓存字段Tag
func GetFieldTags(t reflect.Type, tag ...string) map[int]string {
	ma, mok := __mapStructTags[t]
	if mok {
		return ma
	}
	tagName := StructDefaultTagNameOfTableHeader
	if len(tag) > 0 {
		tagName = tag[0]
	}
	ma = nil
	fieldNum := t.NumField()
	for i := 0; i < fieldNum; i++ {
		field := t.Field(i)
		tag := field.Tag
		if len(tag) > 0 {
			tv, ok := tag.Lookup(tagName)
			tv, _, _ = strings.Cut(tv, ",")
			if ok {
				tv = strings.TrimSpace(tv)
				if ma == nil {
					ma = make(map[int]string)
					__mapStructTags[t] = ma
				}
				ma[i] = tv
			}
		}
	}
	return ma
}

// GetStructHeaders 获取structs表头
func GetStructHeaders(t reflect.Type, tag ...string) []string {
	v, ok := __mapTableHeaders[t]
	if ok {
		return v
	}
	tagName := StructDefaultTagNameOfTableHeader
	if len(tag) > 0 {
		tagName = tag[0]
	}
	mapTags := GetFieldTags(t, tagName)
	var headers []string
	if len(mapTags) == 0 {
		return headers
	}
	fieldNum := t.NumField()
	for i := 0; i < fieldNum; i++ {
		field, ok := mapTags[i]
		if ok {
			headers = append(headers, field)
		}
	}
	__mapTableHeaders[t] = headers
	return headers
}

func checkStructType(value any) (reflect.Type, reflect.Value) {
	obj := reflect.ValueOf(value)
	t := obj.Type()
	if obj.Kind() == reflect.Ptr {
		t = t.Elem()
		obj = obj.Elem()
	}
	return t, obj
}

// GetHeadersByTags 获取字段表头
func GetHeadersByTags(value any, tag ...string) []string {
	t, _ := checkStructType(value)
	return GetStructHeaders(t, tag...)

}

// GetValuesByTags Values 输出表格的行和列
func GetValuesByTags(value any, tag ...string) []string {
	t, obj := checkStructType(value)
	mapTags := GetFieldTags(t, tag...)
	var values []string
	if len(mapTags) == 0 {
		return values
	}
	fieldNum := t.NumField()
	for i := 0; i < fieldNum; i++ {
		_, ok := mapTags[i]
		if ok {
			ov := obj.Field(i).Interface()
			var str string
			switch v := ov.(type) {
			case float32:
				str = fmt.Sprintf("%.02f", base.Decimal(float64(v), 2))
			case float64:
				str = fmt.Sprintf("%.02f", base.Decimal(v, 2))
			default:
				str = base.ToString(ov)
			}
			values = append(values, str)
		}
	}
	return values
}
