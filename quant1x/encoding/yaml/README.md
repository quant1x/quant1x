# encoding/yaml：YAML 编解码（Go 侧带 `default` 标签默认值）

在 `gopkg.in/yaml.v3` 之上包一层 `Unmarshal` / `Marshal` / `Node`，让结构体 tag 里的 `default` 在 YAML
缺省时生效。**不做** 文件读写、schema 校验、JSON ↔ YAML 互转。

## 文件

| 文件 | 职责 |
| --- | --- |
| `yaml.go` | 包本体：`Unmarshal`（默认值填充）、`Marshal`、`Node` 别名 |
| `yaml_test.go` | 同包用例：默认值口径、显式零值、嵌套 / 切片 / map、空文档、非法输出、`TextUnmarshaler`、`Node` 别名、往返 |
| `yaml.md` | 模块文档：默认值口径、功能清单、依赖、测试、TODO |

## 上下游

- 上游：`gopkg.in/yaml.v3`（唯一外部依赖）。
- 下游：`quant1x/config`（配置文件解析）、`quant1x/core`（`BaseConfig` 与 `DecodeTo`）；`quant1x/config`
  同时使用 `quant1x/core`。
- 兄弟包：`quant1x/encoding`（`GBKToUTF8` / `CsvToSlices`）与本包互不依赖；本包不引用仓内其它模块，
  因此不存在导入环。

## 跨语言对应

| 语言 | 文件 | 说明 |
| --- | --- | --- |
| python | `quant1x/encoding/yaml.py` | `load` / `dump` / 带缓存的 `ReadConfigFiles`，文件级，无 `default` 标签 |
| c++ | `quant1x/encoding/yaml.h` | `encoding::yaml`（PFR 结构体序列化）与 `encoding::safe_yaml`（调用方显式传默认值） |
| rust | — | 未落地 |
| go | `quant1x/encoding/yaml/yaml.go` | 本包；`default` 标签驱动默认值 |
| zig | — | 未落地 |
| java | — | 未落地 |

口径与差异见 [yaml.md](yaml.md)。
