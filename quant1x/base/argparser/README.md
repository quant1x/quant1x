# argparser：命令行参数解析（Python `argparse` 语义 + `clap` 交互）

做参数定义与解析：选项 / 位置参数、数量形态、取值动作与类型转换、子命令、互斥与校验、帮助与错误版式，
以及由定义生成 bash / zsh / fish 的补全脚本。**不做** 命令分发与业务校验。

## 文件

| 文件 | 职责 |
| --- | --- |
| `spec.py` / `spec.hpp` / `spec.cpp` / `spec.rs` / `spec.go` / `spec.zig` | 参数本体：取值动作、数量形态、类型转换器、单个参数定义与命名空间 |
| `error.py` / `error.hpp` / `error.cpp` / `error.rs` / `error.go` / `error.zig` | 异常、错误类别、相似项建议（Jaro-Winkler） |
| `parser.py` / `parser.hpp` / `parser.cpp` / `parser.rs` / `parser.go` / `parser.zig` | 命令定义与解析引擎：选项匹配、位置参数分配、子命令、校验 |
| `help.py` / `help.hpp` / `help.cpp` / `help.rs` / `help.go` / `help.zig` | 版式：usage 行、分节帮助、错误块、ANSI 着色 |
| `complete.py` / `complete.hpp` / `complete.cpp` / `complete.rs` / `complete.go` / `complete.zig` | 补全：由命令定义生成 bash / zsh / fish 脚本 |
| `derive.py` | 派生式声明：dataclass 字段与注解直接派生成参数定义（仅 Python 侧） |
| `__init__.py` | Python 包门脸：只做曝光 |
| `parser_test.py` / `help_test.py` / `complete_test.py` / `derive_test.py` | Python 用例（有断言、失败非零退出码） |
| `parser_test.go` / `help_test.go` / `complete_test.go` | Go 用例（同包） |
| `parser_test.zig` / `help_test.zig` / `complete_test.zig` | Zig 用例（`zig test` 直接跑，无 `build.zig`） |
| `tests/test-argparser-parser.cpp` / `tests/test-argparser-complete.cpp` | C++ 用例（仓库级 `tests/`，有断言、失败非零退出码） |
| `argparser.md` | 模块文档：解析口径、功能清单、依赖、测试、TODO |

Rust 用例内联在各 `.rs` 的 `#[cfg(test)] mod tests`。

## 用法（Python）

```python
from quant1x.base.argparser import Parser

parser = Parser("demo", about="演示命令行", version="1.0.0")
parser.add_argument("-v", "--verbose", action="count", help="输出更详细")
parser.add_argument("-o", "--output", required=True, help="输出文件")
parser.add_argument("files", nargs="+", help="输入文件")

namespace = parser.parse_args()   # 非法输入抛 UsageError，不打印也不退出
commands = parser.add_subparsers(dest="command")
build = commands.add_parser("build", about="构建")
build.add_argument("-j", "--jobs", type=int, default=4, help="并发数")
```

`parse_args` / `parse_known_args` 只抛异常；打印与退出码由 `parse_or_exit` 承担（帮助与版本退 0，用法错误退 2）。
完整口径见 [argparser.md](argparser.md) §2。

## 用法（C++ / Rust / Go / Zig）

四侧的入口与 Python 一一对位，只按各语言惯例改名：

- C++：`#include <quant1x/base/argparser/parser.hpp>`，`argparser::Parser`，`add_argument` / `add_subparsers`；
- Rust：`use quant1x::argparser::parser::Parser`，`Parser::new(prog)`，`add_argument` / `add_subparsers`；
- Go：包 `github.com/quant1x/quant1x/quant1x/base/argparser`，`NewParser(prog)`，`AddArgument` / `AddSubparsers`；
- Zig：`@import("parser.zig")`，`Parser.init(allocator, prog)`，`add_argument` / `add_subparsers`。

符号级对照见 [argparser.md](argparser.md) §2 的功能表与命名差异小节。

## 运行用例

```bash
python -m quant1x.base.argparser.parser_test
python -m quant1x.base.argparser.help_test
python -m quant1x.base.argparser.complete_test
python -m quant1x.base.argparser.derive_test
go test ./quant1x/base/argparser
cargo test base::argparser
zig test quant1x/base/argparser/parser_test.zig
zig test quant1x/base/argparser/help_test.zig
zig test quant1x/base/argparser/complete_test.zig
cmake --build build-msvc --config Release --target app-test-argparser-parser app-test-argparser-complete
build-msvc/tests/Release/app-test-argparser-parser.exe
build-msvc/tests/Release/app-test-argparser-complete.exe
```
