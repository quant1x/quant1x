// Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
// SPDX-License-Identifier: MIT

//! 参数本体: 取值动作、取值形态、类型转换器与单个参数的定义。
//!
//! 动作名沿用 Python `argparse` 的写法, 语义与 Rust `clap` 的 `ArgAction` 一一对应
//! (`store` ↔ `Set`、`store_true` ↔ `SetTrue`、`count` ↔ `Count`); 字段对照与解析口径见
//! `quant1x/base/argparser/argparser.md`。

use std::fmt;
use std::rc::Rc;

/// 解析结果的单个取值, 对应 Python 侧的任意对象。
#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    /// 空值, 对应 Python 的 `None`。
    Null,
    /// 布尔。
    Bool(bool),
    /// 整数。
    Int(i64),
    /// 浮点。
    Float(f64),
    /// 字符串。
    Str(String),
    /// 列表, 对应 `nargs` 的收集形态。
    List(Vec<Value>),
}

impl Value {
    /// 字符串形态; 不是字符串时为 `None`。
    pub fn as_str(&self) -> Option<&str> {
        match self {
            Self::Str(text) => Some(text),
            _ => None,
        }
    }

    /// 布尔形态; 不是布尔时为 `None`。
    pub fn as_bool(&self) -> Option<bool> {
        match self {
            Self::Bool(value) => Some(*value),
            _ => None,
        }
    }

    /// 整数形态; 不是整数时为 `None`。
    pub fn as_i64(&self) -> Option<i64> {
        match self {
            Self::Int(value) => Some(*value),
            _ => None,
        }
    }

    /// 浮点形态; 不是浮点时为 `None`。
    pub fn as_f64(&self) -> Option<f64> {
        match self {
            Self::Float(value) => Some(*value),
            _ => None,
        }
    }

    /// 列表形态; 不是列表时为 `None`。
    pub fn as_list(&self) -> Option<&[Value]> {
        match self {
            Self::List(items) => Some(items),
            _ => None,
        }
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::Null => f.write_str("None"),
            Self::Bool(true) => f.write_str("true"),
            Self::Bool(false) => f.write_str("false"),
            Self::Int(value) => write!(f, "{value}"),
            Self::Float(value) => f.write_str(&float_text(*value)),
            Self::Str(text) => f.write_str(text),
            Self::List(items) => {
                let body = items
                    .iter()
                    .map(Value::to_string)
                    .collect::<Vec<_>>()
                    .join(", ");
                f.write_str(&body)
            }
        }
    }
}

/// 把浮点写成与 Python `str(float)` 同形的文本: 整数形态补 `.0`, 无穷与 `nan` 用小写。
fn float_text(value: f64) -> String {
    if value.is_nan() {
        return "nan".to_string();
    }
    if value.is_infinite() {
        return if value > 0.0 { "inf" } else { "-inf" }.to_string();
    }
    let text = format!("{value}");
    if text.contains('.') {
        text
    } else {
        format!("{text}.0")
    }
}

/// 解析结果的属性式容器, 对应 `argparse.Namespace`; 保持首次写入的顺序。
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Namespace {
    values: Vec<(String, Value)>,
}

impl Namespace {
    /// 空命名空间。
    pub fn new() -> Self {
        Self::default()
    }

    /// 按名字读取; 未写入时为 `None`。
    pub fn get(&self, key: &str) -> Option<&Value> {
        self.values
            .iter()
            .find(|(name, _)| name == key)
            .map(|(_, value)| value)
    }

    /// 该名字是否已写入。
    pub fn contains(&self, key: &str) -> bool {
        self.get(key).is_some()
    }

    /// 抹掉一个已写入的取值; 没写过时什么也不做。
    pub fn remove(&mut self, key: &str) {
        self.values.retain(|(name, _)| name != key);
    }

    /// 写入一个取值; 同名覆盖且位置不变。
    pub fn insert(&mut self, key: impl Into<String>, value: Value) {
        let key = key.into();
        if let Some((_, slot)) = self.values.iter_mut().find(|(name, _)| *name == key) {
            *slot = value;
            return;
        }
        self.values.push((key, value));
    }

