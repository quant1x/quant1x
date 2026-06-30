# AGENTS.md — Quant1X 多语言 AI 协作规则

本文件是 AI（CodeBuddy / GitHub Copilot）在参与 Quant1X 项目时的强制执行规则。

详细架构与背景请阅读：[CONTRIBUTING.md](CONTRIBUTING.md)

## 一、总体原则（不可违反）

- 本项目为**多语言平行实现**，严禁任何跨语言调用（FFI / Cgo / SWIG）
- Python 负责探索验证，C/C++ 负责生产，Rust / Go 负责生态扩展
- **语义一致性高于语言习惯**
- 所有实现必须可通过同一组测试向量验证

## 二、目录与文件结构规则

- 严格遵循 [CONTRIBUTING.md](CONTRIBUTING.md) 第 2 节定义的目录结构
- Go 实现必须位于功能子目录中，例如 `data/t1/`
- Go 包名必须与目录名完全一致（如 `package t1`）
- **禁止**为了"方便"合并 Go 文件到根目录

## 三、命名规则

### 3.0 命名参考体系（Type Vocabulary）

本项目有一组核心领域类型的**标准名称**，以国际量化金融社区通用术语（Bloomberg / QuantLib / Backtrader / WRDS 等）为参考基准，四种语言必须使用同一名称。**拒绝拼音、音译、本地化翻译。** AI 生成代码时，必须以此表为准，不可即兴发明别名。

| 领域概念 | 标准类型名 | 说明 |
|---------|-----------|------|
| OHLCV K线数据结构 | **`Bar`** | 不得使用 `KLine`（已废弃的遗留别名，仅 C++ 保留 `using KLine = Bar` 向后兼容） |
| 未复权原始K线 | **`BarRaw`** | 不得使用 `KLineRaw` |
| K线周期枚举 | **`KLineType`** | 此处保留 "KLine" — 它是关于"K线类别"的元数据，非数据结构本身 |
| K线数据适配器类 | **`DataKLine`** | 适配器类名保留 "KLine" — 这是"拉取K线数据"的功能模块名，非数据结构 |
| 原始K线适配器 | **`DataKLineRaw`** | 同上 |
| 分钟K线适配器 | **`DataMinuteKLine`** | 同上 |
| 逐笔成交 | **`Transaction`** | 不得使用 `Tick`（`Tick` 仅作为方向常量前缀如 `TICK_BUY`，无 `Tick` struct） |
| 复权信息 | **`XdxrInfo`** | 除权除息信息 |
| 累积复权因子 | **`CumulativeAdjustment`** | 四种语言同名 |
| 板块分类 | **`Sector`** | 行业/概念板块 |
| 公司信息 | **`CompanyInfoChunk`** | 上市公司基本信息 |
| 分红调整 | **`DividendAdjustment`** | 分红除权调整 |
| 交易方向 | **`Direction`** | 买卖方向标志 |
| 证券K线（协议层） | **`SecurityBar`** | 网络协议/存储层使用的中间结构，Python 端复用 `Bar` |

**核心原则：以国际量化金融标准术语为准，拒绝本地化音译**

本项目类型命名以**国际量化金融社区通用术语**为唯一参考标准。量化交易是一个全球化领域，代码应能被任何操盘手、量化研究员直接阅读，不依赖中文语境。

**为什么用 `Bar` 而不用 `KLine`？**
- `Bar` 是国际量化金融中 OHLCV 柱状数据的**标准术语**（Bloomberg Terminal、QuantLib、Backtrader 等均使用 `Bar`）
- `KLine` 是中文"K线"的音译（源自日语"罫線" Keisen），属于本地化命名，不在国际通用术语范围内
- 同理，`Transaction` 不用 `Tick`（`Tick` 在国际期货/外汇市场另有精确定义：最小价格变动单位）
- 项目经历过 `kline-to-bar` 重构（v0.7.51），已将规范类型名从 `KLine` 统一为 `Bar`
- C++ 中保留 `using KLine = schema::Bar` 仅用于**旧代码向后兼容**，新代码不得引用 `KLine` 作为类型名

**新增类型的命名验收标准**：
1. 该名称是否是 Bloomberg / QuantLib / Backtrader / WRDS 等国际平台的惯用名？
2. 如果不是 → 拒绝，使用国际标准术语
3. 如果在国际标准中不存在直接对应 → 以英文描述性命名，禁止拼音/音译

**文件命名规则**：文件以**领域概念**命名，非以语言命名。同一概念在 `data/schema/` 下有平行的多语言文件：

```
data/schema/
├── bar.md            # 文档（Spec）
├── bar.h             # C++
├── bar.rs            # Rust
├── bar.go            # Go
├── bar.py            # Python
├── transaction.md
├── transaction.h
├── transaction.rs
├── transaction.go
├── transaction.py
└── ...
```

