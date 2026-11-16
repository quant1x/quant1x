// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 命令定义与解析: 选项匹配、位置参数分配、子命令分发、缺省值与校验。
//!
//! 解析是单趟左到右的扫描: 遇到选项就地取走它的取值, 其余 token 攒起来交给位置参数;
//! 遇到子命令名则把剩余 token 整段交给子命令, 此前攒下的 token 仍归本命令的位置参数。
//!
//! 失败一律返回 [`ParseError`], 不打印也不退出; 输出与退出码由帮助渲染层负责。

use std::collections::{BTreeMap, BTreeSet};
use std::env;
use std::io::IsTerminal;

use super::error::{ErrorKind, ParseError, UsageError, best_match, format_items};
use super::spec::{
    Action, Argument, DefaultValue, Namespace, Nargs, Value, ValueParser, parse_bool, parse_float,
    parse_int,
};

/// 着色的时机, 对应 Python 侧的 `color` 参数。
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub enum ColorChoice {
    /// 依目标流是否终端与 `NO_COLOR` 判定。
    #[default]
    Auto,
    /// 总是着色。
    Always,
    /// 从不着色。
    Never,
}

/// 互斥组: 组内参数最多出现一个; `required` 为真时至少出现一个。
#[derive(Debug, Clone)]
pub struct MutexGroup {
    required: bool,
    members: Vec<usize>,
}

impl MutexGroup {
    /// 是否要求至少出现一个。
    pub fn required(&self) -> bool {
        self.required
    }
}

/// 多值组: 组内参数可以同时出现; `required` 为真时至少要出现一个。
#[derive(Debug, Clone)]
pub struct ValueGroup {
    name: String,
    required: bool,
    members: Vec<usize>,
}

impl ValueGroup {
    /// 组名, 缺必填时报在 `<名字>` 那一行。
    pub fn name(&self) -> &str {
        &self.name
    }

    /// 是否要求至少出现一个。
    pub fn required(&self) -> bool {
        self.required
    }
}

/// 子命令集合: 名字 (含别名) 到命令的映射, 并记录帮助里的展示顺序。
#[derive(Debug)]
pub struct SubParsers {
    dest: Option<String>,
    title: String,
    required: bool,
    help: Option<String>,
    prog_prefix: String,
    version: Option<String>,
    allow_abbrev: bool,
    color: ColorChoice,
    max_width: u32,
    /// 建子命令时下沉给它们的 `global` 参数快照。
    globals: Vec<Argument>,
    commands: Vec<Parser>,
    lookup: BTreeMap<String, usize>,
    visible: BTreeMap<String, Vec<String>>,
}

impl SubParsers {
    /// 登记一条子命令; `aliases` 不出现在帮助里, `visible_aliases` 出现。
    pub fn add_parser(
        &mut self,
        name: &str,
        about: &str,
        aliases: &[&str],
        visible_aliases: &[&str],
    ) -> Result<&mut Parser, String> {
        let mut child = Parser::new(&format!("{} {name}", self.prog_prefix));
        child.name = Some(name.to_string());
        child.about = (!about.is_empty()).then(|| about.to_string());
        child.version = self.version.clone();
        child.register_version();
        child.allow_abbrev = self.allow_abbrev;
        child.color = self.color;
        child.max_width = self.max_width;
        child.inherited = self.globals.clone();
        let index = self.commands.len();
        for key in std::iter::once(name)
            .chain(aliases.iter().copied())
            .chain(visible_aliases.iter().copied())
        {
            if self.lookup.contains_key(key) {
                return Err(format!("子命令名重复登记: {key:?}"));
            }
            self.lookup.insert(key.to_string(), index);
        }
        self.visible.insert(
            name.to_string(),
            visible_aliases
                .iter()
                .map(|alias| (*alias).to_string())
                .collect(),
        );
        self.commands.push(child);
        Ok(&mut self.commands[index])
    }

    /// 把一条新登记的 `global` 参数带到已登记的子命令里, 供它们的帮助渲染。
    fn carry_global(&mut self, argument: &Argument) {
        self.globals.push(argument.clone());
        for child in self.commands.iter_mut() {
            child.inherit_global(argument);
        }
    }

    /// 按名字或别名找子命令。
    pub fn lookup(&self, name: &str) -> Option<&Parser> {
        self.lookup.get(name).map(|index| &self.commands[*index])
    }

    /// 该名字 (含别名) 是否已被登记成子命令。
    pub fn has_command(&self, name: &str) -> bool {
        self.lookup.contains_key(name)
    }

    /// 名字归一成注册时的正式名 (别名解析回正名)。
    pub fn canonical(&self, name: &str) -> String {
        match self.lookup.get(name) {
            Some(index) => self.commands[*index]
                .primary_name()
                .unwrap_or(name)
                .to_string(),
            None => name.to_string(),
        }
    }

    /// 按注册顺序给出 `(正式名, 展示用别名, 命令)`。
    pub fn commands(&self) -> impl Iterator<Item = (&str, &[String], &Parser)> {
        self.commands.iter().map(|command| {
            let name = command.name.as_deref().unwrap_or_default();
            let aliases = self
                .visible
                .get(name)
                .map(Vec::as_slice)
                .unwrap_or_default();
            (name, aliases, command)
        })
    }

    /// 帮助章节标题。
    pub fn title(&self) -> &str {
        &self.title
    }

    /// 子命令章节的说明文字, 对应 Python 的 `add_subparsers(help=...)`。
    pub fn help(&self) -> Option<&str> {
        self.help.as_deref()
    }

    /// 设置子命令章节的说明文字。
    pub fn set_help(&mut self, help: &str) {
        self.help = Some(help.to_string());
    }

    /// 写进命名空间的子命令名字段。
    pub fn dest(&self) -> Option<&str> {
        self.dest.as_deref()
    }

    /// 子命令是否必填。
    pub fn is_required(&self) -> bool {
        self.required
    }
}

/// 一条命令: 参数定义、解析入口与子命令。子命令本身也是 [`Parser`]。
#[derive(Debug)]
pub struct Parser {
    prog: String,
    about: Option<String>,
    long_about: Option<String>,
    version: Option<String>,
    epilog: Option<String>,
    usage: Option<String>,
    allow_abbrev: bool,
    argument_default: DefaultValue,
    color: ColorChoice,
    max_width: u32,
    /// 是否按 `COLUMNS` 取帮助宽度。
    auto_width: bool,
    subcommand_negates_reqs: bool,
    /// `@file` 展开用的前缀字符集; `None` 表示不展开。
    fromfile_prefix_chars: Option<String>,
    arguments: Vec<Argument>,
    names: BTreeMap<String, usize>,
    positionals: Vec<usize>,
    dests: BTreeMap<String, usize>,
    groups: Vec<(String, Option<String>)>,
    mutex: Vec<MutexGroup>,
    value_groups: Vec<ValueGroup>,
    subparsers: Option<SubParsers>,
    name: Option<String>,
    /// 祖先命令下沉的 `global` 参数快照, 只用于帮助渲染 (解析仍走命令链)。
    inherited: Vec<Argument>,
}

impl Parser {
    // ---- 定义 ----------------------------------------------------------

    /// 起一条命令; 缺省带 `-h` / `--help`。
    pub fn new(prog: &str) -> Self {
        let mut parser = Self {
            prog: prog.to_string(),
            about: None,
            long_about: None,
            version: None,
            epilog: None,
            usage: None,
            allow_abbrev: true,
            argument_default: DefaultValue::Missing,
            color: ColorChoice::Auto,
            max_width: 100,
            auto_width: false,
            subcommand_negates_reqs: false,
            fromfile_prefix_chars: None,
            arguments: Vec::new(),
            names: BTreeMap::new(),
            positionals: Vec::new(),
            dests: BTreeMap::new(),
            groups: Vec::new(),
            mutex: Vec::new(),
            value_groups: Vec::new(),
            subparsers: None,
            name: None,
            inherited: Vec::new(),
        };
        parser.push(Argument {
            names: vec!["-h".to_string(), "--help".to_string()],
            dest: "help".to_string(),
            action: Action::Help,
            nargs: Nargs::Count(0),
            default: DefaultValue::Suppress,
            help: Some("Print help".to_string()),
            ..Argument::default()
        });
        parser
    }

    /// 按可执行文件名起 `prog`, 对应 Python 侧取 `sys.argv[0]` 的文件名。
    pub fn from_env() -> Self {
        let prog = env::args_os()
            .next()
            .map(std::path::PathBuf::from)
            .and_then(|path| {
                path.file_name()
                    .map(|name| name.to_string_lossy().into_owned())
            })
            .unwrap_or_else(|| "prog".to_string());
        Self::new(&prog)
    }

    /// 设置一段简介。
    pub fn with_about(mut self, about: &str) -> Self {
        self.about = Some(about.to_string());
        self
    }

    /// 设置长简介 (`--help` 用)。
    pub fn with_long_about(mut self, about: &str) -> Self {
        self.long_about = Some(about.to_string());
        self
    }

    /// 设置版本号并登记 `-V` / `--version`; 子命令没自带时沿用父命令的。
    pub fn with_version(mut self, version: &str) -> Self {
        self.version = Some(version.to_string());
        self.register_version();
        self
    }

    /// 有版本号就登记 `-V` / `--version`; 已占住 `version` 这个 dest 的不再重复登记。
    fn register_version(&mut self) {
        if self.version.is_some() && !self.dests.contains_key("version") {
            self.push(Argument {
                names: vec!["-V".to_string(), "--version".to_string()],
                dest: "version".to_string(),
                action: Action::Version,
                nargs: Nargs::Count(0),
                default: DefaultValue::Suppress,
                help: Some("Print version".to_string()),
                ..Argument::default()
            });
        }
    }

    /// 设置尾注。
    pub fn with_epilog(mut self, epilog: &str) -> Self {
        self.epilog = Some(epilog.to_string());
        self
    }

    /// 覆盖 usage 行。
    pub fn with_usage(mut self, usage: &str) -> Self {
        self.usage = Some(usage.to_string());
        self
    }

    /// 关掉长选项前缀缩写。
    pub fn without_abbrev(mut self) -> Self {
        self.allow_abbrev = false;
        self
    }

    /// 设置全局缺省值 (`argument_default`)。
    pub fn with_argument_default(mut self, value: DefaultValue) -> Self {
        self.argument_default = value;
        self
    }

    /// 设置着色时机。
    pub fn with_color(mut self, color: ColorChoice) -> Self {
        self.color = color;
        self
    }

    /// 设置帮助折行宽度。
    pub fn with_max_width(mut self, width: u32) -> Self {
        self.max_width = width;
        self
    }

    /// 子命令出现时是否放行父命令的必填项。
    pub fn with_subcommand_negates_reqs(mut self, flag: bool) -> Self {
        self.subcommand_negates_reqs = flag;
        self
    }

    /// 帮助宽度按 `COLUMNS` 取; 取不到或非法时仍用 `max_width`。
    pub fn with_auto_width(mut self) -> Self {
        self.auto_width = true;
        self
    }