    /// 按写入顺序遍历 `(名字, 取值)`。
    pub fn iter(&self) -> impl Iterator<Item = (&str, &Value)> {
        self.values
            .iter()
            .map(|(name, value)| (name.as_str(), value))
    }

    /// 已写入的条目数。
    pub fn len(&self) -> usize {
        self.values.len()
    }

    /// 是否一条也没写入。
    pub fn is_empty(&self) -> bool {
        self.values.is_empty()
    }

    /// 浅拷贝成 `(名字, 取值)` 列表, 对应 Python 的 `as_dict()` 的遍历形态。
    pub fn as_pairs(&self) -> Vec<(String, Value)> {
        self.values.clone()
    }
}

/// 参数出现时怎么取值、怎么落进命名空间。
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Action {
    /// 取一个 (或一组) 取值写进命名空间。
    Store,
    /// 不取值, 出现即写 `true`。
    StoreTrue,
    /// 不取值, 出现即写 `false`。
    StoreFalse,
    /// 不取值, 出现即写 `const`。
    StoreConst,
    /// 每次出现追加一项。
    Append,
    /// 每次出现追加 `const`。
    AppendConst,
    /// 每次出现把取值并入同一个列表。
    Extend,
    /// 每出现一次计数加一。
    Count,
    /// 请求帮助。
    Help,
    /// 请求版本。
    Version,
}

impl Action {
    /// 该动作是否从命令行吃掉取值; `store_true` / `count` 这类开关为假。
    pub fn takes_values(self) -> bool {
        matches!(self, Self::Store | Self::Append | Self::Extend)
    }
}

/// 取值形态, 对应 Python 的 `nargs`。
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Nargs {
    /// 一个取值 (Python 的 `nargs=None`)。
    One,
    /// 恰好 N 个。
    Count(u32),
    /// 零个或一个 (Python 的 `?`)。
    Optional,
    /// 任意多个 (Python 的 `*`)。
    ZeroOrMore,
    /// 一个及以上 (Python 的 `+`)。
    OneOrMore,
    /// 余下全部 (Python 的 `REMAINDER`)。
    Remainder,
}

impl Nargs {
    /// 最少吃几个取值。
    pub fn min_count(self) -> u32 {
        match self {
            Self::Optional | Self::ZeroOrMore | Self::Remainder => 0,
            Self::OneOrMore | Self::One => 1,
            Self::Count(count) => count,
        }
    }

    /// 最多吃几个取值, `None` 表示无上限。
    pub fn max_count(self) -> Option<u32> {
        match self {
            Self::ZeroOrMore | Self::OneOrMore | Self::Remainder => None,
            Self::One | Self::Optional => Some(1),
            Self::Count(count) => Some(count),
        }
    }

    /// 该形态落值是否恒为列表。
    pub fn collects(self) -> bool {
        matches!(
            self,
            Self::Count(_) | Self::ZeroOrMore | Self::OneOrMore | Self::Remainder
        )
    }
}

/// 可选值集合: 列表形态只给值, 字典形态附带说明。
#[derive(Debug, Clone, PartialEq)]
pub enum Choices {
    /// 只有取值集合。
    Values(Vec<Value>),
    /// 取值与说明配对。
    Described(Vec<(Value, String)>),
}

/// 参数的缺省值来源: 区分"没给默认值"与"默认值就是不写进命名空间"。
#[derive(Debug, Clone, Default, PartialEq)]
pub enum DefaultValue {
    /// 没给默认值, 按动作的隐式缺省落值。
    #[default]
    Missing,
    /// 不写进命名空间。
    Suppress,
    /// 显式默认值。
    Value(Value),
}

/// 取值转换函数: 非法输入返回 `Err(原因)`。
type ConvertFn = dyn Fn(&str) -> Result<Value, String>;

/// 取值的类型转换器: 把命令行上的字符串翻成目标值, 非法取值返回 `Err(原因)`。
///
/// `type_name` 只用于文案 (帮助里的取值说明), 不参与转换。
#[derive(Clone)]
pub struct ValueParser {
    func: Rc<ConvertFn>,
    type_name: String,
}