### 3.1 内存态（代码内部）

| 语言   | 函数 / 方法             | 类型 / Struct           | 变量 / 字段             |
|--------|------------------------|------------------------|------------------------|
| Python | `snake_case`           | `PascalCase`           | `snake_case`           |
| C/C++  | `snake_case`           | `PascalCase`           | `snake_case`           |
| Rust   | `snake_case`           | `PascalCase`           | `snake_case`           |
| Go     | `PascalCase`（导出）    | `PascalCase`           | `PascalCase` / `camelCase` |

**⚠️ AI 必须做到：**

- 从 Python / C++ 生成 Go 代码时，自动将 `snake_case` → `PascalCase`
- **绝不生成** `snake_case` 的 Go 导出符号
- **新增类型名必须查 §3.0 参考体系**，不可即兴发明名称
- 若发现某种语言使用了与 §3.0 不一致的名称，立即指出

### 3.2 边界态（I/O 与序列化）【铁律】

- 所有落盘字段名必须是**全小写 `snake_case`**
- Go 结构体必须显式添加 struct tag：

```go
type Bar struct {
    Open  float64 `json:"open" csv:"open"`
    High  float64 `json:"high" csv:"high"`
    Low   float64 `json:"low" csv:"low"`
    Close float64 `json:"close" csv:"close"`
    Volume float64 `json:"volume" csv:"volume"`
}
```

- AI 生成 Go 代码时，必须自动补全 tag
- **tag 内容永远是小写 snake_case**，不受内存命名影响
- **tag 内字段名必须与 Python/C++ 的 snake_case 字段名一致**，不可自行翻译

## 四、文档驱动的 AI 行为规则

### 上下文读取顺序（强制执行）

1. 优先读取同级 `*.md`（如 `t1.md`）
2. 其次读取 Python / C++ 参考实现
3. 最后才生成目标语言代码

### 生成代码必须满足

- **数学公式**：严格对照 `*.md` 中的 LaTeX
- **数据契约**：字段类型、精度、Null/NaN 行为与文档一致
- **边界条件**：必须实现文档中定义的所有异常分支

## 五、语言职责与 AI 行为约束

### Python

- 作为 Spec 锚点
- 所有新功能必须先有 Python 实现
- docstring 视为跨语言契约的一部分

### C / C++

- 作为生产真相源
- **严禁 UB**，严禁未定义行为
- 内存管理必须显式、可审计

### Rust

- **严禁** `unwrap()` / `expect()` 在业务路径
- 所有错误必须显式返回 `Result`
- `unsafe` 必须说明理由

### Go

- **严禁** `panic` 作为控制流
- 所有 error 必须显式返回
- 并发安全必须显式标注

## 六、AI 生成代码的硬性限制

- ❌ 不允许"看起来差不多"的实现
- ❌ 不允许省略边界条件处理
- ❌ 不允许假设字段默认值
- ❌ 不允许引入语言特有隐式行为
- ✅ 必须保证四种语言对同一输入的数值结果在 epsilon 范围内一致

## 七、开发工作流（AI 视角）

当被要求"实现 / 移植 / 补齐某功能"时，AI 必须按以下顺序执行：

1. 阅读 `CONTRIBUTING.md` 了解整体架构
2. 阅读对应 `t1.md`（如有）
3. 阅读 Python / C++ 实现
4. 按目标语言规范生成代码
5. 自检：
   - 命名是否符合本节规则
   - 序列化 tag 是否正确
   - 边界条件是否全覆盖

## 八、AI 回复风格要求

- 明确指出目标语言所属层级（探索 / 生产 / 生态）
- 若发现 Python / C++ / Rust / Go 行为不一致，立即指出
- **不擅自改变架构约定**，即使"看起来更优雅"

## 九、版本与发布规则（AI 禁止触碰）

- 版本号由 Git Tag（`vMAJOR.MINOR.PATCH`）唯一决定。
- Rust / Python / Go / C++ 四种语言必须共享同一版本号，不可各自独立。
- `autochangelog` 是唯一合法的版本发布入口。
- **AI Agent 严禁以下行为**：
  - ❌ 修改任何文件中的版本号字符串
  - ❌ 创建、修改或删除 Git Tag
  - ❌ 调用 `autochangelog` 或任何发布脚本
  - ❌ 在代码或提交信息中自行推断版本号
  - ❌ 修改 `CHANGELOG.md`、`pyproject.toml`、`Cargo.toml` 中的 `version` 字段

## 十、一句话总结（给 AI 的记忆锚点）

> Quant1X 是多语言语义同构系统，Python 是 Spec，C++ 是 Truth，Rust / Go 是对齐实现，I/O 边界死守 `snake_case`。