    /// 帮助的生效宽度: 开了 `auto_width` 时读 `COLUMNS`, 取不到或非法就用 `max_width`。
    pub fn width(&self) -> u32 {
        if self.auto_width {
            let columns = std::env::var("COLUMNS")
                .ok()
                .and_then(|raw| raw.trim().parse::<u32>().ok())
                .filter(|parsed| *parsed > 0);
            if let Some(parsed) = columns {
                return parsed;
            }
        }
        self.max_width
    }

    /// 把 `@file` 形态的 token 展开成文件里的行; 传空串等于不展开。
    pub fn with_from_file_prefix_chars(mut self, chars: &str) -> Self {
        self.fromfile_prefix_chars = if chars.is_empty() {
            None
        } else {
            Some(chars.to_string())
        };
        self
    }

    /// 登记一个参数: 首个名字以 `-` 开头是选项, 否则是位置参数。
    pub fn add_argument(&mut self, flags: &[&str], mut argument: Argument) -> Result<(), String> {
        let Some(first) = flags.first() else {
            return Err("至少需要一个参数名".to_string());
        };
        let positional = !first.starts_with('-');
        if positional {
            if flags.len() > 1 {
                return Err(format!("位置参数只能有一个名字: {flags:?}"));
            }
            if self
                .positionals
                .iter()
                .any(|index| self.arguments[*index].nargs == Nargs::Remainder)
            {
                return Err("REMAINDER 位置参数必须是最后一个, 其后不能再加位置参数".to_string());
            }
            if argument.required {
                return Err("位置参数的必填性由 nargs 决定, 不接受 required".to_string());
            }
        }
        let names: Vec<String> = if positional {
            Vec::new()
        } else {
            flags.iter().map(|flag| (*flag).to_string()).collect()
        };
        for name in names
            .iter()
            .chain(argument.alias.iter())
            .chain(argument.visible_alias.iter())
        {
            if !name.starts_with('-') {
                return Err(format!("选项名必须以 '-' 开头: {name:?}"));
            }
            if is_signed_number(name) {
                return Err(format!("签名数字不能作选项名, 会被整体当成取值: {name:?}"));
            }
        }

        if argument.dest.is_empty() {
            argument.dest = if positional {
                dest_of_positional(first)
            } else {
                dest_of_option(&names)
            };
        }
        argument.names = names;

        if !argument.action.takes_values() {
            if !matches!(argument.nargs, Nargs::One | Nargs::Count(0)) {
                return Err(format!(
                    "action={:?} 不吃取值, 不能配 nargs",
                    argument.action
                ));
            }
            argument.nargs = Nargs::Count(0);
            match argument.action {
                Action::StoreTrue | Action::StoreFalse => {
                    let enabled = argument.action == Action::StoreTrue;
                    if argument.const_value.is_none() {
                        argument.const_value = Some(Value::Bool(enabled));
                    }
                    if matches!(argument.default, DefaultValue::Missing) {
                        argument.default = DefaultValue::Value(Value::Bool(!enabled));
                    }
                }
                Action::Count => {
                    if matches!(argument.default, DefaultValue::Missing) {
                        argument.default = DefaultValue::Value(Value::Int(0));
                    }
                }
                Action::StoreConst | Action::AppendConst if argument.const_value.is_none() => {
                    return Err(format!("action={:?} 必须给 const", argument.action));
                }
                _ => {}
            }
        } else if argument.value_delimiter.is_some() && argument.nargs == Nargs::One {
            // 配了分隔符就是「一条 token 里塞多个值」, 数量按一个 token 算, 落值仍是列表
            argument.nargs = Nargs::Count(1);
        }

        if argument.value_parser.is_none()
            && argument.action.takes_values()
            && let DefaultValue::Value(value) = &argument.default
        {
            argument.value_parser = infer_parser(value);
        }

        for name in argument
            .names
            .iter()
            .chain(argument.alias.iter())
            .chain(argument.visible_alias.iter())
        {
            if self.names.contains_key(name) {
                return Err(format!("选项名重复登记: {name:?}"));
            }
        }
        if self.dests.contains_key(&argument.dest) {
            return Err(format!("dest 重复登记: {:?}", argument.dest));
        }
        let inherited = argument.global.then(|| argument.clone());
        self.push(argument);
        if let Some(argument) = inherited
            && let Some(subparsers) = self.subparsers.as_mut()
        {
            subparsers.carry_global(&argument);
        }
        Ok(())
    }

    /// 登记一个帮助分组; 该组的参数在帮助里单独成节。
    pub fn add_group(&mut self, title: &str, description: Option<&str>) {
        self.groups
            .push((title.to_string(), description.map(str::to_string)));
    }

    /// 登记一个互斥组, 返回组下标; 用 [`Parser::add_argument_to_group`] 往里加参数。
    pub fn add_mutually_exclusive_group(&mut self, required: bool) -> usize {
        self.mutex.push(MutexGroup {
            required,
            members: Vec::new(),
        });
        self.mutex.len() - 1
    }

    /// 登记一个多值组, 返回组下标; 用 [`Parser::add_argument_to_value_group`] 往里加参数。
    pub fn add_value_group(&mut self, name: &str, required: bool) -> usize {
        self.value_groups.push(ValueGroup {
            name: name.to_string(),
            required,
            members: Vec::new(),
        });
        self.value_groups.len() - 1
    }

    /// 往多值组里加一个参数。
    pub fn add_argument_to_value_group(
        &mut self,
        group: usize,
        flags: &[&str],
        argument: Argument,
    ) -> Result<(), String> {
        self.add_argument(flags, argument)?;
        let index = self.arguments.len() - 1;
        match self.value_groups.get_mut(group) {
            Some(slot) => slot.members.push(index),
            None => return Err(format!("多值组下标越界: {group}")),
        }
        Ok(())
    }

    /// 往互斥组里加一个参数。
    pub fn add_argument_to_group(
        &mut self,
        group: usize,
        flags: &[&str],
        argument: Argument,
    ) -> Result<(), String> {
        self.add_argument(flags, argument)?;
        let index = self.arguments.len() - 1;
        match self.mutex.get_mut(group) {
            Some(slot) => slot.members.push(index),
            None => return Err(format!("互斥组下标越界: {group}")),
        }
        Ok(())
    }

    /// 登记子命令集合; `dest` 给出时把命中 (含别名) 的正名写进命名空间。
    pub fn add_subparsers(
        &mut self,
        dest: Option<&str>,
        title: &str,
        required: bool,
    ) -> Result<&mut SubParsers, String> {
        if self.subparsers.is_some() {
            return Err("子命令集合只能登记一次".to_string());
        }
        self.subparsers = Some(SubParsers {
            dest: dest.map(str::to_string),
            title: title.to_string(),
            required,
            help: None,
            prog_prefix: self.prog.clone(),
            version: self.version.clone(),
            allow_abbrev: self.allow_abbrev,
            color: self.color,
            max_width: self.max_width,
            globals: self.ancestor_globals(),
            commands: Vec::new(),
            lookup: BTreeMap::new(),
            visible: BTreeMap::new(),
        });
        self.subparsers
            .as_mut()
            .ok_or_else(|| "子命令集合登记失败".to_string())
    }

    /// 建子命令时要带下层的 `global` 参数: 祖先的先来, 本命令的随后。
    fn ancestor_globals(&self) -> Vec<Argument> {
        let mut globals = self.inherited.clone();
        globals.extend(
            self.arguments
                .iter()
                .filter(|argument| argument.global)
                .cloned(),
        );
        globals
    }

    /// 记下一条从祖先命令下沉的 `global` 参数, 并继续传给自己的子命令。
    fn inherit_global(&mut self, argument: &Argument) {
        self.inherited.push(argument.clone());
        if let Some(subparsers) = self.subparsers.as_mut() {
            subparsers.carry_global(argument);
        }
    }

    /// 登记一个参数: 名字与别名入索引表, `dest` 入表, 位置参数记入顺序, 本体追加到列表。
    fn push(&mut self, argument: Argument) {
        let index = self.arguments.len();
        for name in argument
            .names
            .iter()
            .chain(argument.alias.iter())
            .chain(argument.visible_alias.iter())
        {
            self.names.insert(name.clone(), index);
        }
        self.dests.insert(argument.dest.clone(), index);
        if argument.is_positional() {
            self.positionals.push(index);
        }
        self.arguments.push(argument);
    }

    // ---- 只读访问 ------------------------------------------------------

    /// 命令名。
    pub fn prog(&self) -> &str {
        &self.prog
    }

    /// 简介。
    pub fn about(&self) -> Option<&str> {
        self.about.as_deref()
    }

    /// 长简介。
    pub fn long_about(&self) -> Option<&str> {
        self.long_about.as_deref()
    }

    /// 版本号; 子命令没自带时沿用父命令的。
    pub fn version(&self) -> Option<&str> {
        self.version.as_deref()
    }

    /// 版本行的正文: `<prog> <version>`。
    pub fn version_text(&self) -> String {
        match &self.version {
            Some(version) => format!("{} {version}", self.prog),
            None => self.prog.clone(),
        }
    }

    /// 尾注。
    pub fn epilog(&self) -> Option<&str> {
        self.epilog.as_deref()
    }

    /// 着色时机。
    pub fn color(&self) -> ColorChoice {
        self.color
    }

    /// 帮助折行宽度。
    pub fn max_width(&self) -> u32 {
        self.max_width
    }

    /// 祖先命令下沉的 `global` 参数: 子命令里同样可用, 也出现在子命令帮助里。
    ///
    /// 这是建子命令时留下的快照, 只供帮助渲染; 解析时仍按命令链查找。
    pub fn global_arguments(&self) -> &[Argument] {
        &self.inherited
    }

    /// 本命令登记的参数 (含位置参数), 按注册顺序。
    pub fn arguments(&self) -> &[Argument] {
        &self.arguments
    }

    /// 按 `dest` 取参数定义。
    pub fn argument(&self, dest: &str) -> Option<&Argument> {
        self.dests.get(dest).map(|index| &self.arguments[*index])
    }

    /// 帮助分组: 标题到描述的映射, 按登记顺序。
    pub fn groups(&self) -> &[(String, Option<String>)] {
        &self.groups
    }

    /// 互斥组, 按登记顺序。
    pub fn mutex_groups(&self) -> &[MutexGroup] {
        &self.mutex
    }

    /// 多值组, 按登记顺序。
    pub fn value_groups(&self) -> &[ValueGroup] {
        &self.value_groups
    }

    /// 子命令集合; 没登记则为 `None`。
    pub fn subparsers(&self) -> Option<&SubParsers> {
        self.subparsers.as_ref()
    }

    /// 子命令集合的可变引用, 用于继续追加子命令。
    pub fn subparsers_mut(&mut self) -> Option<&mut SubParsers> {
        self.subparsers.as_mut()
    }

    /// 在父命令里的子命令名; 顶层命令为 `None`。
    pub fn name(&self) -> Option<&str> {
        self.name.as_deref()
    }

    /// 该 token 是否按选项处理: `-` / `--` 与签名数字以外的 `-` 开头 token。
    pub fn is_option_token(token: &str) -> bool {
        if token == "--" {
            return true;
        }
        if !token.starts_with('-') || token == "-" {
            return false;
        }
        !is_signed_number(token)
    }