impl ValueParser {
    /// 把任意 `&str -> Result<Value, String>` 的可调用包成转换器。
    pub fn of<F>(func: F, type_name: impl Into<String>) -> Self
    where
        F: Fn(&str) -> Result<Value, String> + 'static,
    {
        Self {
            func: Rc::new(func),
            type_name: type_name.into(),
        }
    }

    /// 转换器在文案里的类型名。
    pub fn type_name(&self) -> &str {
        &self.type_name
    }

    /// 转换一个取值; 非法输入返回 `Err(原因)`。
    pub fn parse(&self, text: &str) -> Result<Value, String> {
        (self.func)(text)
    }
}

impl fmt::Debug for ValueParser {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("ValueParser")
            .field("type_name", &self.type_name)
            .finish()
    }
}

/// 把上下界写成 clap 风格的开区间记法 (`1..=10` / `..=10` / `1..`)。
fn range_text(minimum: Option<String>, maximum: Option<String>) -> String {
    match (minimum, maximum) {
        (None, None) => String::new(),
        (None, Some(max)) => format!("..={max}"),
        (Some(min), None) => format!("{min}.."),
        (Some(min), Some(max)) => format!("{min}..={max}"),
    }
}

/// 越界即返回 `Err(原因)`, 文案与 clap 的 ranged parser 同形。
fn check_range_int(value: i64, minimum: Option<i64>, maximum: Option<i64>) -> Result<(), String> {
    if minimum.is_some_and(|min| value < min) || maximum.is_some_and(|max| value > max) {
        let text = range_text(
            minimum.map(|item| item.to_string()),
            maximum.map(|item| item.to_string()),
        );
        return Err(format!("{value} is not in {text}"));
    }
    Ok(())
}

/// 浮点版的越界判定; 文本里的界与取值都按 [`float_text`] 书写。
fn check_range_float(value: f64, minimum: Option<f64>, maximum: Option<f64>) -> Result<(), String> {
    if minimum.is_some_and(|min| value < min) || maximum.is_some_and(|max| value > max) {
        let text = range_text(minimum.map(float_text), maximum.map(float_text));
        return Err(format!("{} is not in {text}", float_text(value)));
    }
    Ok(())
}

/// 十进制整数转换器, 可带闭区间上下界。
pub fn parse_int(minimum: Option<i64>, maximum: Option<i64>) -> ValueParser {
    ValueParser::of(
        move |text| {
            let value: i64 = text
                .parse()
                .map_err(|_| "invalid digit found in string".to_string())?;
            check_range_int(value, minimum, maximum)?;
            Ok(Value::Int(value))
        },
        "INTEGER",
    )
}

/// 浮点转换器, 可带闭区间上下界。
pub fn parse_float(minimum: Option<f64>, maximum: Option<f64>) -> ValueParser {
    ValueParser::of(
        move |text| {
            let value: f64 = text
                .parse()
                .map_err(|_| "invalid float literal".to_string())?;
            check_range_float(value, minimum, maximum)?;
            Ok(Value::Float(value))
        },
        "FLOAT",
    )
}

/// 布尔转换器: `true/false`、`yes/no`、`on/off`、`1/0`, 大小写不敏感。
pub fn parse_bool() -> ValueParser {
    ValueParser::of(
        |text| match text.trim().to_lowercase().as_str() {
            "true" | "yes" | "on" | "1" => Ok(Value::Bool(true)),
            "false" | "no" | "off" | "0" => Ok(Value::Bool(false)),
            _ => Err("provided string was not one of true/false/yes/no/on/off/1/0".to_string()),
        },
        "BOOL",
    )
}

