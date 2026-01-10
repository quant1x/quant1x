package main

import (
	"fmt"

	"time"

	"gitee.com/quant1x/quant1x/quant1x/command"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
	"gitee.com/quant1x/quant1x/quant1x/util"

	_ "gitee.com/quant1x/quant1x/quant1x/data/provider" // for data provider plugins
	_ "gitee.com/quant1x/quant1x/quant1x/markets"       // for go:linkname GetCodeList
)

var (
	MinVersion  = util.InvalidVersion // 版本号
	application = "q1x-go"            // 应用程序名
)

func resetVersions() {
	if MinVersion == util.InvalidVersion {
		MinVersion = util.CurrentVersion()
	}
}

// 更新数据工具
func main() {
	mainStart := time.Now()
	resetVersions()
	defer func() {
		runtime.CatchPanic("")
		elapsedTime := time.Since(mainStart) / time.Millisecond
		fmt.Printf("\n总耗时: %.3fs\n", float64(elapsedTime)/1000)
	}()
	// 更新应用程序名
	command.UpdateApplicationName(application)
	// quant1x模块内的更新版本号
	command.UpdateApplicationVersion(MinVersion)
	runtime.GoMaxProcs()
	rootCommand := command.GlobalFlags()
	_ = rootCommand.Execute()
}
