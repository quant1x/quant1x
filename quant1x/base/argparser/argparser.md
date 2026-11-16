# argparser：借鉴 Python argparse 与 Rust clap 实现的命令行参数解析器

做参数定义与解析：选项 / 位置参数、数量形态、取值动作与类型转换、子命令、互斥与校验、帮助与错误版式，
以及由定义生成 bash / zsh / fish 的补全脚本。
**不做** 命令分发与业务校验。

## 1. 路线图

**当前位置与下一步**：Python spec 已落地 —— 取值形态、十种动作、校验、子命令、`@file` 取参、覆盖、帮助宽度
与版式、错误类别、补全生成、派生式声明都已铺开，用例 274 条全绿（§4.1）。C++ / Rust / Go / Zig 四侧都已落
`spec` + `error` + `parser` + `help` 四层，用例分别 40 / 61 / 40 / 30 条且全绿；四侧的短帮助 / 长帮助 /
子命令帮助 / 窄幅折行 / 错误块五个样本都与 Python 输出字节一致（§4.6）。五侧的入口与退出码也已齐
（`parse_or_exit` / `print_help`）；五侧的 `@file` 取参也已齐（Zig 因 0.17 起文件读写要传 `Io`，前缀与 `Io`
一起登记）。补全生成五侧齐（bash / zsh / fish 三种脚本，逐字节一致）。派生式声明落在 Python 侧（dataclass
注解到参数定义），其余四侧没有运行期注解，不跟进；§5 当前无待办项。

| 阶段 | 语言（角色） | 载体 / 文件 | 状态 | 说明 |
| --- | --- | --- | --- | --- |
| spec 定语义 | Python（spec） | `quant1x/base/argparser/spec.py` / `parser.py` / `help.py` / `error.py` / `complete.py` / `derive.py` | 已落地 | 由它确定语义、API 与失败口径；`main.py` 已接入 |
| 生产实现 | C++（生产） | `quant1x/base/argparser/spec.hpp` / `spec.cpp` / `error.hpp` / `error.cpp` / `parser.hpp` / `parser.cpp` / `help.hpp` / `help.cpp` / `complete.hpp` / `complete.cpp` | 已落地 | 四层与入口齐，另加补全生成；源文件登记在根 `CMakeLists.txt`；`main.cpp` 已接入 |
| 对等实现 | Rust（对等） | `quant1x/base/argparser/spec.rs` / `error.rs` / `parser.rs` / `help.rs` / `complete.rs` | 已落地 | 四层与入口齐（`parse_or_exit` / `print_help`），另加补全生成，与 Python spec 同口径；`main.rs` 已接入 |
| 对等实现 | Go（对等） | `quant1x/base/argparser/spec.go` / `error.go` / `parser.go` / `help.go` / `complete.go` | 已落地 | 四层与入口齐，另加补全生成；包 `quant1x/base/argparser`，已被 `quant1x/command`（`q1x-go` 命令行）接入 |
| 扩展 | Zig（扩展 / 小功能） | `quant1x/base/argparser/spec.zig` / `error.zig` / `parser.zig` / `help.zig` / `complete.zig` | 已落地 | 四层与入口齐，另加补全生成；无 `build.zig`，用例直接 `zig test` |

## 2. 功能清单

**统一口径（成稿必须保留本段）**：

1. **扫描是单趟左到右**：选项就地取走它的取值，其余 token 攒起来交给位置参数；命中子命令名后其余 token 整段
   交给子命令，此前攒下的 token 仍归本命令。子命令名只在本命令位置参数的最小需求已满足时才当子命令。
2. **token 形态**：`--key`、`--key=value`、`-k`、`-kvalue`、`-k=value` 等价；`-abc` 按短选项聚合拆开（首个
   短选项必须是不吃取值的开关，碰到吃取值的那个短选项时，余下字符就是它的取值）；`--` 之后全是位置参数；单个 `-`
   是位置参数；签名数字（可选 `+` / `-` 号加整数或小数，如 `-1`、`-.5`、`+3`、`-1.5`）整个 token 当作一个取值
   参数，既不参与短选项聚合，也不拆成「短选项 + 粘值」，因此签名数字不能作选项名。
3. **长选项前缀**：`allow_abbrev=True` 时唯一前缀可用，命中多个报歧义并列候选；关掉后只认全名。
4. **数量形态与落值**：`nargs` 取 `None`（一个）、整数 N、`?` / `*` / `+`、`REMAINDER`；整数、`*` / `+` /
   `REMAINDER` 或配了 `value_delimiter` 时落值恒为列表，其余为单值；`nargs="?"` 裸写取 `const`（无 `const`
   则 `None`）。变长位置参数先给后面的位置参数留够最小值，`REMAINDER` 必须是最后一个且一旦轮到自己就把余下
   token 全收下（含看起来像选项的）。
