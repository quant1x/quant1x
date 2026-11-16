# encoding::yaml：YAML 编解码（Go 侧按结构体 tag 填默认值）

做 YAML 文本与结构体之间的编解码：反序列化时，YAML 里**没有出现**的字段按结构体 tag 的 `default` 取值。
**不做** 文件读写、schema 校验、注释保留、JSON ↔ YAML 互转。

## 1. 路线图

**当前位置与下一步**：Go 侧已落地（`Unmarshal` / `Marshal` / `Node`，8 条用例全绿）；Python 侧只有文件级
`load` / `dump`，C++ 侧是 PFR 驱动的结构体序列化，默认值由调用方显式传入。三侧 API 形态不同，没有同一签名，
因此**跨语言一致性只落在「YAML 文本 ↔ 结构体」这一层**，`default` 标签是 Go 侧专属能力。下一步要么在 Python
定案默认值口径再推广，要么明确它只属于 Go 侧（见 §5）。

| 阶段 | 语言（角色） | 载体 / 文件 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| spec | Python（spec） | `quant1x/encoding/yaml.py` | 部分落地 | 文件级 `load` / `dump` / `ReadConfigFiles`，无默认值机制 |
| 生产实现 | C++（生产） | `quant1x/encoding/yaml.h` | 部分落地 | `encoding::yaml` 用 Boost.PFR 做结构体序列化；`encoding::safe_yaml::parse_field` 由调用方显式传默认值，不读 tag |
| 对等实现 | Rust（对等） | — | 未落地 | — |
| 对等实现 | Go（对等） | `quant1x/encoding/yaml/yaml.go` | 已落地 | 本包；`default` 标签驱动默认值 |
| 扩展 | Zig（扩展） | — | 未落地 | — |
| 扩展 | Java（扩展） | — | 未落地 | — |

本包实现内联自 `github.com/quant1x/gox` v1.27.1 的 `encoding/yaml`（Apache-2.0，版权归 Quant1X Team），
语义与之一致。

## 2. 功能清单

**统一口径（Go 侧对外行为）**：

1. **默认值三条**：YAML 中出现的字段以 YAML 的值为准（含显式零值 `0` / `false` / 空串、显式 `null`）；
   YAML 中没有出现的字段取 `default` 标签的值；嵌套结构、切片元素、map 的值逐层按这两条处理。
2. **默认值语法按字段类型**：标量走 `strconv`（`bool` / 整数 / 浮点 / 字符串），`int64` 先试
   `time.ParseDuration` 再试整数，故 `time.Duration` 可写 `default:"5s"`；切片 / map / 结构体按 **JSON**
   语法解析默认值（`default:"[\"sh68\",\"bj\"]"`、`default:"{\"k\":\"v\"}"`），解析失败返回错误。
3. **接口优先**：字段指针实现 `encoding.TextUnmarshaler` 或 `json.Unmarshaler` 时，默认值先交给该接口。
4. **跳过项**：未导出字段与 `yaml:"-"` 字段不参与默认值填充；内嵌结构体的字段在 YAML 中是平铺的。
5. **失败与边界**：空文档（空串、只有注释）解析为 no-op，不改写目标；输出参数不是指针或为 nil 指针时返回
   `yaml: out must be a non-nil pointer`；YAML 语法错误与类型不匹配由 `gopkg.in/yaml.v3` 报出，文案不包装。
6. **`Marshal` 与 `Node`**：`Marshal` 直接透传 `yaml.v3`，不额外裁剪零值；`Node` 是 `yaml.Node` 的类型别名，
   类型实现 `UnmarshalYAML(*yaml.Node)` 时按 `yaml.v3` 的语义被调用。
7. **单位与精度**：本模块不做数值换算，浮点原样交给 `yaml.v3`；`time.Duration` 以纳秒为单位落在 `int64` 上。
8. **平台差异**：无。

| 功能 | python | c++ | rust | go | zig | java | 状态 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| YAML 文本 → 结构体（tag 默认值） | — | `safe_yaml::parse_field`（默认值显式传入） | — | `Unmarshal` | — | — | 部分落地（仅 Go / C++） |
| 结构体 → YAML 文本 | — | `yaml::serialize_to_node` | — | `Marshal` | — | — | 部分落地（仅 Go / C++） |
| 文件级 `load` / `dump` | `load` / `dump` | — | — | — | — | — | 部分落地（仅 Python） |
| 节点类型透出（`UnmarshalYAML` 用） | — | `YAML::Node` | — | `Node` | — | — | 部分落地（仅 Go / C++） |

## 3. 依赖

出向：

| 依赖 | 用途 | 为什么必须是它 |
| --- | --- | --- |
| `gopkg.in/yaml.v3` | 词法 / 语法解析、`Node` 与 `Decode` | 默认值填充是**在解析后的节点树上**做二次补齐，需要 `yaml.Node` 的 Content 结构与 `Decode` 行为；换用其它 YAML 库会同时改变节点语义与错误文案 |

入向：

| 使用方 | 用途 |
| --- | --- |
| `quant1x/config` | 解析配置文件到 `TraderParameter` / `RuleParameter` / `StrategyParameter`（54 处 `default` 标签） |
| `quant1x/core` | `BaseConfig` 解析与 `DecodeTo`（动态 map 转强类型结构体） |

无反向约束：本包不引用仓内其它模块（依赖方向单向，`config → core → encoding/yaml`）。

## 4. 测试

| 项 | 结论 |
| --- | --- |
| 单元测试 | `yaml_test.go`，8 条：默认值全类型、显式零值优先、嵌套 / 切片 / map 逐层补齐、空文档、非法输出、`TextUnmarshaler` 默认值、`Node` 别名与 `UnmarshalYAML`、`Marshal` 往返 |
| 覆盖率 | 未设阈值；`applyStructDefaults` / `applySliceDefaults` / `applyMapDefaults` 与标量分支均有用例覆盖 |
| 重复度 | 与 `gox/encoding/yaml` 同源，仓内无第二份实现 |
| 静态检查 | `go vet ./quant1x/encoding/yaml/` |
| 基准测试 | 本模块不涉及：纯反射封装，无性能承诺 |
| 崩溃注入 / 并发 | 本模块不涉及：不落盘、无共享状态，纯内存纯函数 |
| 契约一致性 | 无跨语言对拍：`default` 语义只在 Go 侧存在（§5） |

运行：`go test ./quant1x/encoding/yaml/ -v`。

## 5. TODO

1. **`default` 口径没有 Python spec**：Python `yaml.py` 不读结构体 tag，C++ 由调用方显式传默认值。若要求默认值
   语义跨语言一致，先在 Python 定案（字段缺失 → tag 默认值的三条规则），再推广到 Rust / Zig。
2. **Rust / Zig 无实现**：本模块在两个必做扩展语言上没有对应包。
3. **`gox` 仍在模块图中**：本仓代码不再 import `gox/encoding/yaml`；它经 `quant1x/num` → `gox/logger` /
   `gox/exception` 留在模块图里。
