# dotenv：`.env` 与系统环境变量的唯一读取口

`env(key, default)` 与 `read_dotenv(key)` 是取环境变量的唯一入口：先看两份 `.env` 的合并结果，再退系统
环境变量，最后用调用方给的默认值。只读，不写 `os.environ`。

本模块只管 `.env` 与系统环境变量。**进程运行环境**（入口应用定位、项目布局推演、当前用户）属于另一类
概念，不在本模块 —— “运行环境”是进程上下文，与“环境变量”是两回事。

## 1. 优先级

1. 项目根的 `.env`；
2. 用户主目录的 `~/.env`；
3. 系统环境变量；
4. 调用方给的默认值。

两份 `.env` 都读、**逐键合并**：同名键取项目根那份，只有主目录有的键就用主目录的值；不存在"先找到哪份
就停"。项目根按 `filesystem.find_project_root` 的标记（`pyproject.toml` → `LICENSE` → `.git`）向上定位，
起点依次是工作目录、入口脚本所在目录、本模块所在目录。

## 2. 解析规则

- 只认 `KEY=VALUE`；空行与 `#` 开头的行跳过；
- 容忍 BOM（按 `utf-8-sig` 读）与 `export ` 前缀；
- 值成对引号时剥掉引号并 trim；未加引号时截到空白后的 `#` 注释处；
- 键名不做变量展开。

## 3. 变量展开

值的展开交给 `expandvars.expand`：`$NAME` / `${NAME}` 换成对应取值，先查合并表（另一份 `.env` 的键同样
可用），再查系统环境变量，两处都没有就原样保留；引用成环时整段原样返回；`$$` 是字面 `$`。键名只认
`[A-Za-z_][A-Za-z0-9_]*`，不支持 `${NAME:-默认值}` 这类修饰写法。

## 4. 热加载

合并结果在进程内缓存，由 `hotreload.Reloader` 维护：每次取值只比对两份 `.env` 的存在性与 `st_mtime_ns`，
任一份动过才重新读盘，运行期新建的 `.env` 也能被发现。缓存是模块级状态，测试里用
`dotenv._loader.reset()` 作废。

## 5. 依赖与测试

- 出向：标准库 + 同目录 `expandvars` / `hotreload`；项目根定位复用 `filesystem`。
- 入向：暂无调用方（`filesystem.homedir()` 仍直读系统环境变量，不经过本模块）。
- 用例：`python -m quant1x.base.dotenv_test`、`python -m quant1x.base.expandvars_test`、
  `python -m quant1x.base.hotreload_test`。
