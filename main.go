// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package main

import (
	"fmt"
	"os"
	"time"

	"github.com/quant1x/quant1x/quant1x/command"
	"github.com/quant1x/quant1x/quant1x/runtime"
	"github.com/quant1x/quant1x/quant1x/util"

	_ "github.com/quant1x/quant1x/quant1x/contrib/data/tdx" // for data provider plugins
)

// _ "github.com/quant1x/quant1x/quant1x/data/market"      // for go:linkname GetCodeList
var (
	MinVersion  = util.InvalidVersion // 版本号
	application = "q1x-go"            // 应用程序名
)

func resetVersions() {
	if MinVersion == util.InvalidVersion {
		MinVersion = util.CurrentVersion()
	}
}

func main() {
	os.Exit(run())
}

// run 执行主流程并返回进程退出码
func run() int {
	mainStart := time.Now()
	resetVersions()
	defer func() {
		runtime.WaitForShutdown(1) // 等待最多1秒钟,以完成优雅退出
		runtime.CatchPanic("")
		elapsedTime := time.Since(mainStart) / time.Millisecond
		fmt.Printf("\n总耗时: %.3fs\n", float64(elapsedTime)/1000)
	}()
	// 更新应用程序名
	command.UpdateApplicationName(application)
	// quant1x模块内的更新版本号
	command.UpdateApplicationVersion(MinVersion)
	runtime.GoMaxProcs()
	return command.Run(os.Args[1:], os.Stdout, os.Stderr)
}