/// 单个命令行参数的定义, 由 `Parser::add_argument` 构造并登记。
///
/// 位置参数没有 `names`; 选项至少有一个以 `-` 开头的名字。
#[derive(Debug, Clone)]
pub struct Argument {
    /// 选项名; 位置参数为空。
    pub names: Vec<String>,
    /// 落进命名空间的名字。
    pub dest: String,
    /// 取值动作。
    pub action: Action,
    /// 取值形态。
    pub nargs: Nargs,
    /// 取值转换器; `None` 表示原样取字符串。
    pub value_parser: Option<ValueParser>,
    /// 可选值集合。
    pub choices: Option<Choices>,
    /// 缺省值来源。
    pub default: DefaultValue,
    /// 开关 / 常量动作落的值。
    pub const_value: Option<Value>,
    /// 是否必填 (只对选项生效)。
    pub required: bool,
    /// 帮助里的取值占位名。
    pub metavar: Option<String>,
    /// 短帮助正文。
    pub help: Option<String>,
    /// 长帮助正文。
    pub long_help: Option<String>,
    /// 兜底的环境变量名。
    pub env: Option<String>,
    /// 不出现在帮助里的别名。
    pub alias: Vec<String>,
    /// 出现在帮助里的别名。
    pub visible_alias: Vec<String>,
    /// 与之互斥的参数 (名字或 `dest`)。
    pub conflicts_with: Vec<String>,
    /// 依赖的参数 (名字或 `dest`)。
    pub requires: Vec<String>,
    /// 与之互相覆盖的参数 (名字或 `dest`); 一对里命令行上后出现的留下。
    pub overrides_with: Vec<String>,
    /// 列出的参数里只要有一个出现过, 本参数就不算必填。
    pub required_unless_present: Vec<String>,
    /// 一条 token 内的取值分隔符。
    pub value_delimiter: Option<String>,
    /// 是否从帮助里隐藏。
    pub hide: bool,
    /// 是否隐藏默认值标注。
    pub hide_default: bool,
    /// 是否允许取值以 `-` 开头。
    pub allow_hyphen_values: bool,
    /// 是否下沉到子命令。
    pub global: bool,
    /// 所属帮助分组。
    pub group: Option<String>,
}

impl Default for Argument {
    fn default() -> Self {
        Self {
            names: Vec::new(),
            dest: String::new(),
            action: Action::Store,
            nargs: Nargs::One,
            value_parser: None,
            choices: None,
            default: DefaultValue::Missing,
            const_value: None,
            required: false,
            metavar: None,
            help: None,
            long_help: None,
            env: None,
            alias: Vec::new(),
            visible_alias: Vec::new(),
            conflicts_with: Vec::new(),
            requires: Vec::new(),
            overrides_with: Vec::new(),
            required_unless_present: Vec::new(),
            value_delimiter: None,
            hide: false,
            hide_default: false,
            allow_hyphen_values: false,
            global: false,
            group: None,
        }
    }
}

impl Argument {
    /// 位置参数: 没有选项名, 按命令行顺序吃取值。
    pub fn is_positional(&self) -> bool {
        self.names.is_empty()
    }

    /// 是否吃取值; `store_true` / `count` 这类开关为假。
    pub fn takes_values(&self) -> bool {
        self.action.takes_values()
    }

    /// 取值是否恒以列表形态落进命名空间。
    ///
    /// `nargs` 为整数、`*` / `+` / `REMAINDER`, 或配了 `value_delimiter` 时成立;
    /// `nargs` 缺省与 `?` 是单值。
    pub fn collects(&self) -> bool {
        self.value_delimiter.is_some() || self.nargs.collects()
    }

    /// 最少吃几个取值。
    pub fn min_count(&self) -> u32 {
        self.nargs.min_count()
    }

    /// 最多吃几个取值, `None` 表示无上限。
    pub fn max_count(&self) -> Option<u32> {
        self.nargs.max_count()
    }

    /// 取值的显示名: `metavar` 优先, 其次大写化的 `dest`。
    pub fn value_name(&self) -> String {
        match &self.metavar {
            Some(name) => name.clone(),
            None => self.dest.to_uppercase(),
        }
    }

    /// 取值占位, 如 `<FILE>`。
    pub fn value_spec(&self) -> String {
        format!("<{}>", self.value_name())
    }

    /// 指代该参数时优先用长选项; 只有短选项时退回它。
    pub fn primary_name(&self) -> String {
        if let Some(name) = self.names.iter().find(|name| name.starts_with("--")) {
            return name.clone();
        }
        match self.names.first() {
            Some(name) => name.clone(),
            None => self.dest.clone(),
        }
    }