    /// 是否着色: `always` / `never` 直接定, `auto` 看 stdout 与 `NO_COLOR`。
    pub fn color_enabled(&self) -> bool {
        match self.color {
            ColorChoice::Always => true,
            ColorChoice::Never => false,
            ColorChoice::Auto => {
                if env::var_os("NO_COLOR").is_some() {
                    return false;
                }
                std::io::stdout().is_terminal()
            }
        }
    }

    // ---- 解析 ----------------------------------------------------------

    /// 解析命令行; 输入非法返回 [`ParseError`], 请求帮助 / 版本返回对应变体。
    pub fn parse(&self, argv: &[String]) -> Result<Namespace, ParseError> {
        let (namespace, extras) = self.parse_run(argv, false)?;
        if let Some(first) = extras.first() {
            return Err(self
                .fail(
                    format!("unexpected argument '{first}' found"),
                    ErrorKind::UnknownArgument,
                )
                .into());
        }
        Ok(namespace)
    }

    /// 同 [`Parser::parse`], 但认不出的 token 收进返回值而不报错。
    ///
    /// 未知的**选项**只把该 token 本身收进 `extras`, 不连带吞掉它后面的 token。
    pub fn parse_known(&self, argv: &[String]) -> Result<(Namespace, Vec<String>), ParseError> {
        self.parse_run(argv, true)
    }

    /// 两个解析入口的共用骨架; `tolerant` 决定未知选项是报错还是收进 `extras`。
    fn parse_run(
        &self,
        argv: &[String],
        tolerant: bool,
    ) -> Result<(Namespace, Vec<String>), ParseError> {
        let tokens = self.expand_files(argv)?;
        let mut run = Run::new(tolerant);
        self.parse_tokens(&tokens, &mut run, &[])?;
        self.finish(&mut run)?;
        let extras = run.sorted_extras();
        Ok((run.namespace, extras))
    }

    /// `@file` 展开: 每行原样作一个 token (不拆词、不去空白、不认注释), 展开结果里再出现的
    /// 前缀继续展开; 打不开文件与嵌套过深都按用法错误报。
    fn expand_files(&self, tokens: &[String]) -> Result<Vec<String>, ParseError> {
        self.expand_files_at(tokens, 0)
    }

    /// 递归展开 `@file`: 每行当一个参数, 相对路径按当前文件所在目录解析。
    fn expand_files_at(&self, tokens: &[String], depth: usize) -> Result<Vec<String>, ParseError> {
        let Some(prefixes) = self.fromfile_prefix_chars.as_deref() else {
            return Ok(tokens.to_vec());
        };
        let mut expanded = Vec::with_capacity(tokens.len());
        for token in tokens {
            let mut chars = token.chars();
            let prefixed = chars.next().is_some_and(|first| prefixes.contains(first));
            if !prefixed {
                expanded.push(token.clone());
                continue;
            }
            let path = chars.as_str();
            if depth >= MAX_INCLUDE_DEPTH {
                return Err(self
                    .fail(
                        format!("@file nesting too deep: '{path}'"),
                        ErrorKind::Other,
                    )
                    .into());
            }
            let Ok(body) = std::fs::read_to_string(path) else {
                return Err(self
                    .fail(format!("unable to open file '{path}'"), ErrorKind::Other)
                    .into());
            };
            let lines = split_lines(&body);
            expanded.extend(self.expand_files_at(&lines, depth + 1)?);
        }
        Ok(expanded)
    }

    /// 主循环: `--` 之后全归位置参数, 选项走 `consume_option`, 其余交给 `assign_positionals` 与子命令。
    fn parse_tokens<'a>(
        &'a self,
        tokens: &[String],
        run: &mut Run<'a>,
        chain: &[&'a Parser],
    ) -> Result<(), ParseError> {
        run.parsers.push(self);
        let mut tokens = tokens.to_vec();
        let mut pending: Vec<(usize, String)> = Vec::new();
        let mut index = 0usize;
        while index < tokens.len() {
            let token = tokens[index].clone();
            if token == "--" {
                pending.extend(slots(&tokens[index + 1..], index + 1));
                break;
            }
            if Self::is_option_token(&token) {
                if let Some(expanded) = self.expand_cluster(&token, chain)? {
                    tokens.splice(index..index + 1, expanded);
                    continue;
                }
                index = self.consume_option(&mut tokens, index, run, chain)?;
                continue;
            }
            if self.remainder_ready(&pending) {
                pending.extend(slots(&tokens[index..], index));
                break;
            }
            if self.help_subcommand_of(&token, &pending) {
                let target = self.help_target(&tokens[index + 1..])?;
                return Err(ParseError::Help {
                    long: true,
                    prog: target.prog.clone(),
                });
            }
            if let Some(child) = self.subcommand_of(&token, &pending) {
                run.subcommand = Some(token.clone());
                if let Some(subparsers) = &self.subparsers
                    && let Some(dest) = &subparsers.dest
                {
                    let canonical = subparsers.canonical(&token);
                    run.namespace.insert(dest.clone(), Value::Str(canonical));
                }
                let mut next_chain = chain.to_vec();
                next_chain.push(self);
                child.parse_tokens(&tokens[index + 1..], run, &next_chain)?;
                break;
            }
            pending.push((index, token));
            index += 1;
        }
        self.assign_positionals(&pending, run)?;
        Ok(())
    }

    /// 消费一个选项: 查名与别名、接上短选项粘连的值、按 action 处理帮助/版本/计数, 需要值时取值。
    fn consume_option<'a>(
        &'a self,
        tokens: &mut [String],
        index: usize,
        run: &mut Run<'a>,
        chain: &[&'a Parser],
    ) -> Result<usize, ParseError> {
        let token = tokens[index].clone();
        let (name, inline) = split_option(&token);
        let name = name.to_string();
        let inline = inline.map(str::to_string);
        let mut arg = self.lookup(&name, chain)?;
        let mut inline = inline;
        if arg.is_none() {
            match self.attached_short(&name, chain)? {
                Some((found, value)) => {
                    arg = Some(found);
                    inline = Some(value);
                }
                None => {
                    if run.tolerant {
                        run.extras.push((index, token));
                        return Ok(index + 1);
                    }
                    return Err(self.unknown_argument(&name, chain).into());
                }
            }
        }
        let Some(arg) = arg else {
            return Err(self.unknown_argument(&name, chain).into());
        };
        if !arg.takes_values() {
            if let Some(inline) = &inline {
                return Err(self
                    .fail(
                        format!(
                            "unexpected value '{inline}' for '{}' found; no more were expected",
                            arg.error_name()
                        ),
                        ErrorKind::InvalidValue,
                    )
                    .into());
            }
            match arg.action {
                Action::Help => {
                    return Err(ParseError::Help {
                        long: name.starts_with("--"),
                        prog: self.prog.clone(),
                    });
                }
                Action::Version => {
                    return Err(ParseError::Version {
                        text: self.version_text(),
                    });
                }
                _ => {
                    self.apply_values(arg, Vec::new(), run)?;
                    return Ok(index + 1);
                }
            }
        }
        if let Some(inline) = inline {
            let values = self.convert_texts(arg, &[inline])?;
            self.apply_values(arg, values, run)?;
            return Ok(index + 1);
        }
        let (stop, raw) = self.take_values(arg, tokens, index + 1)?;
        let values = self.convert_texts(arg, &raw)?;
        self.apply_values(arg, values, run)?;
        Ok(stop)
    }

    /// 把 `-abc` 拆成 `-a -b -c`; 遇到需要值的选项时, 剩余字符即它的值。
    fn expand_cluster<'a>(
        &'a self,
        token: &str,
        chain: &[&'a Parser],
    ) -> Result<Option<Vec<String>>, ParseError> {
        if token.starts_with("--") || token.chars().count() <= 2 || token.contains('=') {
            return Ok(None);
        }
        let chars: Vec<char> = token.chars().collect();
        let Some(first) = self.lookup(&format!("-{}", chars[1]), chain)? else {
            return Ok(None);
        };
        if first.takes_values() {
            return Ok(None);
        }
        let mut expanded: Vec<String> = Vec::new();
        let mut index = 1usize;
        while index < chars.len() {
            let piece = format!("-{}", chars[index]);
            match self.lookup(&piece, chain)? {
                None => {
                    // 未认出的尾巴连短横线一起留下, 由后续识别失败报出具体的短选项
                    let rest: String = chars[index..].iter().collect();
                    expanded.push(format!("-{rest}"));
                    break;
                }
                Some(arg) => {
                    if arg.takes_values() {
                        let rest: String = chars[index + 1..].iter().collect();
                        if rest.is_empty() {
                            expanded.push(piece);
                        } else {
                            let value = rest.strip_prefix('=').unwrap_or(&rest);
                            expanded.push(format!("{piece}={value}"));
                        }
                        break;
                    }
                    expanded.push(piece);
                    index += 1;
                }
            }
        }
        Ok(Some(expanded))
    }

