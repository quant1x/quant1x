argparse（Go）使用说明
=====================

本文档描述项目内 Go 版 argparse 的用法与行为约定。

- 代码位置：quant1x/std/argparse
- Go 包路径：`github.com/quant1x/quant1x/quant1x/std/argparse`
- 设计目标：与 C++ 规范实现 `third_party/include/argparse/argparse.hpp` 的行为对齐（尤其是解析规则、usage/help 输出与错误信息）。


快速开始
--------

```go
package main

import (
    "fmt"
    "os"

    "github.com/quant1x/quant1x/quant1x/std/argparse"
)

func main() {
    p := argparse.NewArgumentParser("demo", "1.0.0")
    p.AddDescription("demo program")

    var base int
    p.AddArgument("--base").
        Help("base value").
        DefaultValue(0).
        StoreInto(&base)

    var verbose bool
    p.AddArgument("-v", "--verbose").
        Help("verbose output").
        DefaultValue(false).
        ImplicitValue(true).
        Nargs(0).
        StoreInto(&verbose)

    // 注意：ParseArgs 约定 args[0] 是程序名（与 argparse.hpp 一致）
    if err := p.ParseArgs(os.Args); err != nil {
        fmt.Fprintln(os.Stderr, err.Error())
        os.Exit(1)
    }

    fmt.Println("base=", base, "verbose=", verbose)
}
```


参数写法（最重要）
------------------

【长参数是否需要空格？】

不需要“必须带空格”，下面两种都支持（对齐 `argparse.hpp::preprocess_arguments()`）：

- 空格分隔：`--base 10`
- 赋值分隔：`--base=10`

实现会把 `--base=10` 在预处理阶段拆成两个 token：`--base` 与 `10`。

【单横线长参数（例如 `-modfile`）是否支持？】

不支持作为“单横线长参数名”。该库遵循传统 UNIX 规则：

- `-abc` 形态会被当作“短选项捆绑”（compound short options）处理。
- 因此 `-modfile` 会被视作 `-m -o -d -f -i -l -e` 的组合。
- 若中间任意短选项未定义：
  - `ParseArgs()` 会报错：`Unknown argument: -modfile`
  - `ParseKnownArgs()` 会把 `-modfile` 作为 unknown 返回

如果你需要一个名为 `modfile` 的“长参数”，请使用双横线：`--modfile`。

【Windows 风格前缀与赋值符】

当你把前缀字符设置为包含 `/`（例如 `SetPrefixChars("-/")`），并且把赋值字符设置为包含 `:`（例如 `SetAssignChars("=:")`）时，可支持类似：

- `/A:Foo`

其拆分逻辑与 `--key=value` 同源。


核心概念
--------

【positional vs optional】

- positional：不以 prefix chars 开头（默认 prefix chars 为 `-`）
  - 例：`input`、`output`
- optional：以 prefix chars 开头
  - 例：`-v`、`--base`

【nargs（取值个数）】

- `Nargs(0)`：flag/开关类参数（不消耗值）
- `Nargs(1)`：默认行为（消耗 1 个值）
- `NargsRange(min, max)`：消耗指定范围个值
- `NargsPattern(...)`：支持 optional/any/at_least_one 模式（具体以实现为准）


常用 API
--------

【ArgumentParser】

- `NewArgumentParser(name string, version ...string) *ArgumentParser`
  - 自动添加 `-h/--help`
  - 当传入 version 时自动添加 `-v/--version`
  - 这两个默认参数会在触发时打印到 stdout，并终止进程（对齐 argparse.hpp 默认行为）

- 元信息
  - `AddDescription(string)`
  - `AddEpilog(string)`

- 解析
  - `ParseArgs(args []string) error`
  - `ParseKnownArgs(args []string) ([]string, error)`：返回 unknown

- 查询/取值
  - `At(name string) (*Argument, error)`
  - `Get(name string) (any, error)`
  - `GetInto(name string, dest any) error`
  - `Present(name string) (any, bool, error)`：只有“用户显式提供”才算 present
  - `PresentInto(name string, dest any) (bool, error)`

- 子命令
  - `AddSubparser(sub *ArgumentParser)`
  - `IsSubcommandUsed(name string) bool`

- 互斥组
  - `AddMutuallyExclusiveGroup(required bool) *MutuallyExclusiveGroup`

- usage/help
  - `Usage() string`
  - `FormatHelp() string`
  - `SetUsageMaxLineWidth(uint64)`：0 表示不换行（单行 usage）
  - `SetUsageBreakOnMutex(bool)`

【Argument】

通过 `AddArgument(names ...string)` 创建。names 可以是：

- positional：`"input"`
- optional：`"-v"`, `"--verbose"`

常用链式配置：

- `Help(string)`
- `Metavar(string)`
- `DefaultValue(any)`
- `ImplicitValue(any)`
- `Required()`
- `Hidden()`
- `Nargs(int)` / `NargsRange(min,max)` / `NargsPattern(p)`
- `Choices(...)` / `ChoicesInt(...)` / `ChoicesUint64(...)` 等
- `Action(func(string)(any,error))` / `ActionVoid(func(string) error)`
- `StoreInto(dest any)`：把值写入变量（dest 必须是指针）
- `Scan(shape rune)`：用于严格数值解析（与 argparse.hpp 的 scan 语义对齐）


常见用法示例
------------

【flag（开关）】

```go
var verbose bool
p.AddArgument("-v", "--verbose").
    Help("verbose output").
    DefaultValue(false).
    ImplicitValue(true).
    Nargs(0).
    StoreInto(&verbose)
```

- 用法：`--verbose` 或 `-v`
- 不需要值；如果写成 `--verbose=true`，是否允许取决于你是否让它消费值（一般不建议）。

【带值参数】

```go
var base int
p.AddArgument("--base").
    Help("base value").
    DefaultValue(0).
    StoreInto(&base)
```

- 用法：`--base 10` 或 `--base=10`

【choices（枚举约束）】

```go
var mode string
p.AddArgument("--mode").
    Choices("fast", "slow").
    DefaultValue("fast").
    StoreInto(&mode)
```

当值不在 choices 中时会报错（错误文案对齐 C++ 版本）。

【subcommand（子命令）】

```go
root := argparse.NewArgumentParser("prog")
run := argparse.NewArgumentParser("run")
run.AddDescription("run it")
root.AddSubparser(run)
```

用法：`prog run ...`。


解析与错误行为速查
------------------

- `ParseArgs()`：遇到未知参数会返回错误。
- `ParseKnownArgs()`：未知参数不会报错，会收集并返回。
- 当 parser 没有定义 positional，但用户传入了 positional token：
  - 若存在 subcommand：报 `Failed to parse 'X', did you mean 'Y'`
  - 否则若存在“需要值”的 optional：报 `Zero positional arguments expected, did you mean <usage>`
  - 否则：报 `Zero positional arguments expected`


与 Go 标准库 / go.exe 的差异说明
-------------------------------

- 标准库 `flag` 与很多工具（含 `go.exe`）允许“单横线长参数名”（例如 `-modfile`）。
- 本项目 argparse 为了与 C++ `argparse.hpp` 对齐，选择了传统 UNIX 解析：`-modfile` 会被当作短选项捆绑，而不是一个长参数。
- 因此：
  - 对外 CLI 推荐统一使用 `--long` 风格（例如 `--modfile`）。