    /// 错误文案里的指代, 如 `--output <FILE>` / `<FILE>` / `-v`。
    pub fn error_name(&self) -> String {
        if self.is_positional() {
            return self.value_spec();
        }
        let name = self.primary_name();
        if self.takes_values() {
            format!("{name} {}", self.value_spec())
        } else {
            name
        }
    }

    /// 帮助左列: `-o, --output <FILE>`; 位置参数按是否必填写成 `<FILE>` / `[FILE]...`。
    pub fn help_names(&self) -> String {
        if self.is_positional() {
            return self.usage_spec();
        }
        let mut names = self.names.clone();
        names.extend(self.visible_alias.iter().cloned());
        let mut head = names.join(", ");
        if self.action == Action::Count {
            head.push_str("...");
        }
        if self.takes_values() {
            format!("{head} {}", self.value_spec())
        } else {
            head
        }
    }

    /// usage 行里的位置参数占位: `<FILE>` / `[FILE]` / `<FILE>...`。
    pub fn usage_spec(&self) -> String {
        let name = self.value_name();
        match self.nargs {
            Nargs::Optional => format!("[{name}]"),
            Nargs::ZeroOrMore | Nargs::Remainder => format!("[{name}]..."),
            Nargs::OneOrMore | Nargs::Count(_) => format!("<{name}>..."),
            Nargs::One => format!("<{name}>"),
        }
    }

    /// 可选值集合; 字典形态取其键。
    pub fn choice_values(&self) -> Option<Vec<Value>> {
        self.choices.as_ref().map(|choices| match choices {
            Choices::Values(values) => values.clone(),
            Choices::Described(items) => items.iter().map(|(value, _)| value.clone()).collect(),
        })
    }