    /// 待定的位置参数够数后才认子命令, 否则同名的词仍按位置参数吃。
    fn subcommand_of<'a>(&'a self, token: &str, pending: &[(usize, String)]) -> Option<&'a Parser> {
        let subparsers = self.subparsers.as_ref()?;
        let child = subparsers.lookup(token)?;
        if pending.len() >= self.positional_minimum() {
            Some(child)
        } else {
            None
        }
    }

    /// 未定义 `help` 子命令时, `help` 走内置帮助。
    fn help_subcommand_of(&self, token: &str, pending: &[(usize, String)]) -> bool {
        if token != "help" {
            return false;
        }
        let Some(subparsers) = self.subparsers.as_ref() else {
            return false;
        };
        if subparsers.has_command("help") {
            return false;
        }
        pending.len() >= self.positional_minimum()
    }

    /// 顺着 `help` 之后的词找到目标命令; 找不到就报用法错误。
    fn help_target<'a>(&'a self, rest: &[String]) -> Result<&'a Parser, ParseError> {
        let mut target: &'a Parser = self;
        for token in rest {
            let child = target
                .subparsers
                .as_ref()
                .and_then(|subparsers| subparsers.lookup(token));
            match child {
                Some(child) => target = child,
                None => {
                    return Err(target
                        .fail(
                            format!("unrecognized subcommand '{token}'"),
                            ErrorKind::UnknownArgument,
                        )
                        .into());
                }
            }
        }
        Ok(target)
    }

    /// 位置参数至少要吃掉几个词, 按 `nargs` 取。
    fn positional_minimum(&self) -> usize {
        self.positionals
            .iter()
            .map(|index| self.arguments[*index].min_count() as usize)
            .sum()
    }

    /// `REMAINDER` 是否已经可以开始收集。
    fn remainder_ready(&self, pending: &[(usize, String)]) -> bool {
        for (position, index) in self.positionals.iter().enumerate() {
            if self.arguments[*index].nargs != Nargs::Remainder {
                continue;
            }
            let needed: usize = self.positionals[..position]
                .iter()
                .map(|item| self.arguments[*item].min_count() as usize)
                .sum();
            return pending.len() >= needed;
        }
        false
    }

    /// 按名字或别名查参数; 未命中且在容忍模式下尝试唯一前缀匹配。
    fn lookup<'a>(
        &'a self,
        name: &str,
        chain: &[&'a Parser],
    ) -> Result<Option<&'a Argument>, ParseError> {
        if let Some(index) = self.names.get(name) {
            return Ok(Some(&self.arguments[*index]));
        }
        if self.allow_abbrev
            && name.starts_with("--")
            && name.len() > 2
            && let Some(arg) = self.abbreviate(name)?
        {
            return Ok(Some(arg));
        }
        for parent in chain.iter().rev() {
            if let Some(index) = parent.names.get(name) {
                let arg = &parent.arguments[*index];
                if arg.global {
                    return Ok(Some(arg));
                }
            }
        }
        Ok(None)
    }

    /// 按前缀找唯一匹配: `--ver` 能落到 `--verbose`, 有歧义则列候选报错。
    fn abbreviate(&self, name: &str) -> Result<Option<&Argument>, ParseError> {
        let mut matched: BTreeMap<&str, &Argument> = BTreeMap::new();
        for (option, index) in &self.names {
            if option.starts_with("--") && option.starts_with(name) {
                let arg = &self.arguments[*index];
                matched.entry(arg.dest.as_str()).or_insert(arg);
            }
        }
        if matched.is_empty() {
            return Ok(None);
        }
        if matched.len() > 1 {
            let options: Vec<&str> = self
                .names
                .keys()
                .filter(|option| option.starts_with(name))
                .map(String::as_str)
                .collect();
            return Err(self
                .fail(
                    format!("ambiguous argument '{name}'"),
                    ErrorKind::AmbiguousArgument,
                )
                .with_lines([format!("could be one of: {}", format_items(&options))])
                .into());
        }
        Ok(matched.into_values().next())
    }

    /// 短选项粘连形式 `-ovalue` 的拆分: 返回选项名与粘连上去的值。
    fn attached_short<'a>(
        &'a self,
        name: &str,
        chain: &[&'a Parser],
    ) -> Result<Option<(&'a Argument, String)>, ParseError> {
        if name.starts_with("--") {
            return Ok(None);
        }
        let chars: Vec<char> = name.chars().collect();
        if chars.len() <= 2 {
            return Ok(None);
        }
        let head: String = chars[..2].iter().collect();
        let Some(arg) = self.lookup(&head, chain)? else {
            return Ok(None);
        };
        if !arg.takes_values() {
            return Ok(None);
        }
        let value: String = chars[2..].iter().collect();
        Ok(Some((
            arg,
            value.strip_prefix('=').unwrap_or(&value).to_string(),
        )))
    }

    /// 按 `nargs` 连续取词作为选项值; 取不够时走 `fail_missing_value`。
    fn take_values(
        &self,
        arg: &Argument,
        tokens: &[String],
        start: usize,
    ) -> Result<(usize, Vec<String>), ParseError> {
        if arg.nargs == Nargs::Remainder {
            return Ok((tokens.len(), tokens[start..].to_vec()));
        }
        let minimum = arg.min_count() as usize;
        let maximum = arg.max_count().map(|count| count as usize);
        let mut taken: Vec<String> = Vec::new();
        let mut index = start;
        while index < tokens.len() {
            if maximum.is_some_and(|max| taken.len() >= max) {
                break;
            }
            if Self::is_option_token(&tokens[index]) && !arg.allow_hyphen_values {
                break;
            }
            taken.push(tokens[index].clone());
            index += 1;
        }
        if taken.len() < minimum {
            return Err(self.fail_missing_value(arg, minimum, taken.len()).into());
        }
        Ok((index, taken))
    }

    /// 构造缺值错误, 并带上该选项的用法提示。
    fn fail_missing_value(&self, arg: &Argument, needed: usize, given: usize) -> UsageError {
        let name = arg.error_name();
        if needed <= 1 {
            return self.fail(
                format!("a value is required for '{name}' but none was supplied"),
                ErrorKind::MissingValue,
            );
        }
        let verb = if given == 1 { "was" } else { "were" };
        self.fail(
            format!("{needed} values required for '{name}' but {given} {verb} provided"),
            ErrorKind::MissingValue,
        )
    }

    /// 把取到的原始文本逐个交给 `convert`。
    fn convert_texts(&self, arg: &Argument, raw: &[String]) -> Result<Vec<Value>, ParseError> {
        let mut texts: Vec<&str> = Vec::new();
        for text in raw {
            match &arg.value_delimiter {
                Some(delimiter) => texts.extend(text.split(delimiter.as_str())),
                None => texts.push(text),
            }
        }
        texts
            .into_iter()
            .map(|text| self.convert(arg, text))
            .collect()
    }

    /// 把一段文本按参数类型转成值; 转换失败报用法错误。
    fn convert(&self, arg: &Argument, text: &str) -> Result<Value, ParseError> {
        let mut value = Value::Str(text.to_string());
        if let Some(parser) = &arg.value_parser {
            value = parser.parse(text).map_err(|reason| {
                self.fail(
                    format!("invalid value '{text}' for '{}'", arg.error_name()),
                    ErrorKind::InvalidValue,
                )
                .with_reason(reason)
            })?;
        }
        if let Some(choices) = arg.choice_values()
            && !choices.contains(&value)
        {
            let labels: Vec<String> = choices.iter().map(Value::to_string).collect();
            let mut tips: Vec<String> = Vec::new();
            if let Some(suggestion) = best_match(text, &labels) {
                tips.push(format!("a similar value exists: '{suggestion}'"));
            }
            return Err(self
                .fail(
                    format!("invalid value '{text}' for '{}'", arg.error_name()),
                    ErrorKind::InvalidValue,
                )
                .with_lines([format!("[possible values: {}]", format_items(&labels))])
                .with_tips(tips)
                .into());
        }
        Ok(value)
    }

    /// 把转换好的值写进命名空间, 按 action 决定追加、合并还是覆盖。
    fn apply_values(
        &self,
        arg: &Argument,
        values: Vec<Value>,
        run: &mut Run<'_>,
    ) -> Result<(), ParseError> {
        match arg.action {
            Action::Help => {
                return Err(ParseError::Help {
                    long: arg.primary_name() == "--help",
                    prog: self.prog.clone(),
                });
            }
            Action::Version => {
                return Err(ParseError::Version {
                    text: self.version_text(),
                });
            }
            _ => {}
        }
        let dest = arg.dest.clone();
        run.present.insert(dest.clone());
        run.seen.push(dest.clone());
        match arg.action {
            Action::Count => {
                let base = match run.namespace.get(&dest) {
                    Some(Value::Int(value)) => *value,
                    _ => match implicit_default(arg) {
                        Value::Int(value) => value,
                        _ => 0,
                    },
                };
                run.namespace.insert(dest, Value::Int(base + 1));
                Ok(())
            }
            Action::StoreTrue | Action::StoreFalse | Action::StoreConst => {
                run.namespace
                    .insert(dest, arg.const_value.clone().unwrap_or(Value::Null));
                Ok(())
            }
            Action::AppendConst => {
                append(
                    &mut run.namespace,
                    &dest,
                    arg.const_value.clone().unwrap_or(Value::Null),
                );
                Ok(())
            }
            Action::Extend => {
                let mut base = match run.namespace.get(&dest) {
                    Some(Value::List(items)) => items.clone(),
                    _ => match &arg.default {
                        DefaultValue::Value(Value::List(items)) => items.clone(),
                        _ => Vec::new(),
                    },
                };
                base.extend(values);
                run.namespace.insert(dest, Value::List(base));
                Ok(())
            }
            Action::Append => {
                let item = if arg.collects() {
                    Value::List(values)
                } else {
                    values.into_iter().next().unwrap_or(Value::Null)
                };
                append(&mut run.namespace, &dest, item);
                Ok(())
            }
            Action::Store => {
                if values.is_empty() {
                    let fallback = arg.const_value.clone().unwrap_or(Value::Null);
                    run.namespace.insert(dest, fallback);
                    return Ok(());
                }
                let value = if arg.collects() {
                    Value::List(values)
                } else {
                    values.into_iter().next().unwrap_or(Value::Null)
                };
                run.namespace.insert(dest, value);
                Ok(())
            }
            Action::Help | Action::Version => Ok(()),
        }
    }

    /// 把剩余的词分给位置参数: 先按 `nargs` 定量, 剩下的归 `REMAINDER`。
    fn assign_positionals<'a>(
        &'a self,
        pending: &[(usize, String)],
        run: &mut Run<'a>,
    ) -> Result<(), ParseError> {
        let tokens: Vec<&str> = pending.iter().map(|(_, token)| token.as_str()).collect();
        let total = tokens.len();
        let mut cursor = 0usize;
        for (position, index) in self.positionals.iter().enumerate() {
            let arg = &self.arguments[*index];
            let tail: usize = self.positionals[position + 1..]
                .iter()
                .map(|item| self.arguments[*item].min_count() as usize)
                .sum();
            let available = total.saturating_sub(cursor).saturating_sub(tail);
            let take = match arg.nargs {
                Nargs::Remainder => total - cursor,
                _ => match arg.max_count() {
                    None => available,
                    Some(max) => (max as usize).min(available),
                },
            };
            if take < arg.min_count() as usize {
                run.missing.push(arg);
                continue;
            }
            let chunk = &tokens[cursor..cursor + take];
            cursor += take;
            self.store_positional(arg, chunk, run)?;
        }
        if cursor < total {
            run.extras.extend(pending[cursor..].iter().cloned());
        }
        Ok(())
    }

    /// 把一批词写进某个位置参数的 `dest`, 同时记入出现记录。
    fn store_positional(
        &self,
        arg: &Argument,
        chunk: &[&str],
        run: &mut Run<'_>,
    ) -> Result<(), ParseError> {
        if chunk.is_empty() {
            let value = match (&arg.const_value, &arg.default) {
                (Some(value), _) => value.clone(),
                (None, DefaultValue::Value(value)) => value.clone(),
                (None, _) if matches!(arg.nargs, Nargs::ZeroOrMore | Nargs::Remainder) => {
                    Value::List(Vec::new())
                }
                _ => Value::Null,
            };
            run.namespace.insert(arg.dest.clone(), value);
            return Ok(());
        }
        run.present.insert(arg.dest.clone());
        run.seen.push(arg.dest.clone());
        let raw: Vec<String> = chunk.iter().map(|text| (*text).to_string()).collect();
        let values = self.convert_texts(arg, &raw)?;
        let value = if arg.collects() {
            Value::List(values)
        } else {
            values.into_iter().next().unwrap_or(Value::Null)
        };
        run.namespace.insert(arg.dest.clone(), value);
        Ok(())
    }

    /// 收尾: 取出子命令结果, 处理未满足的必填项与未归位的多余词。
    fn finish<'a>(&'a self, run: &mut Run<'a>) -> Result<(), ParseError> {
        self.apply_overrides(run)?;
        self.apply_env(run)?;
        self.validate(run)?;
        self.apply_defaults(run);
        Ok(())
    }

    /// `overrides_with` 双向生效: 一对里命令行上后出现的留下, 先出现的退回默认值。
    fn apply_overrides<'a>(&'a self, run: &mut Run<'a>) -> Result<(), ParseError> {
        let parsers: Vec<&'a Parser> = run.parsers.clone();
        let mut registry: BTreeMap<&'a str, &'a Argument> = BTreeMap::new();
        let mut by_name: BTreeMap<&'a str, &'a Argument> = BTreeMap::new();
        for parser in &parsers {
            for arg in &parser.arguments {
                if matches!(arg.action, Action::Help | Action::Version) {
                    continue;
                }
                registry.insert(&arg.dest, arg);
            }
            for (name, index) in &parser.names {
                by_name.insert(name.as_str(), &parser.arguments[*index]);
            }
        }

        let mut pairs: BTreeMap<&'a str, BTreeSet<&'a str>> = BTreeMap::new();
        for (dest, arg) in &registry {
            for reference in &arg.overrides_with {
                let other = match registry.get_key_value(reference.as_str()) {
                    Some((key, _)) => *key,
                    None => match by_name.get(reference.as_str()) {
                        Some(found) => found.dest.as_str(),
                        None => {
                            return Err(ParseError::Definition {
                                message: format!(
                                    "overrides_with / conflicts_with / requires 指向了未登记的参数: {reference:?}"
                                ),
                            });
                        }
                    },
                };
                pairs.entry(dest).or_default().insert(other);
                pairs.entry(other).or_default().insert(dest);
            }
        }
        if pairs.is_empty() {
            return Ok(());
        }

        let mut kept: BTreeSet<&str> = BTreeSet::new();
        for dest in &run.seen {
            if let Some(others) = pairs.get(dest.as_str()) {
                for other in others {
                    kept.remove(other);
                }
            }
            kept.insert(dest.as_str());
        }
        let losers: Vec<String> = run
            .seen
            .iter()
            .filter(|dest| !kept.contains(dest.as_str()))
            .cloned()
            .collect();
        for dest in losers {
            run.present.remove(&dest);
            run.namespace.remove(&dest);
        }
        Ok(())
    }

    /// 按 `env` 声明的环境变量补值; 命令行已出现的选项不覆盖。
    fn apply_env<'a>(&'a self, run: &mut Run<'a>) -> Result<(), ParseError> {
        let parsers: Vec<&'a Parser> = run.parsers.clone();
        for parser in parsers {
            for arg in &parser.arguments {
                let Some(key) = &arg.env else {
                    continue;
                };
                if run.present.contains(&arg.dest) {
                    continue;
                }
                let Ok(text) = env::var(key) else {
                    continue;
                };
                if arg.takes_values() {
                    let values = parser.convert_texts(arg, &[text])?;
                    parser.apply_values(arg, values, run)?;
                    continue;
                }
                let enabled = match parse_bool().parse(&text) {
                    Ok(Value::Bool(value)) => value,
                    Ok(_) => false,
                    Err(reason) => {
                        return Err(parser
                            .fail(
                                format!("invalid value '{text}' for '{}'", arg.error_name()),
                                ErrorKind::InvalidValue,
                            )
                            .with_reason(reason)
                            .into());
                    }
                };
                let fallback = match &arg.default {
                    DefaultValue::Value(value) => value.clone(),
                    _ => Value::Bool(false),
                };
                let value = if enabled {
                    arg.const_value.clone().unwrap_or(Value::Bool(true))
                } else {
                    fallback
                };
                run.namespace.insert(arg.dest.clone(), value);
                run.present.insert(arg.dest.clone());
            }
        }
        Ok(())
    }

    /// 跑完所有校验: 必填、`choices`、自定义校验函数, 按注册顺序报第一个错误。
    fn validate<'a>(&'a self, run: &mut Run<'a>) -> Result<(), ParseError> {
        let parsers: Vec<&'a Parser> = run.parsers.clone();
        let Some(leaf) = parsers.last().copied() else {
            return Ok(());
        };
        let mut registry: BTreeMap<&str, &'a Argument> = BTreeMap::new();
        let mut by_name: BTreeMap<&str, &'a Argument> = BTreeMap::new();
        for parser in &parsers {
            for arg in &parser.arguments {
                // 帮助 / 版本只在解析途中返回控制流, 从不落进命名空间, 各层同名不冲突
                if matches!(arg.action, Action::Help | Action::Version) {
                    continue;
                }
                if registry.contains_key(arg.dest.as_str()) {
                    return Err(ParseError::Definition {
                        message: format!(
                            "命令链上的 dest 重复: {:?} (父命令与子命令不能共用 dest)",
                            arg.dest
                        ),
                    });
                }
                registry.insert(&arg.dest, arg);
            }
            for (name, index) in &parser.names {
                by_name.insert(name.as_str(), &parser.arguments[*index]);
            }
        }

        let resolve = |reference: &str| -> Result<&'a str, ParseError> {
            if let Some((key, _)) = registry.get_key_value(reference) {
                return Ok(key);
            }
            match by_name.get(reference) {
                Some(arg) => Ok(arg.dest.as_str()),
                None => Err(ParseError::Definition {
                    message: format!(
                        "overrides_with / conflicts_with / requires 指向了未登记的参数: {reference:?}"
                    ),
                }),
            }
        };

        for (dest, arg) in &registry {
            if !run.present.contains(*dest) {
                continue;
            }
            for reference in &arg.conflicts_with {
                let other = resolve(reference)?;
                if run.present.contains(other) {
                    let label = registry[other].error_name();
                    return Err(leaf
                        .fail(
                            format!(
                                "the argument '{}' cannot be used with '{label}'",
                                arg.error_name()
                            ),
                            ErrorKind::Conflict,
                        )
                        .into());
                }
            }
        }

        for parser in &parsers {
            for group in &parser.mutex {
                let hits: Vec<&Argument> = group
                    .members
                    .iter()
                    .map(|index| &parser.arguments[*index])
                    .filter(|arg| run.present.contains(&arg.dest))
                    .collect();
                if hits.len() > 1 {
                    return Err(leaf
                        .fail(
                            format!(
                                "the argument '{}' cannot be used with '{}'",
                                hits[0].error_name(),
                                hits[1].error_name()
                            ),
                            ErrorKind::Conflict,
                        )
                        .into());
                }
                if hits.is_empty() && group.required {
                    let labels: Vec<String> = group
                        .members
                        .iter()
                        .map(|index| parser.arguments[*index].error_name())
                        .collect();
                    return Err(leaf
                        .fail(
                            "one of the following arguments is required:",
                            ErrorKind::MissingRequired,
                        )
                        .with_lines(labels)
                        .into());
                }
            }
        }

        for parser in &parsers {
            for group in &parser.value_groups {
                if !group.required {
                    continue;
                }
                let mut hit = false;
                for index in &group.members {
                    if run.present.contains(&parser.arguments[*index].dest) {
                        hit = true;
                        break;
                    }
                }
                if hit {
                    continue;
                }
                return Err(leaf
                    .fail(
                        "the following required arguments were not provided:",
                        ErrorKind::MissingRequired,
                    )
                    .with_lines([format!("<{}>", group.name)])
                    .into());
            }
        }

        for (dest, arg) in &registry {
            if !run.present.contains(*dest) {
                continue;
            }
            for reference in &arg.requires {
                let other = resolve(reference)?;
                if !run.present.contains(other) {
                    let label = registry[other].error_name();
                    return Err(leaf
                        .fail(
                            format!("the argument '{}' requires '{label}'", arg.error_name()),
                            ErrorKind::MissingRequired,
                        )
                        .into());
                }
            }
        }

        let mut missing: Vec<&'a Argument> = run.missing.clone();
        for parser in &parsers {
            if parser.subcommand_negates_reqs && run.subcommand.is_some() {
                continue;
            }
            for arg in &parser.arguments {
                if run.present.contains(&arg.dest) {
                    continue;
                }
                let mut needed = arg.required;
                if !needed && !arg.required_unless_present.is_empty() {
                    let mut satisfied = false;
                    for reference in &arg.required_unless_present {
                        if run.present.contains(resolve(reference)?) {
                            satisfied = true;
                            break;
                        }
                    }
                    needed = !satisfied;
                }
                if needed {
                    missing.push(arg);
                }
            }
            if std::ptr::eq(*parser, leaf)
                && let Some(subparsers) = &parser.subparsers
                && subparsers.required
            {
                return Err(leaf
                    .fail(
                        "the following required arguments were not provided:",
                        ErrorKind::MissingRequired,
                    )
                    .with_lines(["<COMMAND>"])
                    .into());
            }
        }
        if !missing.is_empty() {
            let mut labels: Vec<String> = Vec::new();
            for arg in &missing {
                let label = arg.error_name();
                if !labels.contains(&label) {
                    labels.push(label);
                }
            }
            return Err(leaf
                .fail(
                    "the following required arguments were not provided:",
                    ErrorKind::MissingRequired,
                )
                .with_lines(labels)
                .into());
        }
        Ok(())
    }

    /// 给没出现的选项写默认值; 默认值按类型转换, 转不动就报错。
    fn apply_defaults<'a>(&'a self, run: &mut Run<'a>) {
        let parsers: Vec<&'a Parser> = run.parsers.clone();
        for parser in parsers {
            for arg in &parser.arguments {
                if run.namespace.contains(&arg.dest) {
                    continue;
                }
                let value = match &arg.default {
                    DefaultValue::Value(value) => value.clone(),
                    DefaultValue::Suppress => continue,
                    DefaultValue::Missing => match &parser.argument_default {
                        DefaultValue::Value(value) => value.clone(),
                        DefaultValue::Suppress => continue,
                        DefaultValue::Missing => implicit_default(arg),
                    },
                };
                run.namespace.insert(arg.dest.clone(), value);
            }
        }
    }

    /// 构造未知参数错误; 容忍模式下改为收集到 `extras`。
    fn unknown_argument<'a>(&'a self, name: &str, chain: &[&'a Parser]) -> UsageError {
        let mut candidates: Vec<&str> = self.names.keys().map(String::as_str).collect();
        for parent in chain {
            candidates.extend(
                parent
                    .names
                    .iter()
                    .filter(|(_, index)| parent.arguments[**index].global)
                    .map(|(option, _)| option.as_str()),
            );
        }
        let mut tips: Vec<String> = Vec::new();
        match best_match(name, &candidates) {
            Some(suggestion) => tips.push(format!("a similar argument exists: '{suggestion}'")),
            None if name.starts_with("--") => {
                tips.push(format!("to pass '{name}' as a value, use '-- {name}'"));
            }
            None => {}
        }
        self.fail(
            format!("unexpected argument '{name}' found"),
            ErrorKind::UnknownArgument,
        )
        .with_tips(tips)
    }

    /// 构造用法错误并结束本次解析。
    fn fail(&self, message: impl Into<String>, kind: ErrorKind) -> UsageError {
        let mut error = UsageError::new(message)
            .with_kind(kind)
            .with_usage(self.usage_spec());
        if !self.dests.contains_key("help") {
            error = error.without_hint();
        }
        error
    }

    /// 整段帮助文本 (不含末尾换行)。
    pub fn format_help(&self, long: bool) -> String {
        super::help::render_help(self, long)
    }

    /// 单行用法文本 (`Usage:` 前缀在内)。
    pub fn format_usage(&self) -> String {
        super::help::render_usage(self)
    }

    /// 把帮助写到流, 末尾补一个换行。
    pub fn print_help(&self, long: bool, writer: &mut dyn std::io::Write) -> std::io::Result<()> {
        writeln!(writer, "{}", self.format_help(long))
    }

    /// 解析并落到进程出口: 帮助 / 版本写 `stdout` 退出码 0, 用法错误写 `stderr` 退出码 2。
    ///
    /// 返回 `Err(码)` 表示文本已经打印完毕, 调用方据此退出进程; 定义期错误照 Python 侧一样
    /// 直接 panic, 不退化成一次正常退出。
    pub fn parse_or_exit(
        &self,
        argv: &[String],
        stdout: &mut dyn std::io::Write,
        stderr: &mut dyn std::io::Write,
    ) -> std::io::Result<Result<Namespace, i32>> {
        match self.parse(argv) {
            Ok(namespace) => Ok(Ok(namespace)),
            Err(ParseError::Help { long, prog }) => {
                let target = self.parser_by_prog(&prog).unwrap_or(self);
                target.print_help(long, stdout)?;
                Ok(Err(0))
            }
            Err(ParseError::Version { text }) => {
                writeln!(stdout, "{text}")?;
                Ok(Err(0))
            }
            Err(ParseError::Usage(error)) => {
                writeln!(stderr, "{error}")?;
                Ok(Err(2))
            }
            Err(ParseError::Definition { message }) => panic!("{message}"),
        }
    }

    /// 按命令全名定位到命令本身, 供帮助渲染找到请求帮助的那条子命令。
    fn parser_by_prog<'a>(&'a self, prog: &str) -> Option<&'a Parser> {
        if self.prog == prog {
            return Some(self);
        }
        for child in &self.subparsers.as_ref()?.commands {
            if let Some(found) = child.parser_by_prog(prog) {
                return Some(found);
            }
        }
        None
    }

    /// usage 行 (`Usage:` 之后) 的字符串形态。
    pub fn usage_spec(&self) -> String {
        if let Some(usage) = &self.usage {
            return usage.clone();
        }
        let mut parts = vec![self.prog.clone()];
        for arg in &self.arguments {
            if arg.required && !arg.hide {
                parts.push(arg.error_name());
            }
        }
        let has_optional = self.arguments.iter().any(|arg| {
            !arg.hide
                && !arg.required
                && !arg.is_positional()
                && !matches!(arg.action, Action::Help | Action::Version)
        });
        if has_optional {
            parts.push("[OPTIONS]".to_string());
        }
        for index in &self.positionals {
            let arg = &self.arguments[*index];
            if !arg.hide {
                parts.push(arg.usage_spec());
            }
        }
        if let Some(subparsers) = &self.subparsers {
            parts.push(
                if subparsers.required {
                    "<COMMAND>"
                } else {
                    "[COMMAND]"
                }
                .to_string(),
            );
        }
        parts.join(" ")
    }

    /// 命令的主名; 匿名命令没有主名。
    fn primary_name(&self) -> Option<&str> {
        self.name.as_deref()
    }
}