5. **缺省值优先级**：命令行 > 环境变量（`env`，命中即算「出现过」）> `default` > `argument_default` >
   动作的隐式缺省（`store` → `None`、`count` → 0、`store_true` → `False`、`store_false` → `True`）；
   `SUPPRESS` 表示不写进命名空间。默认值不算「出现过」，不满足必填。
6. **校验顺序**：`conflicts_with` → 互斥组（多于一个 / 必选组为空）→ 多值组（必选组为空）→ `requires` →
   必填（选项与位置参数），报第一处问题，不汇总；五侧同一顺序。
7. **返回值**：解析成功返回 `Namespace`（属性与下标两种读法，`as_dict()` 取普通字典）；`parse_known_args`
   额外返回认不出的 token 列表，按命令行顺序排列。未知选项只收该 token 本身、不连带吞掉其后的 token
   （那些 token 照常按位置参数分配，与 `argparse` 一致）；`parse_args` 仍严格报错并保留相似项建议。
8. **失败约定**：两个解析入口只抛异常、不打印不退出 —— 用法错误抛 `UsageError`（`kind` 取 `ErrorKind`
   之一：未知参数 / 前缀歧义 / 缺取值 / 取值非法 / 缺必填 / 冲突），`-h` / `--help` 抛 `HelpRequested(long=...)`，
   `-V` / `--version` 抛 `VersionRequested`。打印与退出码只由 `parse_or_exit` 承担：帮助与版本写 stdout 退 0，
   用法错误写 stderr 退 2。
9. **单位与精度**：本模块不涉数值计算，无单位与精度口径；数值类型只经 `type=` 转换器落地，精度由调用方给的
   转换器决定。
10. **文案语言**：错误正文与 usage 为英文且与 `clap` 同形（`error:` / `tip:` / `For more information, try
    '--help'.`），跨语言对拍与用例断言都按这套；`about` / `help` 等用户文字由调用方给，不做翻译层。
11. **平台差异**：只有两处且不改语义 —— 默认 `prog` 取 `sys.argv[0]` 的文件名（路径分隔符由
    `os.path.basename` 吸收）；`color="auto"` 依目标流是否终端与 `NO_COLOR` 判定着色。帮助宽度默认固定
    `max_width=100`、不读 `COLUMNS`，故各平台版式一致；只有登记了 `auto_width` 才按 `COLUMNS` 取（去掉首尾
    空白后须为正整数，否则退回 `max_width`）。
12. **自动 `help` 子命令**：登记子命令后自动生成 `help` —— `prog help` 打印本命令长帮助，`prog help a b`
    沿子命令名逐级下沉、打印最深一层的长帮助；走 `HelpRequested` 控制流，不写命名空间，也不进
    `SubParsers.commands()`，只在帮助的 `Commands` 节末行占一行。本命令位置参数尚未够填时 `help` 仍按普通
    位置 token 处理；用户自己登记了 `help` 子命令时以用户的为准、不生成。指向未登记的名字报 `UsageError`
    （`unrecognized subcommand '<名字>'`）。
13. **`@file` 取参**：登记前缀字符集后（Python `fromfile_prefix_chars`），token 首字符命中集合即展开成文件里
    的行 —— 每行原样作一个 token（不拆词、不去空白、不认注释），按 LF 拆行并去掉行尾 CR，末尾换行不多出
    一个空 token；展开结果里再出现的前缀继续展开，嵌套超过 16 层按用法错误报（挡住自引用），打不开文件
    同样按用法错误报（`ErrorKind.OTHER`，文案 `unable to open file '<路径>'`）。展开只在顶层做一次，
    子命令沿用展开后的 token，与 `argparse` 一致。
14. **覆盖**：`overrides_with` 双向生效 —— 一对里命令行上后出现的留下，先出现的从命名空间摘掉、按默认值
    重填；只认命令行上的出现顺序，环境变量与默认值都不参与。收尾顺序固定为 覆盖 → 环境变量 → 校验 →
    默认值，所以被覆盖的参数不会带着取值参与互斥 / 依赖判定。
15. **必选组与条件必填**：`add_value_group` 登记的组内参数可以同时出现，`required` 为真时至少出现一个，缺了
    报在 `<组名>` 那一行（`the following required arguments were not provided:`）；`required_unless_present`
    列出的参数里只要有一个出现过，本参数就不算必填。判据只看校验时刻的「出现过」集合，而覆盖发生在它之前，
    因此被 `overrides_with` 覆盖掉的参数不再算出现过，依赖它的条件必填回退为必填。
16. **补全生成**：`complete(parser, shell, prog=None)` 输出 bash / zsh / fish 的脚本文本，`shell` 只认这三种、
    其余抛 `ValueError`；`prog` 缺省取 `parser.prog`。候选词按「已敲定的子命令链」给，顺序为 选项名（`hide`
    的不列，含自动的 `-h` / `--help`；子命令里再追加祖先的 `global_` 选项）→ 可见子命令名 → 位置参数的
    `choices`，去重保序，含空白的词跳过；不区分「正在写取值」的语境，也不补文件路径。bash / zsh 走子命令链
    状态机（`case "$cmd|$word"`），fish 改用 `__fish_seen_subcommand_from` 条件。函数名取 `_argparser_<prog>`
    （非标识符字符换成下划线），换行一律 LF、脚本以单个 LF 收尾。
