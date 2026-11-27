# AGENTS.md — AI 助手入口

> **本仓库的规范源是 [CONTRIBUTING.md](CONTRIBUTING.md)**：规则、约定与判据全部在那里。
> 本文件只做三件事 —— 告诉 AI 该读什么、哪些线不能踩、改完怎么验。**与 CONTRIBUTING.md 冲突时以
> 后者为准**；本文件不复制细则，只给指针。构建与依赖见 [BUILDING.md](BUILDING.md)。

## 1. 开工前必读

按顺序读，读完再动手：

| 顺序 | 读什么 | 为什么 |
| --- | --- | --- |
| 1 | [CONTRIBUTING.md](CONTRIBUTING.md) | 规范全集：语言角色、目录布局、命名、注释、文档、测试、提交、工作准则 |
| 2 | 目标模块的 `README.md` 与同名 `.md`（如 `quant1x/data/schema/bar.md`） | 该模块的口径、数据契约、跨语言对照、已知缺口（§5 TODO） |
| 3 | **该语义的 Python 实现（spec）与 Python 用例** | 语义以此为准；不要拿其它语言的既有实现反推 spec |
| 4 | 受影响的构建清单（`CMakeLists.txt` / `tests/CMakeLists.txt` / `benches/CMakeLists.txt`） | 新增源文件必须显式登记，见 BUILDING.md §2.10 |

## 2. 硬约束