/// 一次解析的共享状态: 命名空间、出现记录、参与的命令链与待报缺口。
struct Run<'a> {
    tolerant: bool,
    namespace: Namespace,
    present: BTreeSet<String>,
    /// 命令行上出现的 dest, 按出现顺序 (覆盖判定用)。
    seen: Vec<String>,
    parsers: Vec<&'a Parser>,
    missing: Vec<&'a Argument>,
    extras: Vec<(usize, String)>,
    subcommand: Option<String>,
}

impl<'a> Run<'a> {
    /// 按容忍模式新建一次解析的共享状态。
    fn new(tolerant: bool) -> Self {
        Self {
            tolerant,
            namespace: Namespace::new(),
            present: BTreeSet::new(),
            seen: Vec::new(),
            parsers: Vec::new(),
            missing: Vec::new(),
            extras: Vec::new(),
            subcommand: None,
        }
    }

    /// 按收集顺序返回多余词, 并清空缓冲区。
    fn sorted_extras(&mut self) -> Vec<String> {
        self.extras.sort_by_key(|(slot, _)| *slot);
        std::mem::take(&mut self.extras)
            .into_iter()
            .map(|(_, token)| token)
            .collect()
    }
}

/// `@file` 展开的嵌套层数上限; 只用来挡住自引用, 正常用法远到不了。
const MAX_INCLUDE_DEPTH: usize = 16;