17. **派生式声明**：`from_dataclass(cls, prog=None, **parser_kwargs)` 按 dataclass 的字段顺序与类型注解建一条
    命令，`add_arguments(parser, cls)` 把同一批定义挂到已有命令上（子命令场景），返回登记到的参数定义。词表：

    | 注解 / 字段形态 | 派生结果 |
    | --- | --- |
    | `str` | 不设 `type` |
    | `int` / `float` | `type=parse_int()` / `type=parse_float()` |
    | `bool` | `action="store_true"`；默认值为真时 `action="store_false"`；不吃取值 |
    | `X \| None` | `nargs="?"`，内层按 `X` 取转换器 |
    | `list[X]` / `set[X]` / `frozenset[X]` / `tuple[X, ...]` | `nargs="*"`，元素按 `X` 取转换器 |
    | `Literal[...]` / `Enum` 子类 | 取值进 `choices`，转换器按取值的共同类型推 |
    | 其余注解 | 定义期 `ValueError`（不静默退回字符串） |

    字段名去下划线加 `--` 成选项名，`dest` 沿用字段名；位置参数由 `field(metadata={"positional": True})` 标出，
    名字即字段名。说明文字取 `metadata={"help": "..."}`。没有 `default` 也没有 `default_factory` 的字段是必填
    （位置参数除外）；`default_factory` 在定义时取一次值当默认值；`Enum` 默认值落成成员取值。非 dataclass 报
    `TypeError`；`X | None` 只接受恰好一个非 `None` 成员；布尔不能出现在可空或多值位置；`Literal` / `Enum` 的
    取值类型必须一致。

