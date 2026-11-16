// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package command

import (
	"fmt"
	"os"

	"github.com/quant1x/quant1x/quant1x/base/argparser"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
	"github.com/quant1x/quant1x/quant1x/data"
)

const (
	updateCommand     = "update"
	updateDescription = "更新数据"
)

// update 子命令的选项名
const (
	flagAll       = "all"
	flagBaseData  = "base"
	flagFeatures  = "features"
	flagStartDate = "start"
	flagEndDate   = "end"
)

// registerUpdate 登记 update 子命令及其选项
func registerUpdate(subparsers *argparser.SubParsers) (*argparser.Parser, error) {
	sub, err := subparsers.AddParser(updateCommand, updateDescription, nil, nil)
	if err != nil {
		return nil, err
	}
	defaultDate := data.LastTradingDay(data.NowTimestamp()).OnlyDate()
	options := []struct {
		name string
		arg  argparser.Argument
	}{
		{flagAll, argparser.Argument{Action: argparser.StoreTrue, Help: "全部"}},
		{flagBaseData, argparser.Argument{Default: argparser.ValueOf(""), Help: getPluginsUsage(data.Plugins(data.PluginMaskBaseData)), HideDefault: true}},
		{flagFeatures, argparser.Argument{Default: argparser.ValueOf(""), Help: getPluginsUsage(data.Plugins(data.PluginMaskFeature)), HideDefault: true}},
		{flagStartDate, argparser.Argument{Default: argparser.ValueOf(defaultDate), Help: "开始日期"}},
		{flagEndDate, argparser.Argument{Default: argparser.ValueOf(defaultDate), Help: "结束日期"}},
	}
	for _, option := range options {
		if _, err := sub.AddArgument([]string{"--" + option.name}, option.arg); err != nil {
			return nil, err
		}
	}
	return sub, nil
}

// runUpdate 执行 update 子命令
func runUpdate(namespace *argparser.Namespace, sub *argparser.Parser) {
	flagAllValue := namespaceBool(namespace, flagAll)
	baseDataValue := namespaceString(namespace, flagBaseData)
	featuresValue := namespaceString(namespace, flagFeatures)
	startDateValue := namespaceString(namespace, flagStartDate)
	endDateValue := namespaceString(namespace, flagEndDate)

	fmt.Println()
	now := data.NowTimestamp()
	currentDate := data.LastTradingDay(now)
	tsStart, err := data.NewTimestampFromString(startDateValue)
	if err != nil {
		fmt.Printf("Error: 无效的开始日期: %s\n", startDateValue)
		sub.PrintHelp(os.Stdout, false)
		return
	}
	tsStart = tsStart.PreMarketTime()
	fmt.Println("开始日期:", tsStart.OnlyDate())
	tsEnd, err := data.NewTimestampFromString(endDateValue)
	if err != nil {
		fmt.Printf("Error: 无效的结束日期: %s\n", endDateValue)
		sub.PrintHelp(os.Stdout, false)
		return
	}
	tsEnd = tsEnd.PreMarketTime()
	fmt.Println("结束日期:", tsEnd.OnlyDate())
	plugins := []data.DataAdapter{}
	if flagAllValue {
		// 全部更新
		plugins = data.Plugins(0)
	} else {
		var basePlugins []data.DataAdapter
		var featurePlugins []data.DataAdapter
		if len(baseDataValue) > 0 {
			all, keywords := parseFields(baseDataValue)
			if all {
				basePlugins = data.Plugins(data.PluginMaskBaseData)
			} else if len(keywords) > 0 {
				basePlugins = data.PluginsWithName(data.PluginMaskBaseData, keywords...)
			}
		} else if len(featuresValue) > 0 {
			all, keywords := parseFields(featuresValue)
			if all {
				featurePlugins = data.Plugins(data.PluginMaskFeature)
			} else if len(keywords) > 0 {
				featurePlugins = data.PluginsWithName(data.PluginMaskFeature, keywords...)
			}
		} else {
			fmt.Println("Error: 非全部更新, 必须携带--features或--base")
			sub.PrintHelp(os.Stdout, false)
			return
		}
		plugins = append(plugins, basePlugins...)
		plugins = append(plugins, featurePlugins...)
	}
	fmt.Println("plugin num:", len(plugins))
	ts := data.DateRange(tsStart, tsEnd, false)
	fmt.Println(ts)
	fmt.Println("date count:", len(ts))
	codes := tdx.GetCodeList()
	fmt.Println("code count:", len(codes))
	for _, date := range ts {
		fmt.Println("处理日期:", date.OnlyDate())
		data.UpdateWithAdapters(plugins, date, codes)
	}
	_ = currentDate
}