/// 按 LF 拆行, 行尾的 CR 去掉, 末尾那个空元素不算一行。
fn split_lines(body: &str) -> Vec<String> {
    let mut lines: Vec<String> = body
        .split('\n')
        .map(|line| line.strip_suffix('\r').unwrap_or(line).to_string())
        .collect();
    if lines.last().is_some_and(|line| line.is_empty()) {
        lines.pop();
    }
    lines
}

/// 给一段 token 配扫描序号; 序号只用于把 `extras` 还原成命令行顺序。
fn slots(tokens: &[String], start: usize) -> Vec<(usize, String)> {
    tokens
        .iter()
        .enumerate()
        .map(|(offset, token)| (start + offset, token.clone()))
        .collect()
}

/// 把 `--key=value` / `-k=value` 拆成 (名字, 内联取值)。
fn split_option(token: &str) -> (&str, Option<&str>) {
    match token.split_once('=') {
        Some((name, value)) => (name, Some(value)),
        None => (token, None),
    }
}

/// 签名数字: 可选的 `+` / `-` 号加整数或小数, 整个 token 当一个取值参数 (`-1`、`-.5`、`+3`、`-1.5`)。
fn is_signed_number(token: &str) -> bool {
    let Some(body) = token.strip_prefix('-').or_else(|| token.strip_prefix('+')) else {
        return false;
    };
    if body.is_empty() {
        return false;
    }
    match body.split_once('.') {
        Some((int, frac)) => {
            int.chars().all(|char| char.is_ascii_digit())
                && !frac.is_empty()
                && frac.chars().all(|char| char.is_ascii_digit())
        }
        None => body.chars().all(|char| char.is_ascii_digit()),
    }
}

/// 由选项名推 `dest`: 优先最长形态的长选项, 中划线转下划线。
fn dest_of_option(names: &[String]) -> String {
    for name in names {
        if let Some(body) = name.strip_prefix("--") {
            return body.replace('-', "_");
        }
    }
    names
        .first()
        .map(|name| name.trim_start_matches('-').replace('-', "_"))
        .unwrap_or_default()
}

/// 位置参数的 `dest`: 名字原样, 中划线转下划线。
fn dest_of_positional(name: &str) -> String {
    name.replace('-', "_")
}

/// 没显式给默认值时的落值。
fn implicit_default(arg: &Argument) -> Value {
    match arg.action {
        Action::StoreTrue => Value::Bool(false),
        Action::StoreFalse => Value::Bool(true),
        Action::Count => Value::Int(0),
        _ if arg.is_positional() && matches!(arg.nargs, Nargs::ZeroOrMore | Nargs::Remainder) => {
            Value::List(Vec::new())
        }
        _ => Value::Null,
    }
}

/// 按默认值的形态推断取值转换器。
fn infer_parser(default: &Value) -> Option<ValueParser> {
    match default {
        Value::Bool(_) => Some(parse_bool()),
        Value::Int(_) => Some(parse_int(None, None)),
        Value::Float(_) => Some(parse_float(None, None)),
        _ => None,
    }
}