| 功能 | python（spec） | c++ | rust | go | zig | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| 选项与位置参数登记 | `Parser.add_argument`（别名 / 展示别名 / `hide`） | `Parser::add_argument` / `Argument` | `Parser::add_argument` / `Argument` | `Parser.AddArgument` / `Argument` | `Parser.add_argument` / `Argument` | 已落地 |
| 数量形态与落值 | `nargs` / `REMAINDER` / `value_delimiter` | `Nargs` / `Argument::collects` | `Nargs` / `Argument::collects` | `Nargs` / `Argument.Collects` | `Nargs` / `Argument.collects` | 已落地 |
| 取值动作 | `Action`（十种，兼容 clap 的 `SetTrue` 拼法） | `Action` | `Action` | `Action` | `Action` | 已落地 |
| 类型转换 | `ValueParser` / `parse_int` / `parse_float` / `parse_bool`；按 `default` 类型推断 | `ValueParser` / `parse_int` / `parse_float` / `parse_bool` | `ValueParser` / `parse_int` / `parse_float` / `parse_bool` | `ValueParser` / `ParseInt` / `ParseFloat` / `ParseBool` | `ValueParser` / `parse_int` / `parse_float` / `parse_bool_value` | 已落地 |
| 可选值 | `choices`（列表或 `{值: 说明}`） | `Choice` / `Argument::choice_values` | `Choices` / `Argument::choice_values` | `Choice` / `Argument.ChoiceValues` | `Choice` / `Argument.choice_values` | 已落地 |
| 缺省值与环境变量 | `default` / `argument_default` / `env` / `SUPPRESS` | `DefaultValue` / `Argument::env` | `DefaultValue` / `Argument::env` | `DefaultValue` / `Argument.Env` | `DefaultValue` / `Argument.env` | 已落地 |
| 校验 | `add_mutually_exclusive_group` / `conflicts_with` / `requires` / `required` | `Parser::add_mutually_exclusive_group` / `Argument::conflicts_with` / `Argument::requires_args` | `Parser::add_mutually_exclusive_group` / `Argument::conflicts_with` / `Argument::requires` | `Parser.AddMutuallyExclusiveGroup` / `Argument.ConflictsWith` / `Argument.Requires` | `Parser.add_mutually_exclusive_group` / `Argument.conflicts_with` / `Argument.requires_args` | 已落地 |
| 必选组与条件必填 | `add_value_group`（`required`）/ `required_unless_present` | `Parser::add_value_group` / `ValueGroup::add_argument` / `Argument::required_unless_present` | `Parser::add_value_group` / `Parser::add_argument_to_value_group` / `Argument::required_unless_present` | `Parser.AddValueGroup` / `ValueGroup.AddArgument` / `Argument.RequiredUnlessPresent` | `Parser.add_value_group` / `ValueGroup.add_argument` / `Argument.required_unless_present` | 已落地 |
| 子命令与全局参数 | `add_subparsers`（别名 / 必填 / `dest`）/ `global_` | `SubParsers` / `Argument::global` | `SubParsers` / `Argument::global` | `SubParsers` / `Argument.Global` | `SubParsers` / `Argument.global` | 已落地 |
| 自动 `help` 子命令 | `add_subparsers` 后自动生成，可被同名自定义子命令覆盖 | `Parser::help_target` | `Parser::help_target` | `helpSubcommandOf` / `helpTarget`（非导出） | `Parser.help_target` | 已落地 |
| 帮助与 usage 渲染 | `format_help(long=)` / `format_usage` / `print_help` / `add_group` | `help::render_help` / `render_usage` / `Parser::format_help` / `Argument::usage_spec` | `help::render_help` / `render_usage` / `Parser::format_help` / `Argument::usage_spec` | `help::render_help` / `render_usage` / `Parser::format_help` / `Argument::usage_spec` | `help.render_help` / `render_usage` / `Parser.format_help` / `Argument.usage_spec` | 已落地 |
| 帮助开关 | `add_help=False` | `Parser::without_help` | `Parser::without_help` | `Parser.WithoutHelp` | `Parser.without_help` | 已落地 |
| 错误与相似项建议 | `UsageError.kind` / `ErrorKind` / `best_match` | `UsageError` / `ErrorKind` / `best_match` | `UsageError` / `ErrorKind` / `best_match` | `UsageError` / `ErrorKind` / `BestMatch` | `UsageError` / `ErrorKind` / `best_match` | 已落地 |
| 解析入口与退出码 | `parse_args` / `parse_known_args` / `parse_or_exit` / `print_help` | `Parser::parse` / `Parser::parse_known` / `Parser::parse_or_exit` / `Parser::print_help` / `exit_code` | `Parser::parse` / `Parser::parse_known` / `Parser::parse_or_exit` / `Parser::print_help` / `ParseError::exit_code` | `Parser.Parse` / `Parser.ParseKnown` / `Parser.ParseOrExit` / `Parser.PrintHelp` / `ExitCode` | `Parser.parse` / `Parser.parse_known` / `Parser.parse_or_exit` / `Parser.print_help` / `Outcome.exit_code` | 已落地 |
| `@file` 取参 | `fromfile_prefix_chars` | `Parser::with_from_file_prefix_chars` | `Parser::with_from_file_prefix_chars` | `Parser.WithFromfilePrefixChars` | `Parser.with_fromfile_prefix_chars(chars, io)` | 已落地 |
| 覆盖 | `overrides_with` | `Argument::overrides_with` | `Argument::overrides_with` | `Argument.OverridesWith` | `Argument.overrides_with` | 已落地 |
| 帮助宽度 | `max_width` / `auto_width`（读 `COLUMNS`） | `Parser::with_max_width` / `with_auto_width` / `width` | `Parser::with_max_width` / `with_auto_width` / `width` | `Parser.WithMaxWidth` / `WithAutoWidth` / `Width` | `Parser.with_max_width` / `with_auto_width` / `width` | 已落地 |
| 着色 | `color`（`auto` / `always` / `never`）+ `NO_COLOR` | `ColorChoice` / `Parser::color_enabled` / `help::Style` / `render_error` | `ColorChoice` / `Parser::color_enabled` / `help::Style` / `help::render_error` | `ColorChoice` / `Parser.ColorEnabled` / `Style` / `RenderError` | `ColorChoice` / `Parser.color_enabled` / `Style` / `help.render_error` | 已落地 |

| 补全生成 | `complete(parser, shell, prog=None)` / `SHELLS` | `complete` / `SHELLS` | `complete` / `SHELLS` | `Complete` / `Shells` | `complete` / `SHELLS` | 已落地 |
| 派生式声明 | `from_dataclass` / `add_arguments` | — | — | — | — | 部分落地（python 已落地；其余四侧无运行期注解） |

**单元格记法**：单载体只写符号，文件路径在 §1 声明；同一语言多载体时写「符号 @ 文件」；空位一律写 `—`。

**Rust 侧的命名与类型对照**（语言机制导致的名字差异，语义不变；改口径时各侧一起改）：

