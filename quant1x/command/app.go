// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package command

import (
	"fmt"
	"io"
	"os"
	goruntime "runtime"
	_ "unsafe" // For go:linkname

	//"github.com/quant1x/engine/models"
	//"github.com/quant1x/engine/tracker"
	"github.com/klauspost/cpuid/v2" // For cpuid
	"github.com/quant1x/num"
	"github.com/quant1x/quant1x/quant1x/base/argparser"
	"github.com/quant1x/quant1x/quant1x/log"
	"github.com/quant1x/quant1x/quant1x/runtime"
)

var (
	Application = "stock"
	MinVersion  = "0.0.1" // 主程序版本号
)

func init() {
	Application = runtime.ApplicationName()
}

// UpdateApplicationName 更新应用(Application)名称
func UpdateApplicationName(name string) {
	Application = name
}

// UpdateApplicationVersion 更新版本号
func UpdateApplicationVersion(v string) {
	MinVersion = v
}

// 获取CPU型号
func cpuModelName() string {
	return cpuid.CPU.BrandName
}

const (
	KB = 1024      // Kilo Byte
	MB = 1024 * KB // Mega Byte
	GB = 1024 * MB // Giga Byte
)

var (
	strategyNumber = uint64(1)              // 策略编号
	businessDebug  = runtime.Debug()        // 业务调试开关
	cpuAvx2        = false                  // AVX2加速状态
	cpuNum         = goruntime.NumCPU() / 2 // cpu数量
)

var (
	commandDefaultLongFlag = "" // 默认的长标志为空, 主要用在tools
)

// 根命令的全局选项名与子命令的落值名
const (
	flagDebug      = "debug" // 业务调试开关
	flagAvx2       = "avx2"  // AVX2 加速开关
	flagCPU        = "cpu"   // CPU 最大核数
	subcommandDest = "command"
)

// 输出欢迎语
func printMotd() {
	fmt.Printf("CPU: %s, %dCores, AVX2: %t\n", cpuModelName(), goruntime.NumCPU(), num.GetAvx2Enabled())
	fmt.Println()
}

// newRootParser 构建根命令与全部子命令
func newRootParser() (*argparser.Parser, error) {
	parser := argparser.NewParser(Application)
	if _, err := parser.AddArgument([]string{"--" + flagDebug}, argparser.Argument{
		Action: argparser.StoreTrue,
		Help:   "打开业务调试开关, 慎重使用!",
		Global: true,
	}); err != nil {
		return nil, err
	}
	if _, err := parser.AddArgument([]string{"--" + flagAvx2}, argparser.Argument{
		Action: argparser.StoreTrue,
		Help:   "Avx2 加速开关",
		Global: true,
	}); err != nil {
		return nil, err
	}
	if _, err := parser.AddArgument([]string{"--" + flagCPU}, argparser.Argument{
		Default: argparser.ValueOf(int64(cpuNum)),
		Help:    "设置CPU最大核数",
		Global:  true,
	}); err != nil {
		return nil, err
	}

	subparsers, err := parser.AddSubparsers(subcommandDest, "命令", false)
	if err != nil {
		return nil, err
	}
	if _, err = registerUpdate(subparsers); err != nil {
		return nil, err
	}
	return parser, nil
}

// Run 解析命令行并执行命中的子命令, 返回进程退出码
func Run(argv []string, out, errOut io.Writer) int {
	parser, err := newRootParser()
	if err != nil {
		fmt.Fprintf(errOut, "%v\n", err)
		return 2
	}
	namespace, code := parser.ParseOrExit(argv, out, errOut)
	if code != 0 || namespace == nil {
		return code
	}
	applyGlobalFlags(namespace)

	name := namespaceString(namespace, subcommandDest)
	switch name {
	case updateCommand:
		runUpdate(namespace, parser.Subparsers().Lookup(updateCommand))
	default:
		log.Warnf("stock default args:%+v", os.Args)
	}
	return 0
}

// applyGlobalFlags 把全局选项落到运行期开关上
func applyGlobalFlags(namespace *argparser.Namespace) {
	businessDebug = namespaceBool(namespace, flagDebug)
	cpuAvx2 = namespaceBool(namespace, flagAvx2)
	if value := namespaceInt(namespace, flagCPU); value > 0 {
		cpuNum = value
	}
	// 重置全局调试状态
	runtime.SetDebug(businessDebug)
	// AVX2 加速
	num.SetAvx2Enabled(cpuAvx2)
	// 设置CPU最大核数
	runtime.GoMaxProcs(cpuNum)
}