    /// 字典形态可选值的说明文字。
    pub fn choice_help(&self, value: &Value) -> Option<&str> {
        match self.choices.as_ref()? {
            Choices::Values(_) => None,
            Choices::Described(items) => items
                .iter()
                .find(|(item, _)| item == value)
                .map(|(_, note)| note.as_str()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn positional(dest: &str, nargs: Nargs) -> Argument {
        Argument {
            dest: dest.to_string(),
            nargs,
            ..Argument::default()
        }
    }

    #[test]
    fn action_takes_values_only_for_store_like() {
        for action in [Action::Store, Action::Append, Action::Extend] {
            assert!(action.takes_values(), "{action:?}");
        }
        for action in [
            Action::StoreTrue,
            Action::StoreFalse,
            Action::StoreConst,
            Action::AppendConst,
            Action::Count,
            Action::Help,
            Action::Version,
        ] {
            assert!(!action.takes_values(), "{action:?}");
        }
    }

    #[test]
    fn nargs_shapes_drive_counts_and_collection() {
        assert_eq!(Nargs::One.min_count(), 1);
        assert_eq!(Nargs::One.max_count(), Some(1));
        assert!(!Nargs::One.collects());

        assert_eq!(Nargs::Optional.min_count(), 0);
        assert_eq!(Nargs::Optional.max_count(), Some(1));
        assert!(!Nargs::Optional.collects());

        assert_eq!(Nargs::ZeroOrMore.min_count(), 0);
        assert_eq!(Nargs::ZeroOrMore.max_count(), None);
        assert!(Nargs::ZeroOrMore.collects());

        assert_eq!(Nargs::OneOrMore.min_count(), 1);
        assert_eq!(Nargs::OneOrMore.max_count(), None);
        assert!(Nargs::OneOrMore.collects());

        assert_eq!(Nargs::Remainder.min_count(), 0);
        assert_eq!(Nargs::Remainder.max_count(), None);
        assert!(Nargs::Remainder.collects());

        assert_eq!(Nargs::Count(3).min_count(), 3);
        assert_eq!(Nargs::Count(3).max_count(), Some(3));
        assert!(Nargs::Count(3).collects());
    }

    #[test]
    fn value_delimiter_always_collects() {
        let arg = Argument {
            names: vec!["--tag".to_string()],
            dest: "tag".to_string(),
            value_delimiter: Some(",".to_string()),
            ..Argument::default()
        };
        assert!(arg.collects());
    }

    #[test]
    fn float_text_matches_python_str() {
        assert_eq!(float_text(5.0), "5.0");
        assert_eq!(float_text(1.5), "1.5");
        assert_eq!(float_text(f64::INFINITY), "inf");
        assert_eq!(float_text(f64::NEG_INFINITY), "-inf");
        assert_eq!(float_text(f64::NAN), "nan");
    }

    #[test]
    fn value_display_follows_python_repr_rules() {
        assert_eq!(Value::Null.to_string(), "None");
        assert_eq!(Value::Bool(true).to_string(), "true");
        assert_eq!(Value::Bool(false).to_string(), "false");
        assert_eq!(Value::Int(-3).to_string(), "-3");
        assert_eq!(Value::Float(2.0).to_string(), "2.0");
        assert_eq!(Value::Str("text".to_string()).to_string(), "text");
        let list = Value::List(vec![Value::Int(1), Value::Bool(true)]);
        assert_eq!(list.to_string(), "1, true");
    }

    #[test]
    fn namespace_keeps_first_write_order_and_overwrites_in_place() {
        let mut namespace = Namespace::new();
        assert!(namespace.is_empty());
        namespace.insert("first", Value::Int(1));
        namespace.insert("second", Value::Int(2));
        namespace.insert("first", Value::Int(9));
        assert_eq!(namespace.len(), 2);
        assert_eq!(namespace.get("first"), Some(&Value::Int(9)));
        assert!(namespace.contains("second"));
        assert!(!namespace.contains("third"));
        let names: Vec<&str> = namespace.iter().map(|(name, _)| name).collect();
        assert_eq!(names, ["first", "second"]);
        assert_eq!(namespace.as_pairs().len(), 2);
    }

    #[test]
    fn parse_int_rejects_bad_digits_and_out_of_range() {
        let parser = parse_int(Some(1), Some(10));
        assert_eq!(parser.parse("5").unwrap(), Value::Int(5));
        assert_eq!(parser.parse("11").unwrap_err(), "11 is not in 1..=10");
        assert_eq!(parser.parse("0").unwrap_err(), "0 is not in 1..=10");
        assert_eq!(
            parser.parse("abc").unwrap_err(),
            "invalid digit found in string"
        );
        let open = parse_int(None, Some(10));
        assert_eq!(open.parse("11").unwrap_err(), "11 is not in ..=10");
        let unbounded = parse_int(None, None);
        assert_eq!(unbounded.parse("-7").unwrap(), Value::Int(-7));
    }

    #[test]
    fn parse_float_rejects_bad_literals_and_out_of_range() {
        let parser = parse_float(Some(0.0), Some(1.0));
        assert_eq!(parser.parse("0.5").unwrap(), Value::Float(0.5));
        assert_eq!(parser.parse("1.5").unwrap_err(), "1.5 is not in 0.0..=1.0");
        assert_eq!(parser.parse("abc").unwrap_err(), "invalid float literal");
    }

    #[test]
    fn parse_bool_is_case_insensitive() {
        let parser = parse_bool();
        assert_eq!(parser.parse("YES").unwrap(), Value::Bool(true));
        assert_eq!(parser.parse(" off ").unwrap(), Value::Bool(false));
        assert_eq!(parser.parse("1").unwrap(), Value::Bool(true));
        assert_eq!(
            parser.parse("maybe").unwrap_err(),
            "provided string was not one of true/false/yes/no/on/off/1/0"
        );
    }

    #[test]
    fn value_parser_keeps_given_type_name() {
        let parser = ValueParser::of(|text: &str| Ok(Value::Str(text.to_uppercase())), "UPPER");
        assert_eq!(parser.type_name(), "UPPER");
        assert_eq!(parser.parse("abc").unwrap(), Value::Str("ABC".to_string()));
        assert_eq!(
            format!("{parser:?}"),
            "ValueParser { type_name: \"UPPER\" }"
        );
    }

    #[test]
    fn argument_names_prefer_long_option() {
        let arg = Argument {
            names: vec!["-o".to_string(), "--output".to_string()],
            dest: "output".to_string(),
            metavar: Some("FILE".to_string()),
            ..Argument::default()
        };
        assert!(arg.takes_values());
        assert_eq!(arg.primary_name(), "--output");
        assert_eq!(arg.value_name(), "FILE");
        assert_eq!(arg.error_name(), "--output <FILE>");
        assert_eq!(arg.help_names(), "-o, --output <FILE>");

        let short_only = Argument {
            names: vec!["-v".to_string()],
            dest: "verbose".to_string(),
            action: Action::StoreTrue,
            nargs: Nargs::Count(0),
            ..Argument::default()
        };
        assert_eq!(short_only.primary_name(), "-v");
        assert_eq!(short_only.error_name(), "-v");
        assert_eq!(short_only.help_names(), "-v");
    }

    #[test]
    fn count_action_marks_help_head_with_ellipsis() {
        let arg = Argument {
            names: vec!["-v".to_string()],
            dest: "verbose".to_string(),
            action: Action::Count,
            nargs: Nargs::Count(0),
            ..Argument::default()
        };
        assert_eq!(arg.help_names(), "-v...");
    }

    #[test]
    fn positional_usage_spec_covers_every_nargs() {
        assert_eq!(positional("file", Nargs::One).usage_spec(), "<FILE>");
        assert_eq!(positional("file", Nargs::Optional).usage_spec(), "[FILE]");
        assert_eq!(
            positional("file", Nargs::ZeroOrMore).usage_spec(),
            "[FILE]..."
        );
        assert_eq!(
            positional("file", Nargs::OneOrMore).usage_spec(),
            "<FILE>..."
        );
        assert_eq!(
            positional("file", Nargs::Remainder).usage_spec(),
            "[FILE]..."
        );
        assert_eq!(
            positional("file", Nargs::Count(2)).usage_spec(),
            "<FILE>..."
        );
    }

    #[test]
    fn positional_help_names_follow_usage_spec() {
        let arg = positional("files", Nargs::ZeroOrMore);
        assert!(arg.is_positional());
        assert_eq!(arg.help_names(), "[FILES]...");
        assert_eq!(arg.error_name(), "<FILES>");
    }

    #[test]
    fn visible_alias_shows_in_help_but_alias_does_not() {
        let arg = Argument {
            names: vec!["--build".to_string()],
            dest: "build".to_string(),
            alias: vec!["-b".to_string()],
            visible_alias: vec!["-B".to_string()],
            action: Action::StoreTrue,
            nargs: Nargs::Count(0),
            ..Argument::default()
        };
        assert_eq!(arg.help_names(), "--build, -B");
    }

    #[test]
    fn choices_expose_values_and_notes() {
        let described = Argument {
            dest: "level".to_string(),
            choices: Some(Choices::Described(vec![
                (Value::Str("debug".to_string()), "逐条打印".to_string()),
                (Value::Str("info".to_string()), "常规".to_string()),
            ])),
            ..Argument::default()
        };
        let values = described.choice_values().unwrap();
        assert_eq!(
            values,
            vec![
                Value::Str("debug".to_string()),
                Value::Str("info".to_string())
            ]
        );
        assert_eq!(
            described.choice_help(&Value::Str("debug".to_string())),
            Some("逐条打印")
        );
        assert_eq!(described.choice_help(&Value::Str("warn".to_string())), None);

        let plain = Argument {
            dest: "level".to_string(),
            choices: Some(Choices::Values(vec![Value::Str("a".to_string())])),
            ..Argument::default()
        };
        assert_eq!(plain.choice_help(&Value::Str("a".to_string())), None);
        assert_eq!(Argument::default().choice_values(), None);
    }

    #[test]
    fn default_value_distinguishes_missing_from_suppress() {
        assert_eq!(DefaultValue::default(), DefaultValue::Missing);
        assert_ne!(DefaultValue::Missing, DefaultValue::Suppress);
        assert_eq!(
            DefaultValue::Value(Value::Int(1)),
            DefaultValue::Value(Value::Int(1))
        );
    }
}
