// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package command

import (
	"strings"

	"github.com/olekukonko/tablewriter"
	"github.com/olekukonko/tablewriter/tw"
	"github.com/quant1x/quant1x/quant1x/base/argparser"
	"github.com/quant1x/quant1x/quant1x/base/meta"
	"github.com/quant1x/quant1x/quant1x/data"
)

const (
	defaultFlagAll = "all" // 全部
)

// namespaceBool 读取布尔开关, 未出现或类型不符时取 false
func namespaceBool(namespace *argparser.Namespace, key string) bool {
	value, ok := namespace.Get(key)
	if !ok {
		return false
	}
	flag, _ := value.(bool)
	return flag
}

// namespaceString 读取字符串选项, 未出现或类型不符时取空串
func namespaceString(namespace *argparser.Namespace, key string) string {
	value, ok := namespace.Get(key)
	if !ok {
		return ""
	}
	text, _ := value.(string)
	return text
}

// namespaceInt 读取整数选项, 未出现或类型不符时取 0
func namespaceInt(namespace *argparser.Namespace, key string) int {
	value, ok := namespace.Get(key)
	if !ok {
		return 0
	}
	number, _ := value.(int64)
	return int(number)
}

func parseFields(text string) (all bool, keywords []string) {
	text = strings.TrimSpace(text)
	if len(text) == 0 {
		return
	}
	arr := strings.Split(text, ",")
	var list []string
	for _, v := range arr {
		v := strings.TrimSpace(v)
		if v == defaultFlagAll {
			return true, keywords
		} else if len(v) == 0 {
			continue
		} else {
			list = append(list, v)
		}
	}
	return false, list
}

type optionUsage struct {
	Key      string `name:"关键字"`
	Name     string `name:"概要"`
	Provider string `name:"数据源"`
	Usage    string `name:"描述"`
}

// 获取插件的Usage信息
func getPluginsUsage(plugins []data.DataAdapter) string {
	writer := strings.Builder{}
	table := tablewriter.NewWriter(&writer)
	table.Header(meta.GetHeadersByTags(optionUsage{}))
	table.Append(meta.GetValuesByTags(optionUsage{Key: defaultFlagAll, Name: "全部", Provider: ""}))
	for _, plugin := range plugins {
		ou := optionUsage{Key: plugin.Key(), Name: plugin.Name(), Provider: plugin.Owner(), Usage: plugin.Usage()}
		table.Append(meta.GetValuesByTags(ou))
	}
	table.Options(tablewriter.WithRendition(tw.Rendition{Borders: tw.BorderNone}))
	table.Render()
	return writer.String()
}
