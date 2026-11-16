# encoding：编解码（字符集、CSV、YAML、JSON、Base64、二进制打包）

做**外部表示 ↔ 内存结构**的转换：字符集转码、CSV / YAML / JSON 的解析与序列化、Base64 编解码、定长二进制
打包。**不做** 业务字段校验、配置加载策略（谁读哪个文件由 `quant1x/config` 决定）、网络协议编解码（在
`quant1x/net` / `quant1x/proto`）。

## 文件与子目录

| 路径 | 职责 |
| --- | --- |
| `charsets.go` / `charsets.h` / `charsets.cpp` | 字符集转码：`GBKToUTF8`（Go）、对应 C++ 实现 |
| `csv.go` / `csv.h` | CSV ↔ 结构体切片；Go 侧 `CsvToSlices`，`csv` 作为结构体 tag 名 |
| `yaml.py` | Python 侧文件级 `load` / `dump` 与带缓存的 `ReadConfigFiles` |
| `yaml.h` | C++ 侧 `encoding::yaml`（Boost.PFR 结构体序列化）与 `encoding::safe_yaml`（显式默认值） |
| `yaml/` | Go 侧 YAML 包 `package yaml`：`Unmarshal`（按 `default` 标签补默认值）/ `Marshal` / `Node`，见 [yaml/README.md](yaml/README.md) |
| `json.h` / `json.cpp` / `json.md` | C++ 侧 JSON 序列化（PFR 反射 + nlohmann::json），口径见 [json.md](json.md) |
| `base64/` | Go 侧 Base64：字符表与随机串生成，`package base64` |
| `binary/cstruct` | 定长二进制结构体编解码（第三方移植，见其 `FORK.md` / `LICENSE`） |
| `binary/struc` | 按字段描述打包 / 解包（第三方移植，见其 `LICENSE`） |
| `__init__.py` | Python 包门脸 |

## 上下游

- 上游：`gopkg.in/yaml.v3`、`github.com/gocarina/gocsv`、`golang.org/x/text`（Go）；`yaml-cpp`、`nlohmann::json`、
  `boost::pfr`（C++）；`PyYAML`（Python）。
- 下游：`quant1x/config`、`quant1x/core`（经 `yaml` 子包）；`quant1x/contrib/data/tdx` 直接使用 `gopkg.in/yaml.v3`。
- 边界：本目录只做格式转换，不持有全局状态、不做缓存策略。

## 跨语言对应

| 语义 | python | c++ | rust | go | zig |
| --- | --- | --- | --- | --- | --- |
| 字符集转码 | — | `charsets.cpp` | — | `charsets.go` | — |
| CSV | — | `csv.h` | — | `csv.go` | — |
| YAML | `yaml.py` | `yaml.h` | — | `yaml/yaml.go` | — |
| JSON | — | `json.h` / `json.cpp` | — | — | — |
| Base64 | — | — | — | `base64/base64.go` | — |
| 定长二进制打包 | — | — | — | `binary/struc`、`binary/cstruct` | — |