| # | 约束 | 细则 |
| --- | --- | --- |
| 1 | **Python 是 spec**，语义、API、默认行为、失败约定一律以它为准；C++ 是生产实现，Rust / Go 是对齐实现，Zig 为扩展（当前只承载小功能），Java 为可选对照 | [§2.3](CONTRIBUTING.md#23-语言角色与演进顺序) |
| 2 | **不自己造东西**：不造 Python 里没有的结构 / 函数名 / 逻辑；需要新能力先补 Python spec，再推广 | [§4.1](CONTRIBUTING.md#41-总纲snake_case-优先) |
| 3 | **读 spec 代码，不读注释猜**；怀疑 spec 有错时先跑 Python 验证，再与用户商定是否改 spec | [§2.3](CONTRIBUTING.md#23-语言角色与演进顺序) |
| 4 | **类型名查词表**：`Bar` / `Transaction` / `Sector` … 不得即兴发明，不得用拼音音译 | [§4.3](CONTRIBUTING.md#43-领域类型词表type-vocabulary) |
| 5 | **注释只说"是什么 / 怎么做 / 为什么这样"**，不写"本轮改了啥、上一版是什么" | [§5.1](CONTRIBUTING.md#51-注释只说是什么怎么做) |
| 6 | **通用能力先查 `quant1x/base/`**，不写私有副本；缺就补 base | [§5.2](CONTRIBUTING.md#52-通用能力优先落-base) |
| 7 | **不擅自加开关**；调试开关一律 `debug_` 前缀 | [§5.3](CONTRIBUTING.md#53-调试开关) |
| 8 | **写盘必须幂等**：同一批输入跑多遍，产物一致 | [§5.4](CONTRIBUTING.md#54-重复触发与幂等) |
| 9 | **文档与代码同批**：新目录带 `README.md`，跨语言模块带同名 `.md` | [§6](CONTRIBUTING.md#6-文档规范) |
| 10 | **编码与行尾**：UTF-8 无 BOM、LF、末尾一个换行（`.bat` / `.cmd` 例外为 GBK + CRLF） | [§5.8](CONTRIBUTING.md#58-文件编码与行尾) |
| 11 | **不擅自 `git commit` / `push` / `rebase`**；提交信息按 `type[language]：summary` 模板，Windows 下用 `-F` 传文件 | [§8](CONTRIBUTING.md#8-git-提交规范) |
| 12 | **对照表列序固定** `python → c++ → rust → go → zig → java`；前四门必须实现，`zig`、`java` 为扩展（缺失不影响状态） | [§2.3](CONTRIBUTING.md#23-语言角色与演进顺序) |
| 13 | **`temp/` 才是垃圾站**：一次性脚本、中间产物、草稿落 `temp/`，层级复刻源码结构（不许扁平堆在根），不入库、文档与提交信息也不引用其路径 | [§3.6](CONTRIBUTING.md#36-temp仓库的垃圾站) |
| 14 | **只在项目目录内写文件**：临时文件、导出物、调试脚本一律落 `temp/`；只有用户明确要求时才可写到项目外，并在回复里点名路径 | [§3.6](CONTRIBUTING.md#36-temp仓库的垃圾站) |
| 15 | **Go 包名铁律**：`package <目录名>`；Java 测试包名 = `tests/` 下目录层级（不多 `quant1x.`、不省 `tests.`） | [§3.3](CONTRIBUTING.md#33-go-的目录与包名)、[§3.4](CONTRIBUTING.md#34-java-测试包的层级) |
| 16 | **先建单**：每项功能或任务先用仓库自带 `labs-git-issue` 建 Issue（不传 `--host` / `--repo`，平台与仓库按 `origin` 自动识别；背景 / 目标与范围 / 验收 / 涉及语言 / 风险），完成后关单 | [§9.1](CONTRIBUTING.md#91-先建单再动手) |
| 17 | **版本号与 tag 一律不碰**：不得改版本字符串、不建 tag、不调 `autochangelog`、不手工改 `CHANGELOG.md` | [§10](CONTRIBUTING.md#10-版本与发布) |

## 3. 主题索引

| 我要… | 去哪 |
| --- | --- |
| 查语言最低版本 / 编译器基线 | [§2.1](CONTRIBUTING.md#21-最低版本)、[§2.2](CONTRIBUTING.md#22-c-编译器基线) |
| 判断"同构 / 异构"这类术语怎么用（语义 vs 实现） | [§2.4](CONTRIBUTING.md#24-术语语义与实现) |
| 决定文件放哪、叫什么 | [§3](CONTRIBUTING.md#3-目录与多语言布局) |
| 查跨语言导入 / 包含路径怎么对位 | [§3.1](CONTRIBUTING.md#31-仓库根与实现根) |
| 临时脚本 / 一次性复现 / 中间产物放哪 | [§3.6](CONTRIBUTING.md#36-temp仓库的垃圾站) |
| 定函数名 / 命名空间 / 头文件守卫宏 / 平台宏 | [§4](CONTRIBUTING.md#4-命名与数据契约) |
| 查领域类型标准名（`Bar` / `Transaction` / `Sector` …） | [§4.3](CONTRIBUTING.md#43-领域类型词表type-vocabulary) |
| 定整型 / 浮点的跨语言名与宽度 | [§4.6](CONTRIBUTING.md#46-标量类型) |
| 定落盘字段名 / Go struct tag | [§4.7](CONTRIBUTING.md#47-边界态io-与序列化铁律) |
| 写注释、判断该不该写某句 | [§5.1](CONTRIBUTING.md#51-注释只说是什么怎么做) |
| 找某个通用能力该不该落 `base` | [§5.2](CONTRIBUTING.md#52-通用能力优先落-base) |
| 处理重复触发 / 幂等 | [§5.4](CONTRIBUTING.md#54-重复触发与幂等) |
| 跑 Python、写 import | [§5.5](CONTRIBUTING.md#55-python-包形态与导入) |
| 写 `#include` | [§5.6](CONTRIBUTING.md#56-c-头文件引用) |
| 处理字符串 | [§5.7](CONTRIBUTING.md#57-字符串与通用工具复用) |
| 遇到缺依赖 / spec 未定 | [§5.9](CONTRIBUTING.md#59-卡点先抛不支持)、[§5.10](CONTRIBUTING.md#510-抽象接口留给实现方的方法必须抛异常) |
| 写模块文档 / 判"算不算完成" | [§6](CONTRIBUTING.md#6-文档规范) |
| 写测试 | [§7](CONTRIBUTING.md#7-测试规范) |
| 写提交信息 | [§8.1](CONTRIBUTING.md#81-提交信息) |
| 判断要不要立提案 | [§9.2](CONTRIBUTING.md#92-提案rfc变更受理与裁决) |
| 定工作方式（假设 / 简洁 / 精准 / 目标驱动） | [§11](CONTRIBUTING.md#11-工作准则) |
| 构建 / 依赖 / 发布 | [BUILDING.md](BUILDING.md) |

## 4. 常用命令

| 目的 | 命令 |
| --- | --- |
| C++ 配置与构建（Windows） | `scripts\msvc_configure.bat`，或见 [BUILDING.md](BUILDING.md) §2.4 |
| C++ 构建 | `cmake --build build-msvc --config Release -j` |
| C++ 跑用例 | `ctest --test-dir build-msvc -C Release --output-on-failure -j 8`（按名过滤加 `-R <名称>`） |
| Rust 构建 / 用例 | `cargo build --bin q1x`、`cargo test`（ringbuffer 相关需 `--features vyukov`） |
| Rust 基准 | `cargo bench --bench vyukov_bench` |
| Go 编译 / 用例 / 检查 | `go build ./...`、`go test ./...`、`go vet ./...` |
| Zig 用例 | `zig test quant1x/base/argparser/parser_test.zig`（同目录另有 help / complete 用例）；`zig test quant1x/num/num_test.zig`、`zig test quant1x/num/ndarray_test.zig`（无 `build.zig`） |
| Python 可编辑安装 | `pip install -e .` |
| Python 跑用例 | `python -m pytest tests -q`，或模块级 `python -m unittest quant1x.<模块>.<子模块>_test` |
| Java 用例 | `mvnw.cmd -B test`（Linux / macOS 用 `./mvnw -B test`） |
| 编码自查（BOM / CRLF） | [§5.8](CONTRIBUTING.md#58-文件编码与行尾) 的 PowerShell 片段 |
| 提交（Windows） | 写入 `.git/commit-msg.tmp` 后 `git commit -F .git/commit-msg.tmp` |
| 建 / 查 / 关工单 | `labs-git-issue`（不传 `--host` / `--repo`，按 `origin` 自动识别），正文模板见 [§9.1](CONTRIBUTING.md#91-先建单再动手) |

## 5. 交付前自检

1. **语义**：与 Python spec 逐值对拍过（不是"看起来一样"）；有差异先改实现，或与用户商定改 spec。
2. **构建与用例**：相关语言的编译通过、相关用例通过；跳过项（在线 / 依赖本地数据）要能说明原因。
3. **一致性**：新增 / 修改的接口在五种语言（python / c++ / rust / go / zig）里命名对位；落盘字段名全小写蛇形；Go tag 齐全。
4. **注释**：没有"本轮 / 上一版 / 原来 / 曾经"这类变更叙事（[§5.1](CONTRIBUTING.md#51-注释只说是什么怎么做)）。
5. **文档**：目录 `README.md`、模块 `.md` 的路线图 / 功能表 / 测试 / TODO 与实际一致，无陈旧条目。
6. **编码**：UTF-8 无 BOM、LF、末尾换行（[§5.8](CONTRIBUTING.md#58-文件编码与行尾)）。
7. **提交信息**：符合 `type[language]：summary`，正文分条含验证命令，无版本号 / 代理署名。
8. **工单**：这轮任务有对应 Issue，状态与正文和实际落地一致（[§9.1](CONTRIBUTING.md#91-先建单再动手)）。