| Python | Rust | 原因 |
| --- | --- | --- |
| `nargs=None` / `3` / `"?"` / `"*"` / `"+"` / `REMAINDER` | `Nargs::One` / `Count(3)` / `Optional` / `ZeroOrMore` / `OneOrMore` / `Remainder` | 形态收成枚举 |
| `Action.STORE` 等十种 | `Action::Store` 等 | 成员写法按 Rust 惯例 |
| `type=` | `Argument::value_parser` | `type` 是 Rust 关键字 |
| `const=` | `Argument::const_value` | `const` 是 Rust 关键字 |
| `global_=` | `Argument::global` | Python 侧带下划线才是为避开保留字 |
| `default=MISSING` / `SUPPRESS` / 任意值 | `DefaultValue::Missing` / `Suppress` / `Value(..)` | 两个哨兵收成枚举 |
| `choices=[..]` / `{值: 说明}` | `Choices::Values` / `Described` | |
| 任意 Python 值 | `Value::Null` / `Bool` / `Int(i64)` / `Float(f64)` / `Str` / `List` | 取值收成枚举；宽度按 §4.5 |
| 帮助 / 版本异常携带 `parser` 引用 | `ParseError::Help { long, prog }` | Rust 侧不持有借用，用命令全名回指 |
| `Namespace.as_dict()` | `Namespace::as_pairs()` | 保序形态，不转 `HashMap` |
| `Parser._parent` 链上取全局参数 | `Parser::global_arguments()` 返回快照 | 子命令按值持有、不存父指针; 快照在建子命令与登记新 `global` 参数时刷新, 只供帮助渲染, 解析仍走命令链 |

**C++ / Go / Zig 侧的命名差异**（同上，只列与 Python 不同的地方）：

- C++：类型与函数 snake_case，枚举成员全大写（`Action::STORE`）；`requires` 是 C++20 关键字，字段写
  `requires_args`；`type` / `const` 在 C++ 不是关键字，分别照写 `type` 与 `const_value`；解析结果是
  `std::variant`，退出码与渲染是自由函数 `exit_code` / `render`；补全的 `shell` 不认时抛
  `std::invalid_argument`，`prog` 用 `std::optional` 给（缺省取 `parser.prog()`）。
- Go：导出符号 PascalCase（`Argument.ConflictsWith` / `Argument.Global`），包内私有函数保持小驼峰；`type`
  是 Go 关键字，字段写 `Type`；`nargs` 的构造是包级函数 `NargsOne()` / `NargsFixed(3)` / `NargsOptional()` /
  `NargsZeroOrMore()` / `NargsOneOrMore()` / `NargsRemainder()`；`ParseKnown` 多返回值直接给出 `extras`，
  故没有 `KnownOutcome` 这一层；转换器字段是 `Type *ValueParser`，登记时取地址（`converter := ParseInt(nil, nil)`
  后 `Type: &converter`）；帮助渲染是包级函数 `RenderHelp` / `RenderUsage` / `RenderError`，命令上另有
  `FormatHelp(longHelp bool)` / `FormatUsage()`；帮助开关写 `WithoutHelp()`（Go 的导出约定决定了首字母大写）；
  补全的 `shell` 不认时返回 `error`，`prog` 传空串表示取 `Parser.Prog()`。
- Zig：`type` / `const` 是关键字，字段写 `value_parser` / `const_value`；`requires` 无冲突，但为与 C++ 对齐
  统一写 `requires_args`；`Value` 是 union，按 tag 读（`value == .integer` 后取 `value.integer`）；
  解析结果是 `Outcome` union（`ok` / `usage` / `help` / `version` / `definition`）；`error` 是关键字，着色方法
  叫 `Style.error_text`；`format_help` 的长短形态用入参 `long: bool` 给，Python 侧是关键字参数 `long=`；
  入口收 `*std.Io.Writer`（Go 收 `io.Writer`、C++ 收 `std::ostream&`），`parse_or_exit` 返回
  `Parser.ExitResult` union（`ok` / `exit`，退出码由 `exit_code()` 取）；`@file` 的前缀与 `Io` 一起登记
  （`with_fromfile_prefix_chars(chars, io)`），其余四侧只收前缀；补全的 `shell` 不认时返回
  `error.UnsupportedShell`，`prog` 用 `?[]const u8` 给。

## 3. 依赖

**出向依赖（本模块依赖的东西）**：本模块无三方运行时依赖 —— Python 侧只用标准库 `os` / `re` / `sys` /
`dataclasses` / `enum` / `typing` / `textwrap`；Rust 侧只用 `std`（`fmt` / `rc::Rc`）；C++ 侧只用标准库
（`<optional>` / `<variant>` / `<string>` / `<vector>`），Go 侧只用 `errors` / `fmt` / `os` / `path/filepath` /
`reflect` / `sort` / `strings` / `math` / `strconv`，Zig 侧只用 `std`。相似项建议的 Jaro-Winkler 各侧都自带实现
（`error.py` / `error.rs` / `error.cpp` / `error.go` / `error.zig`）：阈值 0.8 与「宁缺勿滥」的匹配口径是
CLI 特有，不属于通用字符串工具，因此不进 `quant1x/base`。

**入向依赖（谁依赖本模块）**：

