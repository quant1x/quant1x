// package main

// import (
// 	"fmt"

// 	"time"

// 	"gitee.com/quant1x/engine/command"
// 	"gitee.com/quant1x/engine/utils"
// 	"gitee.com/quant1x/gox/runtime"
// 	//_ "gitee.com/quant1x/labs/qlab/factors"
// 	//_ "gitee.com/quant1x/labs/services"
// 	//_ "gitee.com/quant1x/labs/strategies"
// )

// var (
// 	MinVersion  = utils.InvalidVersion // 版本号
// 	application = "q1x"                // 应用程序名
// )

// func resetVersions() {
// 	if MinVersion == utils.InvalidVersion {
// 		MinVersion = utils.CurrentVersion()
// 	}
// }

// // 更新数据工具
// func main() {
// 	mainStart := time.Now()
// 	resetVersions()
// 	defer func() {
// 		runtime.CatchPanic("")
// 		elapsedTime := time.Since(mainStart) / time.Millisecond
// 		fmt.Printf("\n总耗时: %.3fs\n", float64(elapsedTime)/1000)
// 	}()
// 	// 更新应用程序名
// 	command.UpdateApplicationName(application)
// 	// quant1x模块内的更新版本号
// 	command.UpdateApplicationVersion(MinVersion)
// 	runtime.GoMaxProcs()
// 	rootCommand := command.GlobalFlags()
// 	_ = rootCommand.Execute()
// }