/// 往命名空间里的列表追加一项; 首次出现时新开列表。
fn append(namespace: &mut Namespace, dest: &str, item: Value) {
    let mut items = match namespace.get(dest) {
        Some(Value::List(items)) => items.clone(),
        _ => Vec::new(),
    };
    items.push(item);
    namespace.insert(dest.to_string(), Value::List(items));
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::base::argparser::spec::Choices;

    fn argv(items: &[&str]) -> Vec<String> {
        items.iter().map(|item| (*item).to_string()).collect()
    }

    fn text(namespace: &Namespace, key: &str) -> String {
        namespace.get(key).map(Value::to_string).unwrap_or_default()
    }

    fn usage_error(error: ParseError) -> UsageError {
        match error {
            ParseError::Usage(error) => *error,
            other => panic!("应当是用法错误, 实际是 {other:?}"),
        }
    }

    fn base() -> Parser {
        let mut parser = Parser::new("demo").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["-v", "--verbose"],
                Argument {
                    action: Action::Count,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["-o", "--output"],
                Argument {
                    required: true,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--level"],
                Argument {
                    choices: Some(Choices::Values(vec![
                        Value::Str("debug".to_string()),
                        Value::Str("info".to_string()),
                        Value::Str("warn".to_string()),
                    ])),
                    default: DefaultValue::Value(Value::Str("info".to_string())),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["files"],
                Argument {
                    nargs: Nargs::OneOrMore,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
    }

    fn tool() -> Parser {
        let mut parser = Parser::new("tool")
            .with_version("9.9.9")
            .with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["-q", "--quiet"],
                Argument {
                    action: Action::StoreTrue,
                    global: true,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_subparsers(Some("command"), "Commands", true)
            .unwrap();
        {
            let build = parser
                .subparsers_mut()
                .unwrap()
                .add_parser("build", "构建", &[], &["b"])
                .unwrap();
            build
                .add_argument(
                    &["-j", "--jobs"],
                    Argument {
                        value_parser: Some(parse_int(None, None)),
                        default: DefaultValue::Value(Value::Int(4)),
                        ..Argument::default()
                    },
                )
                .unwrap();
            let targets = build
                .add_subparsers(Some("target"), "Targets", false)
                .unwrap();
            targets.add_parser("release", "发布版", &[], &[]).unwrap();
            targets.add_parser("debug", "调试版", &[], &[]).unwrap();
        }
        {
            let run = parser
                .subparsers_mut()
                .unwrap()
                .add_parser("run", "运行", &["r"], &[])
                .unwrap();
            run.add_argument(&["script"], Argument::default()).unwrap();
        }
        parser
    }

    #[test]
    fn options_take_values_in_every_form() {
        let parser = base();
        let namespace = parser
            .parse(&argv(&[
                "-o",
                "out.txt",
                "-vv",
                "--level=warn",
                "a.txt",
                "b.txt",
            ]))
            .unwrap();
        assert_eq!(text(&namespace, "output"), "out.txt");
        assert_eq!(text(&namespace, "verbose"), "2");
        assert_eq!(text(&namespace, "level"), "warn");
        assert_eq!(text(&namespace, "files"), "a.txt, b.txt");

        let glued = parser.parse(&argv(&["-oout.txt", "a.txt"])).unwrap();
        assert_eq!(text(&glued, "output"), "out.txt");
        assert_eq!(text(&glued, "verbose"), "0");
        assert_eq!(text(&glued, "level"), "info");
    }

    #[test]
    fn missing_required_option_reports_usage_and_hint() {
        let error = usage_error(base().parse(&argv(&["a.txt"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::MissingRequired);
        let rendered = error.to_string();
        assert!(
            rendered.contains("the following required arguments were not provided"),
            "{rendered}"
        );
        assert!(rendered.contains("--output <OUTPUT>"), "{rendered}");
        assert!(
            rendered.contains("Usage: demo --output <OUTPUT> [OPTIONS] <FILES>..."),
            "{rendered}"
        );
        assert!(
            rendered.ends_with("For more information, try '--help'."),
            "{rendered}"
        );
    }

    #[test]
    fn missing_value_and_invalid_choice_report_their_kinds() {
        let parser = base();
        let error = usage_error(parser.parse(&argv(&["-o"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::MissingValue);
        assert!(
            error
                .to_string()
                .contains("a value is required for '--output <OUTPUT>' but none was supplied"),
            "{error}"
        );

        let error = usage_error(
            parser
                .parse(&argv(&["-o", "x", "--level", "warnn", "a.txt"]))
                .unwrap_err(),
        );
        assert_eq!(error.kind, ErrorKind::InvalidValue);
        let rendered = error.to_string();
        assert!(
            rendered.contains("invalid value 'warnn' for '--level <LEVEL>'"),
            "{rendered}"
        );
        assert!(
            rendered.contains("[possible values: debug, info, warn]"),
            "{rendered}"
        );
        assert!(
            rendered.contains("a similar value exists: 'warn'"),
            "{rendered}"
        );
    }

    #[test]
    fn unknown_option_suggests_and_abbreviation_resolves() {
        let parser = base();
        let rendered = usage_error(
            parser
                .parse(&argv(&["-o", "x", "--verbsoe", "a.txt"]))
                .unwrap_err(),
        )
        .to_string();
        assert!(
            rendered.contains("unexpected argument '--verbsoe' found"),
            "{rendered}"
        );
        assert!(
            rendered.contains("a similar argument exists: '--verbose'"),
            "{rendered}"
        );

        let resolved = parser
            .parse(&argv(&["-o", "x", "--verbos", "a.txt"]))
            .unwrap();
        assert_eq!(text(&resolved, "verbose"), "1");

        let rendered = usage_error(
            parser
                .parse(&argv(&["-o", "x", "--zzz", "a.txt"]))
                .unwrap_err(),
        )
        .to_string();
        assert!(
            rendered.contains("to pass '--zzz' as a value, use '-- --zzz'"),
            "{rendered}"
        );
    }

    #[test]
    fn ambiguous_prefix_lists_candidates() {
        let mut parser = Parser::new("amb").with_color(ColorChoice::Never);
        parser
            .add_argument(&["--output"], Argument::default())
            .unwrap();
        parser
            .add_argument(&["--outdir"], Argument::default())
            .unwrap();
        let error = usage_error(parser.parse(&argv(&["--out", "x"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::AmbiguousArgument);
        assert!(
            error.to_string().contains("ambiguous argument '--out'"),
            "{error}"
        );
        assert!(
            error
                .to_string()
                .contains("could be one of: --outdir, --output"),
            "{error}"
        );
    }

    #[test]
    fn signed_numbers_and_double_dash_are_values() {
        let mut parser = Parser::new("neg").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["nums"],
                Argument {
                    nargs: Nargs::ZeroOrMore,
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = parser
            .parse(&argv(&["-1", "-.5", "+1", "-1.5", "+.5"]))
            .unwrap();
        assert_eq!(text(&namespace, "nums"), "-1, -.5, +1, -1.5, +.5");

        let namespace = parser.parse(&argv(&["--", "--help"])).unwrap();
        assert_eq!(text(&namespace, "nums"), "--help");
    }

    #[test]
    fn nargs_shapes_assign_values() {
        let mut parser = Parser::new("nargs").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--pair"],
                Argument {
                    nargs: Nargs::Count(2),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--maybe"],
                Argument {
                    nargs: Nargs::Optional,
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = parser
            .parse(&argv(&["--pair", "a", "b", "--maybe"]))
            .unwrap();
        assert_eq!(text(&namespace, "pair"), "a, b");
        assert_eq!(text(&namespace, "maybe"), "None");

        let mut positional = Parser::new("pos").with_color(ColorChoice::Never);
        positional
            .add_argument(
                &["src"],
                Argument {
                    nargs: Nargs::ZeroOrMore,
                    ..Argument::default()
                },
            )
            .unwrap();
        positional
            .add_argument(&["dst"], Argument::default())
            .unwrap();
        let namespace = positional.parse(&argv(&["a", "b", "c"])).unwrap();
        assert_eq!(text(&namespace, "src"), "a, b");
        assert_eq!(text(&namespace, "dst"), "c");

        let namespace = positional.parse(&argv(&[])).unwrap_err();
        let error = usage_error(namespace);
        assert_eq!(error.kind, ErrorKind::MissingRequired);
    }

    #[test]
    fn remainder_takes_the_rest_of_the_positionals() {
        let mut parser = Parser::new("rest").with_color(ColorChoice::Never);
        parser.add_argument(&["head"], Argument::default()).unwrap();
        parser
            .add_argument(
                &["rest"],
                Argument {
                    nargs: Nargs::Remainder,
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = parser.parse(&argv(&["a", "b", "c"])).unwrap();
        assert_eq!(text(&namespace, "head"), "a");
        assert_eq!(text(&namespace, "rest"), "b, c");

        // 看起来像选项的 token 在 REMAINDER 之前就被选项分支拦下, 与 Python spec 同口径
        let error = usage_error(parser.parse(&argv(&["a", "--flag"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::UnknownArgument);
    }

    #[test]
    fn store_like_actions_land_their_values() {
        let mut parser = Parser::new("actions").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--on"],
                Argument {
                    action: Action::StoreTrue,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--off"],
                Argument {
                    action: Action::StoreFalse,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--mode"],
                Argument {
                    action: Action::StoreConst,
                    const_value: Some(Value::Str("fast".to_string())),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--tag"],
                Argument {
                    action: Action::Append,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--mark"],
                Argument {
                    action: Action::AppendConst,
                    const_value: Some(Value::Int(1)),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--num"],
                Argument {
                    action: Action::Extend,
                    nargs: Nargs::OneOrMore,
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = parser
            .parse(&argv(&[
                "--on", "--off", "--mode", "--tag", "a", "--tag", "b", "--mark", "--mark", "--num",
                "1", "2",
            ]))
            .unwrap();
        assert_eq!(text(&namespace, "on"), "true");
        assert_eq!(text(&namespace, "off"), "false");
        assert_eq!(text(&namespace, "mode"), "fast");
        assert_eq!(text(&namespace, "tag"), "a, b");
        assert_eq!(text(&namespace, "mark"), "1, 1");
        assert_eq!(text(&namespace, "num"), "1, 2");

        let defaults = Parser::new("defs").with_color(ColorChoice::Never);
        let _ = defaults;
        let plain = parser.parse(&argv(&[])).unwrap();
        assert_eq!(text(&plain, "on"), "false");
        assert_eq!(text(&plain, "off"), "true");
        assert_eq!(text(&plain, "tag"), "None");
    }

    #[test]
    fn value_delimiter_splits_one_token() {
        let mut parser = Parser::new("delim").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--tag"],
                Argument {
                    action: Action::Append,
                    value_delimiter: Some(",".to_string()),
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = parser
            .parse(&argv(&["--tag", "a,b", "--tag", "c"]))
            .unwrap();
        assert_eq!(text(&namespace, "tag"), "a, b, c");
    }

    #[test]
    fn mutex_conflicts_and_requires_are_validated() {
        let mut parser = Parser::new("rules").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--fast"],
                Argument {
                    action: Action::StoreTrue,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--cache"],
                Argument {
                    action: Action::StoreTrue,
                    requires: vec!["fast".to_string()],
                    ..Argument::default()
                },
            )
            .unwrap();
        let group = parser.add_mutually_exclusive_group(false);
        parser
            .add_argument_to_group(
                group,
                &["--json"],
                Argument {
                    action: Action::StoreTrue,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument_to_group(
                group,
                &["--yaml"],
                Argument {
                    action: Action::StoreTrue,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--plain"],
                Argument {
                    action: Action::StoreTrue,
                    conflicts_with: vec!["json".to_string()],
                    ..Argument::default()
                },
            )
            .unwrap();

        let error = usage_error(parser.parse(&argv(&["--json", "--yaml"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::Conflict);
        assert!(error.to_string().contains("cannot be used with"), "{error}");

        let error = usage_error(parser.parse(&argv(&["--json", "--plain"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::Conflict);

        let error = usage_error(parser.parse(&argv(&["--cache"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::MissingRequired);
        assert!(
            error
                .to_string()
                .contains("the argument '--cache' requires '--fast'"),
            "{error}"
        );

        let required_group = parser.add_mutually_exclusive_group(true);
        parser
            .add_argument_to_group(
                required_group,
                &["--x"],
                Argument {
                    action: Action::StoreTrue,
                    ..Argument::default()
                },
            )
            .unwrap();
        let error = usage_error(parser.parse(&argv(&[])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::MissingRequired);
        assert!(
            error
                .to_string()
                .contains("one of the following arguments is required:"),
            "{error}"
        );
    }

    #[test]
    fn subcommands_route_tokens_and_globals() {
        let parser = tool();
        let namespace = parser
            .parse(&argv(&["-q", "build", "-j", "8", "release"]))
            .unwrap();
        assert_eq!(text(&namespace, "quiet"), "true");
        assert_eq!(text(&namespace, "command"), "build");
        assert_eq!(text(&namespace, "jobs"), "8");
        assert_eq!(text(&namespace, "target"), "release");

        let namespace = parser.parse(&argv(&["r", "s.py"])).unwrap();
        assert_eq!(text(&namespace, "command"), "run");
        assert_eq!(text(&namespace, "script"), "s.py");

        let namespace = parser.parse(&argv(&["b"])).unwrap();
        assert_eq!(text(&namespace, "command"), "build");
        assert_eq!(text(&namespace, "jobs"), "4");

        let error = usage_error(parser.parse(&argv(&[])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::MissingRequired);
        assert!(error.to_string().contains("<COMMAND>"), "{error}");
    }

    #[test]
    fn help_subcommand_targets_the_deepest_command() {
        let parser = tool();
        match parser.parse(&argv(&["help"])).unwrap_err() {
            ParseError::Help { long, prog } => {
                assert!(long);
                assert_eq!(prog, "tool");
            }
            other => panic!("应当是帮助请求, 实际是 {other:?}"),
        }
        match parser
            .parse(&argv(&["help", "build", "release"]))
            .unwrap_err()
        {
            ParseError::Help { long, prog } => {
                assert!(long);
                assert_eq!(prog, "tool build release");
            }
            other => panic!("应当是帮助请求, 实际是 {other:?}"),
        }
        let error = usage_error(parser.parse(&argv(&["help", "nosuch"])).unwrap_err());
        assert!(
            error
                .to_string()
                .contains("unrecognized subcommand 'nosuch'"),
            "{error}"
        );
    }

    #[test]
    fn help_and_version_flags_return_control_flow() {
        let parser = Parser::new("demo")
            .with_version("1.2.3")
            .with_color(ColorChoice::Never);
        match parser.parse(&argv(&["-h"])).unwrap_err() {
            ParseError::Help { long, prog } => {
                assert!(!long);
                assert_eq!(prog, "demo");
            }
            other => panic!("应当是帮助请求, 实际是 {other:?}"),
        }
        match parser.parse(&argv(&["--help"])).unwrap_err() {
            ParseError::Help { long, .. } => assert!(long),
            other => panic!("应当是帮助请求, 实际是 {other:?}"),
        }
        for flag in ["-V", "--version"] {
            match parser.parse(&argv(&[flag])).unwrap_err() {
                ParseError::Version { text } => assert_eq!(text, "demo 1.2.3"),
                other => panic!("应当是版本请求, 实际是 {other:?}"),
            }
        }
    }

    #[test]
    fn subcommand_inherits_version_flag() {
        let parser = tool();
        for name in ["build", "run"] {
            match parser.parse(&argv(&[name, "--version"])).unwrap_err() {
                ParseError::Version { text } => assert_eq!(text, format!("tool {name} 9.9.9")),
                other => panic!("应当是版本请求, 实际是 {other:?}"),
            }
        }
    }

    #[test]
    fn value_group_and_required_unless_present_gate_required() {
        let mut parser = Parser::new("grouped").with_color(ColorChoice::Never);
        let group = parser.add_value_group("input", true);
        parser
            .add_argument_to_value_group(group, &["--file"], Argument::default())
            .unwrap();
        parser
            .add_argument_to_value_group(group, &["--dir"], Argument::default())
            .unwrap();

        let one = parser.parse(&argv(&["--file", "x"])).unwrap();
        assert_eq!(text(&one, "file"), "x");

        let both = parser.parse(&argv(&["--file", "x", "--dir", "y"])).unwrap();
        assert_eq!(text(&both, "dir"), "y");

        let empty = usage_error(parser.parse(&argv(&[])).unwrap_err());
        assert_eq!(empty.kind, ErrorKind::MissingRequired);
        assert!(empty.to_string().contains("<input>"), "{empty}");

        let mut gated = Parser::new("gated").with_color(ColorChoice::Never);
        gated
            .add_argument(
                &["--token"],
                Argument {
                    required_unless_present: vec!["user".to_string()],
                    ..Argument::default()
                },
            )
            .unwrap();
        gated
            .add_argument(&["--user"], Argument::default())
            .unwrap();
        gated
            .add_argument(
                &["--key"],
                Argument {
                    overrides_with: vec!["user".to_string()],
                    ..Argument::default()
                },
            )
            .unwrap();

        assert!(gated.parse(&argv(&["--user", "u"])).is_ok());
        let without = usage_error(gated.parse(&argv(&[])).unwrap_err());
        assert_eq!(without.kind, ErrorKind::MissingRequired);
        // 被 `--key` 覆盖掉的 `--user` 不再算出现过, 于是 `--token` 又变回必填
        let overridden = usage_error(
            gated
                .parse(&argv(&["--user", "u", "--key", "k"]))
                .unwrap_err(),
        );
        assert_eq!(overridden.kind, ErrorKind::MissingRequired);
    }

    #[test]
    fn auto_width_reads_columns() {
        let fixed = Parser::new("demo").with_max_width(44);
        assert_eq!(fixed.width(), 44);
        // 只有开了 auto_width 的参数解析器会读 `COLUMNS`, 所以这里临时改环境变量不影响别的用例
        unsafe { std::env::set_var("COLUMNS", "60") };
        assert_eq!(fixed.width(), 44);
        let auto = Parser::new("demo").with_max_width(44).with_auto_width();
        assert_eq!(auto.width(), 60);
        unsafe { std::env::set_var("COLUMNS", "abc") };
        assert_eq!(auto.width(), 44);
        unsafe { std::env::remove_var("COLUMNS") };
        assert_eq!(auto.width(), 44);
    }

    #[test]
    fn overrides_with_keeps_the_last_one() {
        let mut parser = Parser::new("over").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--a"],
                Argument {
                    default: DefaultValue::Value(Value::Str("A".into())),
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--b"],
                Argument {
                    default: DefaultValue::Value(Value::Str("B".into())),
                    overrides_with: vec!["a".into()],
                    ..Argument::default()
                },
            )
            .unwrap();

        let last_b = parser.parse(&argv(&["--a", "1", "--b", "2"])).unwrap();
        assert_eq!(text(&last_b, "b"), "2");
        assert_eq!(text(&last_b, "a"), "A");

        let last_a = parser.parse(&argv(&["--b", "2", "--a", "1"])).unwrap();
        assert_eq!(text(&last_a, "a"), "1");
        assert_eq!(text(&last_a, "b"), "B");
    }

    #[test]
    fn from_file_prefix_expands_lines() {
        let dir = std::env::temp_dir().join("argparser-include");
        std::fs::create_dir_all(&dir).unwrap();
        let inner = dir.join("inner.txt");
        std::fs::write(&inner, "--output\ninner.txt\nb.txt\n").unwrap();
        let outer = dir.join("outer.txt");
        std::fs::write(&outer, format!("@{}\n", inner.display())).unwrap();

        let mut parser = Parser::new("files")
            .with_color(ColorChoice::Never)
            .with_from_file_prefix_chars("@");
        parser
            .add_argument(&["-o", "--output"], Argument::default())
            .unwrap();
        parser
            .add_argument(
                &["files"],
                Argument {
                    nargs: Nargs::ZeroOrMore,
                    ..Argument::default()
                },
            )
            .unwrap();

        let plain = parser.parse(&argv(&["a.txt"])).unwrap();
        assert_eq!(text(&plain, "files"), "a.txt");

        let expanded = parser.parse(&[format!("@{}", outer.display())]).unwrap();
        assert_eq!(text(&expanded, "output"), "inner.txt");
        assert_eq!(text(&expanded, "files"), "b.txt");

        let missing = usage_error(
            parser
                .parse(&[format!("@{}", dir.join("nope.txt").display())])
                .unwrap_err(),
        );
        assert!(
            missing.message.contains("unable to open file"),
            "{}",
            missing.message
        );
        assert_eq!(missing.kind, ErrorKind::Other);
        std::fs::remove_dir_all(&dir).ok();
    }

    #[test]
    fn parse_known_collects_extras_in_command_line_order() {
        let mut parser = Parser::new("known").with_color(ColorChoice::Never);
        parser
            .add_argument(&["first"], Argument::default())
            .unwrap();
        let (namespace, extras) = parser.parse_known(&argv(&["a", "b", "--nope"])).unwrap();
        assert_eq!(text(&namespace, "first"), "a");
        assert_eq!(extras, ["b", "--nope"]);

        let (_, extras) = parser.parse_known(&argv(&["--nope", "a", "b"])).unwrap();
        assert_eq!(extras, ["--nope", "b"]);

        let (_, extras) = parser.parse_known(&argv(&["-xval", "a"])).unwrap();
        assert_eq!(extras, ["-xval"]);

        let error = usage_error(parser.parse(&argv(&["a", "--nope"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::UnknownArgument);
        assert!(
            error
                .to_string()
                .contains("unexpected argument '--nope' found"),
            "{error}"
        );
    }

    #[test]
    fn suppress_and_argument_default_control_the_namespace() {
        let mut parser = Parser::new("suppress").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--gone"],
                Argument {
                    default: DefaultValue::Suppress,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser
            .add_argument(
                &["--kept"],
                Argument {
                    default: DefaultValue::Value(Value::Null),
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = parser.parse(&argv(&[])).unwrap();
        assert!(!namespace.contains("gone"));
        assert!(namespace.contains("kept"));

        let mut fallback = Parser::new("fallback")
            .with_argument_default(DefaultValue::Suppress)
            .with_color(ColorChoice::Never);
        fallback
            .add_argument(&["--x"], Argument::default())
            .unwrap();
        let namespace = fallback.parse(&argv(&[])).unwrap();
        assert!(!namespace.contains("x"));
    }

    #[test]
    fn value_parser_ranges_and_inferred_parsers() {
        let mut parser = Parser::new("types").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["--count"],
                Argument {
                    value_parser: Some(parse_int(Some(1), Some(10))),
                    ..Argument::default()
                },
            )
            .unwrap();
        assert_eq!(
            text(&parser.parse(&argv(&["--count", "5"])).unwrap(), "count"),
            "5"
        );

        let error = usage_error(parser.parse(&argv(&["--count", "11"])).unwrap_err());
        assert_eq!(error.kind, ErrorKind::InvalidValue);
        assert!(error.to_string().contains("11 is not in 1..=10"), "{error}");

        let mut inferred = Parser::new("inferred").with_color(ColorChoice::Never);
        inferred
            .add_argument(
                &["--port"],
                Argument {
                    default: DefaultValue::Value(Value::Int(8080)),
                    ..Argument::default()
                },
            )
            .unwrap();
        let namespace = inferred.parse(&argv(&["--port", "9000"])).unwrap();
        assert_eq!(namespace.get("port"), Some(&Value::Int(9000)));

        let error = usage_error(inferred.parse(&argv(&["--port", "abc"])).unwrap_err());
        assert!(
            error.to_string().contains("invalid digit found in string"),
            "{error}"
        );
    }

    #[test]
    fn definition_errors_are_rejected_at_registration() {
        let mut parser = Parser::new("dup").with_color(ColorChoice::Never);
        parser.add_argument(&["--a"], Argument::default()).unwrap();
        assert!(parser.add_argument(&["--a"], Argument::default()).is_err());

        let mut dests = Parser::new("dests").with_color(ColorChoice::Never);
        dests.add_argument(&["--a"], Argument::default()).unwrap();
        let other = Argument {
            dest: "a".to_string(),
            ..Argument::default()
        };
        assert!(dests.add_argument(&["--b"], other).is_err());

        let mut bad_name = Parser::new("bad").with_color(ColorChoice::Never);
        assert!(bad_name.add_argument(&["-1"], Argument::default()).is_err());

        let mut remainder = Parser::new("rem").with_color(ColorChoice::Never);
        remainder
            .add_argument(
                &["rest"],
                Argument {
                    nargs: Nargs::Remainder,
                    ..Argument::default()
                },
            )
            .unwrap();
        assert!(
            remainder
                .add_argument(&["tail"], Argument::default())
                .is_err()
        );

        let mut consts = Parser::new("const").with_color(ColorChoice::Never);
        assert!(
            consts
                .add_argument(
                    &["--m"],
                    Argument {
                        action: Action::StoreConst,
                        ..Argument::default()
                    }
                )
                .is_err()
        );
    }

    #[test]
    fn duplicated_dest_across_commands_is_a_definition_error() {
        let mut parser = Parser::new("chain").with_color(ColorChoice::Never);
        parser
            .add_argument(
                &["-v"],
                Argument {
                    action: Action::StoreTrue,
                    global: true,
                    ..Argument::default()
                },
            )
            .unwrap();
        parser.add_subparsers(None, "Commands", false).unwrap();
        {
            let child = parser
                .subparsers_mut()
                .unwrap()
                .add_parser("c", "", &[], &[])
                .unwrap();
            child
                .add_argument(
                    &["-v"],
                    Argument {
                        action: Action::StoreTrue,
                        ..Argument::default()
                    },
                )
                .unwrap();
        }
        match parser.parse(&argv(&["c"])) {
            Err(ParseError::Definition { message }) => {
                assert!(message.contains("dest 重复"), "{message}")
            }
            other => panic!("应当是定义期错误, 实际是 {other:?}"),
        }
    }
}
