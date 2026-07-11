// Copyright (c) 2015, Emir Pasic. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Package utils provides common utility functions.
//
// Provided functionalities:
// - sorting
// - comparators

package base

import (
	"fmt"
	"strconv"
	"strings"
)

// ToString converts a value to string.
func ToString(value any) string {
	switch value := value.(type) {
	case string:
		return value
	case int8:
		return strconv.FormatInt(int64(value), 10)
	case int16:
		return strconv.FormatInt(int64(value), 10)
	case int32:
		return strconv.FormatInt(int64(value), 10)
	case int64:
		return strconv.FormatInt(int64(value), 10)
	case uint8:
		return strconv.FormatUint(uint64(value), 10)
	case uint16:
		return strconv.FormatUint(uint64(value), 10)
	case uint32:
		return strconv.FormatUint(uint64(value), 10)
	case uint64:
		return strconv.FormatUint(uint64(value), 10)
	case float32:
		return strconv.FormatFloat(float64(value), 'g', -1, 64)
	case float64:
		return strconv.FormatFloat(float64(value), 'g', -1, 64)
	case bool:
		return strconv.FormatBool(value)
	default:
		return fmt.Sprintf("%+v", value)
	}
}

// ToCamelCase 转驼峰
func ToCamelCase(kebab string) (camelCase string) {
	isToUpper := false
	for _, runeValue := range kebab {
		if isToUpper {
			camelCase += strings.ToUpper(string(runeValue))
			isToUpper = false
		} else {
			if runeValue == '-' || runeValue == '_' {
				isToUpper = true
			} else {
				camelCase += string(runeValue)
			}
		}
	}
	return
}

// StartsWith 字符串前缀判断
func StartsWith(str string, prefixes []string) bool {
	if len(str) == 0 || len(prefixes) == 0 {
		return false
	}
	for _, prefix := range prefixes {
		if strings.HasPrefix(str, prefix) {
			return true
		}
	}
	return false
}

// EndsWith 字符串前缀判断
func EndsWith(str string, suffixes []string) bool {
	if len(str) == 0 || len(suffixes) == 0 {
		return false
	}
	for _, prefix := range suffixes {
		if strings.HasSuffix(str, prefix) {
			return true
		}
	}
	return false
}

// IsEmpty Code to test if string is empty
func IsEmpty(s string) bool {
	if strings.TrimSpace(s) == "" {
		return true
	} else {
		return false
	}
}