| 调用方 | 用途 | 备注 |
| --- | --- | --- |
| `main.py`（Python） | `update` 子命令与 `--all` / `--base` / `--feature` | 取代原来的 `click` |
| `main.cpp`（C++） | 根命令 `--verbose` / `--debug` / `--version`，`service` 与 `update` 子命令 | 取代原来的 p-ranav `argparse` |
| `main.rs`（Rust） | 根命令 `--verbose` / `--debug` / `--version`，`service` 与 `update` 子命令 | 取代原来的 `clap` |
| `quant1x/command`（Go） | `q1x-go` 的命令行入口：根命令的 `--debug` / `--avx2` / `--cpu` 与 `update` 子命令 | 取代原来的 `cobra` |
| `tools/tail`（Go） | 文件末端阅览工具：`-f` / `-q` / `-v` / `-n` / `-c` 与文件位置参数 | 取代原来的 `cobra` + 手写扫描；GNU 的裸数字写法在解析前归一成 `--lines=N` |

## 4. 测试

### 4.1 单元测试

| 语言 | 测试文件 | 用例数 | 运行命令 |
| --- | --- | --- | --- |
| Python（spec） | `quant1x/base/argparser/parser_test.py` | 160 | `python -m quant1x.base.argparser.parser_test` |
| Python（spec） | `quant1x/base/argparser/help_test.py` | 54 | `python -m quant1x.base.argparser.help_test` |
| Python（spec） | `quant1x/base/argparser/complete_test.py` | 27 | `python -m quant1x.base.argparser.complete_test` |
| Python（spec） | `quant1x/base/argparser/derive_test.py` | 33 | `python -m quant1x.base.argparser.derive_test` |
| Rust | `quant1x/base/argparser/spec.rs` / `error.rs` / `parser.rs` / `help.rs` / `complete.rs`（内联 `mod tests`） | 61 | `cargo test base::argparser` |
| C++ | `tests/test-argparser-parser.cpp` / `tests/test-argparser-complete.cpp` | 35 个用例 / 172 处断言 ＋ 5 个用例 / 9 处断言 | `cmake --build build-msvc --config Release --target app-test-argparser-parser app-test-argparser-complete`，再运行 `build-msvc/tests/Release/app-test-argparser-parser.exe` 与 `app-test-argparser-complete.exe`（app 用例未登记 `add_test`，不走 `ctest`） |
| Go | `quant1x/base/argparser/parser_test.go` / `help_test.go` / `complete_test.go` | 40 | `go test ./quant1x/base/argparser` |
| Zig | `quant1x/base/argparser/parser_test.zig` | 25 | `zig test quant1x/base/argparser/parser_test.zig` |
| Zig | `quant1x/base/argparser/help_test.zig` | 9 个用例 / 73 条断言 | `zig test quant1x/base/argparser/help_test.zig` |
| Zig | `quant1x/base/argparser/complete_test.zig` | 5 | `zig test quant1x/base/argparser/complete_test.zig` |

覆盖到的边界：取值形态（空格 / `=` / 粘值 / 短选项聚合 / 未认出的尾巴）、签名数字与 `--`、`nargs` 全形态与
`REMAINDER`、十种动作与 `value_delimiter`、长选项前缀与歧义、未知参数与非法可选值的建议、必填 / 条件必填 /
必选组 / 互斥 / 冲突 / 依赖、`env` 兜底与非法环境变量、子命令（别名 / `dest` / 全局参数 / 必填 /
`subcommand_negates_reqs`）、自动 `help` 子命令（本命令 / 下沉一二级 / 未知名字 / 被同名自定义子命令覆盖 /
位置参数未够填时让位）、`parse_known_args` 与 `extras`（未知选项只收自身不吞取值、extras 保命令行顺序、
短选项粘值与 `=` 形态整体收下）、
`SUPPRESS` / `argument_default`、类型转换与区间、错误类别、退出码与输出流、
定义期校验（重名 / 未知关键字 / `REMAINDER` 之后加位置参数 / 命令链 `dest` 重复）、usage 组成与章节顺序、
标注与 `hide`、分组、对齐与折行、`-h` 与 `--help` 的区分、着色开关、
`@file` 展开（默认不展开 / 每行一个 token / 行内不拆词 / 嵌套 / `parse_known_args` 同样展开 / 空行成空 token /
打不开与自引用的报错）、`overrides_with`（后出现者胜 / 反向同样 / 被覆盖方退回默认值 / 指向未登记参数报
定义期错误）、帮助宽度（默认不读 `COLUMNS` / 开了 `auto_width` 才读 / 非法与非正值退回 `max_width`）、
补全生成（三种 shell 的骨架与注册行 / 子命令链状态机 / `hide` 参数不列 / 全局参数下沉子命令 / 位置参数
`choices` / `prog` 覆盖与函数名归一 / fish 的单引号转义 / 未知 shell 报 `ValueError`）、派生式声明（字段顺序与
`dest` / 必填 / 四种标量转换 / 布尔两种默认 / 可空与多值形态 / `Literal` 与 `Enum` 的可选值 / 位置参数元数据 /
说明文字 / 认不出的注解与非法形态在定义期报错）。

