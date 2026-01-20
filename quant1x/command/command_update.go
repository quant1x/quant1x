package command

import (
	"fmt"

	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/markets"
	cli "github.com/spf13/cobra"
)

const (
	updateCommand     = "update"
	updateDescription = "更新数据"
)

var (
	// CmdUpdate 更新数据
	CmdUpdate *cli.Command = nil
	barIndex               = 1
)

func initUpdate() {
	CmdUpdate = &cli.Command{
		Use:     updateCommand,
		Example: Application + " " + updateCommand + " --all",
		//Args:    args.MinimumNArgs(0),
		Args: func(cmd *cli.Command, args []string) error {
			return nil
		},
		Short: updateDescription,
		Long:  updateDescription,
		Run: func(cmd *cli.Command, args []string) {
			fmt.Println()
			now := exchange.NowTimestamp()
			currentDate := exchange.LastTradingDay(now)
			tsStart, err := exchange.NewTimestampFromString(flagStartDate.Value)
			if err != nil {
				fmt.Printf("Error: 无效的开始日期: %s\n", flagStartDate.Value)
				_ = cmd.Usage()
				return
			}
			tsStart = tsStart.PreMarketTime()
			fmt.Println("开始日期:", tsStart.OnlyDate())
			tsEnd, err := exchange.NewTimestampFromString(flagEndDate.Value)
			if err != nil {
				fmt.Printf("Error: 无效的结束日期: %s\n", flagEndDate.Value)
				_ = cmd.Usage()
				return
			}
			tsEnd = tsEnd.PreMarketTime()
			fmt.Println("结束日期:", tsEnd.OnlyDate())
			plugins := []data.DataAdapter{}
			if flagAll.Value {
				// 全部更新
				//handleUpdateAll(cacheDate, featureDate)
				plugins = data.Plugins(0)
			} else {
				var basePlugins []data.DataAdapter
				var featurePlugins []data.DataAdapter
				if len(flagBaseData.Value) > 0 {
					all, keywords := parseFields(flagBaseData.Value)
					if all {
						basePlugins = data.Plugins(data.PluginMaskBaseData)
					} else if len(keywords) > 0 {
						basePlugins = data.PluginsWithName(data.PluginMaskBaseData, keywords...)
					}
				} else if len(flagFeatures.Value) > 0 {
					all, keywords := parseFields(flagFeatures.Value)
					if all {
						featurePlugins = data.Plugins(data.PluginMaskFeature)
					} else if len(keywords) > 0 {
						featurePlugins = data.PluginsWithName(data.PluginMaskFeature, keywords...)
					}
				} else {
					fmt.Println("Error: 非全部更新, 必须携带--features或--base")
					_ = cmd.Usage()
					return
				}
				plugins = append(plugins, basePlugins...)
				plugins = append(plugins, featurePlugins...)
			}
			fmt.Println("plugin num:", len(plugins))
			ts := exchange.DateRange(tsStart, tsEnd, false)
			fmt.Println(ts)
			fmt.Println("date count:", len(ts))
			codes := markets.GetCodeList()
			fmt.Println("code count:", len(codes))
			for _, date := range ts {
				fmt.Println("处理日期:", date.OnlyDate())
				data.UpdateWithAdapters(plugins, date, codes)
			}
			_ = currentDate
		},
	}
	commandInit(CmdUpdate, &flagAll)
	commandInit(CmdUpdate, &flagStartDate)
	commandInit(CmdUpdate, &flagEndDate)

	// 1. 基础数据
	plugins := data.Plugins(data.PluginMaskBaseData)
	flagBaseData.Usage = getPluginsUsage(plugins)
	commandInit(CmdUpdate, &flagBaseData)

	// 2. 特征数据
	plugins = data.Plugins(data.PluginMaskFeature)
	flagFeatures.Usage = getPluginsUsage(plugins)
	commandInit(CmdUpdate, &flagFeatures)

	//// 3. 处理异常
	//CmdUpdate.SetFlagErrorFunc(func(cmd *cli.Command, err error) error {
	//	return nil
	//})
}