未覆盖及原因：`color="auto"` 的真实终端判定（需 TTY，用例只断言 `always` / `never`）；三层以上子命令嵌套
（当前最深两层，无调用方需要）；线程安全不在契约内（定义期可变、解析期只读）。

C++ / Rust / Go / Zig 四侧覆盖同一批可判定项（`spec` / `error` / `parser` 三层）：动作是否吃取值、`Nargs` 的数量
与收集形态、`Value` 的文本形态、命名空间的写入顺序与覆盖、三个转换器的合法 / 非法 / 越界文案、`Argument` 的占位与
指代、Jaro-Winkler 相似度与 `best_match` 阈值、`UsageError` 的非着色版式、解析结果的退出码，以及解析引擎本身
（token 形态与短选项聚合、长选项前缀与歧义、位置参数分配与 `REMAINDER`、`parse` / `parse_known` 的 `extras` 与
相似项建议、`env` 兜底、必填 / 条件必填 / 必选组 / 互斥 / 冲突 / 依赖、子命令与自动 `help` 子命令、命令链
`dest` 重复的定义期错误）。
入口的落点也各测一遍：`print_help` 末尾补一个换行、`parse_or_exit` 把帮助 / 版本写到 `out` 退 0、用法错误写到
`err` 退 2、成功时不打印任何文本。`@file` 展开五侧各一条用例（嵌套展开与打不开报错）。补全生成五侧齐
（Python 27 条、Rust 5 条、C++ 5 条 / 9 处断言、Go 5 条、Zig 5 条），都是三种 shell 的脚本文本比字节，另加
`prog` 覆盖与函数名归一、无子命令时不留状态机、空说明文字不写 `-d`、未知 shell 报错。

差异只在用例条数，属于平台手段而非语义：Zig 侧把 `env` 用例留给对拍、不在单测里改进程环境（见
`parser_test.zig` 的文件头说明），故 Zig 30 条；C++ 与 Go 的 40 条里各有 9 组 `help` 用例与 5 组补全用例，
Rust 的 61 条里有 9 条（`help.rs` 内联 `mod tests`）与 5 条补全生成（`complete.rs` 内联 `mod tests`），Zig 另起
`help_test.zig`（9 个用例 / 73 条断言）。四侧的 `help`
用例都覆盖 `help_test.py` 的 54 项检查：usage 组成、章节顺序、标注、分组、对齐与折行、子命令继承全局参数、
错误版式、着色与入口的落点。

### 4.2 覆盖率

| 语言 | 工具 | 门槛 | 运行命令 |
| --- | --- | --- | --- |
| Python | `coverage` | 未设门槛（仓库未配置覆盖率门槛） | `python -m coverage run --source=quant1x.base.argparser -m quant1x.base.argparser.parser_test && python -m coverage run --append --source=quant1x.base.argparser -m quant1x.base.argparser.help_test && python -m coverage run --append --source=quant1x.base.argparser -m quant1x.base.argparser.complete_test && python -m coverage report` |
| Rust | `cargo-llvm-cov` | 未设门槛（工具未纳入本机工具链） | `cargo llvm-cov`（未实测） |
| C++ | `lcov` 或 `gcovr` | 未接入（构建走 MSVC，覆盖率工具未纳入本机流程） | — |
| Go | `go test -cover` | 未设门槛（实测 81.4%，2026-11-15） | `go test ./quant1x/base/argparser -cover` |
| Zig | 覆盖率导出（工具待定） | 未接入（本模块无 `build.zig`，用例直接 `zig test`） | — |

### 4.3 重复度

本模块不涉及（本仓未引入 jscpd 门禁，根目录无 `.jscpd.json`）。引入后按
[CONTRIBUTING.md](../../../CONTRIBUTING.md) §4.3 的口径计入全仓指标。

### 4.4 静态检查

| 语言 | 工具 | 运行命令 | 备注 |
| --- | --- | --- | --- |
| Python | `ruff` ＋ `pyright` | `python -m ruff check quant1x/base/argparser` ＋ `python -m pyright quant1x/base/argparser` | 仓库未配置 ruff / pyright，本节只给命令；`Namespace` 的属性读法由类上的 `__getattr__` 声明，类型检查据此认可任意属性名 |
| Rust | `cargo fmt` ＋ `cargo clippy -D warnings` | `cargo fmt -- --check` ＋ `cargo clippy --all-targets -- -D warnings` | 组件见根 `rust-toolchain.toml` |
| C++ | `clang-format --dry-run -Werror` ＋ `clang-tidy` | `clang-format --dry-run -Werror quant1x/base/argparser/*.hpp quant1x/base/argparser/*.cpp` ＋ `clang-tidy quant1x/base/argparser/*.cpp -- -std=c++20 -I.` | 配置在仓库根（`.clang-format` / `.clang-tidy` / `.clangd`），模块内 10 个源文件、5 个 TU；`Value` 的值语义递归按 `NOLINT(misc-no-recursion)` 豁免，系统头 `xutility` 由 `HeaderFilter` 排除；clangd 走 `.clangd` 的兜底参数（`-std=c++20` 与 `-I../..`）—— CMake 走 VS 生成器，不导出 `compile_commands.json` |
| Go | `gofmt -l` ＋ `go vet` | `gofmt -l quant1x/base/argparser` ＋ `go vet ./quant1x/base/argparser` | 实测两条命令均无输出 |
| Zig | `zig fmt --check` | `zig fmt --check quant1x/base/argparser/spec.zig quant1x/base/argparser/error.zig quant1x/base/argparser/parser.zig quant1x/base/argparser/help.zig quant1x/base/argparser/complete.zig quant1x/base/argparser/parser_test.zig quant1x/base/argparser/help_test.zig quant1x/base/argparser/complete_test.zig` | 实测通过；本模块无 `build.zig`，不提供 `zig build test` |

### 4.5 基准测试

本模块不涉及 —— 无性能契约：解析是进程启动时的一次性动作，规模由命令行长度决定，没有值得设基线的吞吐或延迟
指标；故五门语言（含 `zig`）都不设基准行，本节不建表。

### 4.6 契约 / 一致性测试

`spec` / `error` / `parser` / `help` 四层在五侧（python / c++ / rust / go / zig）都已可对拍；对拍样本：

| 样本 | 期望值来源 | 说明 |
| --- | --- | --- |
| `Value` 的文本形态 | Python `_format_value` / `str` | 浮点补 `.0`、`inf` / `nan` 小写、布尔 `true` / `false`、列表 `a, b` |
| 转换器的合法 / 非法 / 越界 | Python `parse_int` / `parse_float` / `parse_bool` | 文案逐字一致，区间记法 `1..=10` / `..=10` / `1..` |
| `Argument` 的占位与指代 | Python `usage_spec` / `help_names` / `error_name` | 各 `nargs` 形态逐个比对 |
| Jaro-Winkler 相似度 | Python `jaro_winkler` | `--flah` ↔ `--flag` = 0.9333、`--verbose` ↔ `--version` = 0.8825 |
| `UsageError` 的非着色版式 | Python `render_error(color=False)` | 首行 / 明细 / `tip:` / usage / 帮助提示的段间空行位置 |
| 必选组缺项的明细行 | Python `_validate` 的 `lines` | 缺必选组时报 `<组名>` 一行，排在首行之下、usage 之上 |
| `ParseError` 的退出码 | Python 三类异常的 `exit_code` | 用法错误 2，帮助与版本 0 |
| 解析结果与 `extras` | Python `parse_args` / `parse_known_args` | 同一命令行两侧落出同一个命名空间与同一顺序的 `extras` |
| 未知参数的建议 | Python `_unknown_argument` | 相似项建议优于 `-- <token>` 提示，两者的取舍两侧一致 |
| 版本行 | Python `version_text()` | 子命令沿用父命令版本，文本为 `<prog> <version>` |
| 帮助版面 | Python `format_help(long=)` / `format_usage` | 章节顺序、左列对齐、说明折行缩进、全局参数下沉子命令帮助 |
| 补全脚本 | Python `complete(parser, shell)` | bash / zsh / fish 三种脚本比字节：函数骨架与注册行、子命令链状态机、候选词顺序、fish 的单引号转义 |

C++ / Rust / Go / Zig 四侧已按上表的样本逐字对拍（2026-11-15）：同一份定义分别渲染后比对字节，短帮助、长帮助、
子命令帮助（含全局参数下沉）、`max_width=44` 的窄幅折行、用法错误块五个样本与 Python 输出完全一致（C++ 侧
比对前把 CRLF 归一成 LF —— 平台运行库的文本模式换行，Python 的 `print` 同样如此）；`help.rs` 内联的 9 条用例、
`tests/test-argparser-parser.cpp` 里的 9 组断言、`help_test.go` 的 9 条用例与 `help_test.zig` 的 73 条断言就是这些样本的
可执行形态。补全脚本这一行五侧都已对拍：`complete.rs` 内联的 5 条用例、`tests/test-argparser-complete.cpp`、`complete_test.go`
与 `complete_test.zig` 各把三种 shell 的脚本与 Python `complete_test.py` 的样本逐字节比对（含 fish 的单引号
转义）。新增或改动版式时按
[CONTRIBUTING.md](../../../CONTRIBUTING.md) §7 继续：样本按边界选取、清单写回
本节，期望值只由 Python 生成，其余侧只被比对。着色版式（ANSI）不做文本对拍 —— 各侧都只断言「关着色时等价、
开着色时版式不变」。

## 5. TODO

当前无待办项。派生式声明已按 §2 第 17 条落在 Python 侧（C++ / Rust / Go / Zig 无运行期注解，不跟进），
补全生成与「签名数字整体当取值」已收口；后续新增能力先进 §2 的口径表，再按 §4.6 对拍。
