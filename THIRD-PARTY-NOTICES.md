# 第三方组件声明（Third-Party Notices）

本文件列出来自第三方的组件及其许可证，按各许可证要求随本项目归档声明。
panshi 本体版权与许可见根 `LICENSE`（Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>，MIT）。panshi 本体以 MIT 许可授权：在软件副本或实质部分中须保留版权声明与许可声明，完整条款见仓库根 `LICENSE`。
许可证全文归档位于 `third_party/licenses/` 目录（按语言拆分：`third_party/licenses/python/`、`third_party/licenses/rust/`）。

## 文档结构与维护边界

本文件为「人工合规正文 + 机器生成区段」的混合文档：**带围栏注释（`<!--…_START-->` ~ `<!--…_END-->`）的区段由 `panshi.tool.license.audit` 按锁文件 / 工具链元数据自动全量重建**；围栏之外的内容一律人工维护。改文前先分清两类，避免人工手写被工具重跑覆盖。

### 机器生成区段（勿手抄 / 勿手改，重跑即全量重建）

每次运行对应子命令时，下列区段的数据行全部按本次扫描重建（组件行随依赖增删自动进出，版本 / 许可证 / dev 注 / purl / sha256 均为机器确定，以工具输出为准）；**同一组件库存在多个版本时只列最高版本**，被淘汰版本的行随之消失：

| 围栏区段 | 内容 | 刷新命令（仓库根执行） |
| --- | --- | --- |
| `<!--PYTHON_COMPONENTS_START-->` ~ `_END` | Python 组件表 | `labs-license-audit python` |
| `<!--CPP_COMPONENTS_START-->` ~ `_END` | C++（Conan）组件表 | `labs-license-audit cpp` |
| `<!--RUST_DIRECT_START-->` ~ `_END` | Rust 直接依赖表 | `labs-license-audit rust` |
| `<!--GO_COMPONENTS_START-->` ~ `_END` | Go 组件表 | `labs-license-audit go` |
| `<!--JAVA_COMPONENTS_START-->` ~ `_END` | Java 组件表 | `labs-license-audit java` |
| `<!--RUST_CLOSURE_START-->` ~ `_END` | Rust 完整依赖闭包（按许可证分组，每行含 purl 与 sha256） | `labs-license-audit rust` |

### 人工维护内容（工具一律原样保留）

- **围栏外全部正文**：本头部声明、各语言小节的导语与表外注记、SPDX "OR" 履行原则、归档目录、更新流程与 SBOM 映射小节——工具绝不触碰（不删改格式、不增减信息）；
- **机器区段内的人工程**（刷新时按名从旧文件继承，升级换版本自动延续，不因重跑丢失）：
  - 各组件表格的「版权 / 主页」列：按组件名从旧文件继承；本次新增且旧文件没有的组件该格留空并写入根 `THIRD-PARTY-TODO.md` 待办，**绝不编造**——人工补录后重跑即消除；
  - Rust 闭包分组标题的「履行注文」（如「按 MIT 履行」；含 copyleft 选项时的「选择 X：当前分发方式不触发 copyleft 条件」）：按许可证表达式原文从旧文件继承；新出现的表达式组会进待办提示人工补注文。

### 维护规则

1. 新增 / 升级依赖只更新清单文件（`pyproject.toml` / `conanfile.py` / `Cargo.toml` / `go.mod` / `pom.xml`），不要直接在围栏区段手写组件行；
2. 仓库根执行 `labs-license-audit`（不带语言 = 全部，按 python → c++ → rust → go → java 顺序）刷新全部机器区段与 `third_party/licenses/<lang>/` 归档（每个库只保留最高版本的归档目录，旧版本目录同时清理）；对应语言扫描失败时其区段保持原样不刷新；
3. 机器无法自动确定的事项写入根 `THIRD-PARTY-TODO.md`（无待办自动删除）；**发布前必须清零（退出码 0）**，处理方式见「更新流程」。

## Python（运行时依赖）

<!--PYTHON_COMPONENTS_START-->
| 组件 | 版本 | 许可证 | 版权 / 主页 |
| --- | --- | --- | --- |
| PyYAML | 6.0.4 | MIT | （Copyright (c) 2017-2021 Ingy döt Net；Copyright (c) 2006-2016 Kirill Simonov） · <https://pyyaml.org/wiki/PyYAMLDocumentation> |
| akshare | 1.18.97 | MIT | （Copyright (c) 2019-2026 Albert King） · <https://github.com/akfamily/akshare> |
| apscheduler | 3.11.3 | MIT | （Copyright (c) Alex Grönholm） · <https://apscheduler.readthedocs.io/en/3.x/> |
| caerus | 1.0.1 | MIT | （Copyright (c) 2020 Erdogan Taskesen） · <https://erdogant.github.io/caerus> |
| findpeaks | 2.7.5 | MIT | （Copyright (c) 2020 Erdogan Taskesen） · <https://erdogant.github.io/findpeaks> |
| gitpython | 3.1.62 | BSD-3-Clause | （Copyright (C) 2008, 2009 Michael Trier and contributors） · <https://github.com/gitpython-developers/GitPython> |
| icecream | 2.1.11 | MIT | （Copyright 2018 Ansgar Grunseid） · <https://github.com/gruns/icecream> |
| inflection | 0.5.1 | MIT | （Copyright (C) 2012-2020 Janne Vanhala） · <https://github.com/jpvanhal/inflection> |
| joblib | 1.5.3 | BSD-3-Clause | （Copyright (c) 2008-2021, The joblib developers） · <https://joblib.readthedocs.io> |
| loguru | 0.8.0 | MIT License | （Copyright (c) 2017） · <https://github.com/Delgan/loguru> |
| matplotlib | 3.10.9 | Python Software Foundation License | （Copyright (c) 1997, 2009, American Mathematical Society (http://www.ams.org)；Copyright (C) 1994, 1995, Basil K. Malyshev. All Rights Reserved） · <https://matplotlib.org> |
| mlxtend | 0.25.0 | BSD 3-Clause | （Copyright (c) 2014-2026, Sebastian Raschka. All rights reserved） · <https://github.com/rasbt/mlxtend> |
| mootdx | 0.11.7 | MIT | （Copyright (c) 2017, mootdx） · <https://github.com/mootdx/mootdx> |
| numba | 0.62.1 | BSD | （Copyright (c) 2012, Anaconda, Inc） · <https://numba.pydata.org> |
| numpy | 2.3.5 | BSD License | （Copyright (c) 2005-2025, NumPy Developers；Copyright (c) 2011-2014, The OpenBLAS Project） · <https://numpy.org> |
| pandas | 3.1.1 | BSD-3-Clause | （Copyright (c) 2008-2011, AQR Capital Management, LLC, Lambda Foundry, Inc. and PyData Development Team；Copyright (c) 2011-2026, Open source contributors） · <https://pandas.pydata.org> |
| python-dateutil | 2.9.0.post0 | Dual License | （Copyright 2017- Paul Ganssle；Copyright 2017- dateutil contributors (see AUTHORS file)） · <https://github.com/dateutil/dateutil> |
| python-dotenv | 1.2.4 | BSD-3-Clause | （Copyright (c) 2014, Saurabh Kumar (python-dotenv), 2013, Ted Tieken (django-dotenv-rw), 2013, Jacob Kaplan-Moss (django-dotenv)） · <https://github.com/theskumar/python-dotenv> |
| requests | 2.34.2 | Apache-2.0 | （Copyright 2019 Kenneth Reitz） · <https://github.com/psf/requests> |
| requests-toolbelt | 1.0.0 | Apache 2.0 | （Copyright 2014 Ian Cordasco, Cory Benfield） · <https://github.com/requests/toolbelt> |
| scikit-learn | 1.8.0 | BSD-3-Clause | （Copyright (c) 2007-2024 The scikit-learn developers） · <https://scikit-learn.org> |
| scipy | 1.16.3 | BSD License | （Copyright (c) 2001-2002 Enthought, Inc. 2003, SciPy Developers；Copyright (c) 2011-2014, The OpenBLAS Project） · <https://scipy.org/> |
| tqdm | 4.67.3 | MPL-2.0 AND MIT | （Copyright (c) 2013 noamraph） · <https://tqdm.github.io> |
| xgboost | 3.0.5 | Apache-2.0 | <https://github.com/dmlc/xgboost> |
<!--PYTHON_COMPONENTS_END-->

许可证全文：见 `third_party/licenses/python/<发行名>-<版本>/` 下的归档文件（wheel 未随附许可证文件者已按上游官方 LICENSE 文本人工补录；依赖声明与源码 import 交叉核对由 `panshi.tool.license.audit` 的 `python` 子命令审计核验，见「归档目录」与「更新流程」）。

## C++（Conan 依赖）

<!--CPP_COMPONENTS_START-->
| 组件 | 版本 | 许可证 | 版权 / 主页 |
| --- | --- | --- | --- |
| asio | 1.38.2 | BSL-1.0 | <http://think-async.com/Asio> |
| backward-cpp | 1.6 | MIT | （Copyright 2013 Google Inc. All Rights Reserved） · <https://github.com/bombela/backward-cpp> |
| benchmark | 1.9.5 | Apache-2.0 | <https://github.com/google/benchmark> |
| bshoshany-thread-pool | 5.1.0 | MIT | <https://github.com/bshoshany/thread-pool> |
| capnproto | 1.5.0 | MIT | （Copyright (c) 2013-2017 Sandstorm Development Group, Inc.; Cloudflare, Inc） · <https://capnproto.org> |
| catch2 | 3.16.0 | BSL-1.0 | <https://github.com/catchorg/Catch2> |
| cpp-httplib | 0.65.0 | MIT | <https://github.com/yhirose/cpp-httplib> |
| cpr | 1.14.2 | MIT | （Copyright (c) 2017-2021 Huu Nguyen；Copyright (c) 2022 libcpr and many other contributors） · <https://docs.libcpr.org/> |
| croncpp | 2023.03.30 | MIT | <https://github.com/mariusbancila/croncpp/> |
| csv2 | 0.2 | MIT | <https://github.com/p-ranav/csv2> |
| date | 3.0.5 | MIT | <https://github.com/HowardHinnant/date> |
| fmt | 11.2.0 | MIT | （Copyright (c) 2012 - present, Victor Zverovich and {fmt} contributors） · <https://github.com/fmtlib/fmt> |
| gtest | 1.18.0 | BSD-3-Clause | （Copyright 2008, Google Inc） · <https://github.com/google/googletest> |
| libcurl | 8.21.0 | curl | （Copyright (c) 1996 - 2026, Daniel Stenberg, , and many） · <https://curl.se> |
| libiconv | 1.19 | LGPL-2.1-or-later | （Copyright (C) 2007 Free Software Foundation, Inc.；Copyright (C) 1991, 1999 Free Software Foundation, Inc） · <https://www.gnu.org/software/libiconv/> |
| magic_enum | 0.9.8 | MIT | <https://github.com/Neargye/magic_enum> |
| mimalloc | 3.5.1 | MIT | （Copyright (c) 2018-2025 Microsoft Corporation, Daan Leijen） · <https://github.com/microsoft/mimalloc> |
| minizip | 1.2.12 | Zlib | <https://zlib.net> |
| mio | cci.20230303 | MIT | <https://github.com/mandreyel/mio> |
| nlohmann_json | 3.11.3 | MIT | <https://github.com/nlohmann/json> |
| openssl | 3.6.4 | Apache-2.0 | （Copyright (C) 1989 Free Software Foundation, Inc；Copyright (C) 19yy） · <https://github.com/openssl/openssl> |
| protobuf | 3.21.12 | BSD-3-Clause | （Copyright 2008 Google Inc. All rights reserved；Copyright (c) 2019 Yibo Cai） · <https://github.com/protocolbuffers/protobuf> |
| spdlog | 1.15.3 | MIT | （Copyright (c) 2016 Gabi Melman） · <https://github.com/gabime/spdlog> |
| tsl-robin-map | 1.4.0 | MIT | <https://github.com/Tessil/robin-map> |
| xsimd | 13.2.0 | BSD-3-Clause | <https://github.com/xtensor-stack/xsimd> |
| xtensor | 0.27.1 | BSD-3-Clause | <https://github.com/xtensor-stack/xtensor> |
| xtl | 0.8.0 | BSD-3-Clause | <https://github.com/xtensor-stack/xtl> |
| yaml-cpp | 0.9.0 | MIT | （Copyright (c) 2008-2015 Jesse Beder；Copyright 2008, Google Inc） · <https://github.com/jbeder/yaml-cpp> |
| zlib | 1.3.2 | Zlib | （Copyright (c) 1990-2009 Info-ZIP. All rights reserved） · <https://zlib.net> |
| pfr | 2.2.0 | BSL-1.0 | <https://github.com/boostorg/pfr> |
<!--CPP_COMPONENTS_END-->

本仓 `conanfile.py` 只生成 CMake 工具链、当前未声明第三方依赖，故 C++ 组件表为空；新增依赖后 `labs-license-audit cpp` 自动补齐本表并归档到 `third_party/licenses/cpp/<name>-<version>/`。

## Rust（直接依赖）

<!--RUST_DIRECT_START-->
| 组件 | 版本 | 许可证 |
| --- | --- | --- |
| bytes（dev 依赖，仅测试） | 1.12.1 | MIT |
| chrono | 0.4.45 | MIT OR Apache-2.0 |
| criterion（dev 依赖，仅测试） | 0.5.1 | Apache-2.0 OR MIT |
| cron | 0.12.1 | MIT OR Apache-2.0 |
| csv | 1.4.0 | Unlicense/MIT |
| ctrlc（dev 依赖，仅测试） | 3.5.2 | MIT/Apache-2.0 |
| dirs | 4.0.0 | MIT OR Apache-2.0 |
| encoding | 0.2.33 | MIT |
| encoding_rs | 0.8.35 | (Apache-2.0 OR MIT) AND BSD-3-Clause |
| env_logger | 0.10.2 | MIT OR Apache-2.0 |
| filetime | 0.2.29 | MIT/Apache-2.0 |
| flate2 | 1.1.10 | MIT OR Apache-2.0 |
| hex | 0.4.3 | MIT OR Apache-2.0 |
| httpdate | 1.0.3 | MIT OR Apache-2.0 |
| indicatif | 0.17.11 | MIT |
| libc | 0.2.189 | MIT OR Apache-2.0 |
| log | 0.4.34 | MIT OR Apache-2.0 |
| log4rs | 1.4.0 | MIT OR Apache-2.0 |
| memmap2 | 0.9.11 | MIT OR Apache-2.0 |
| mimalloc | 0.1.52 | MIT |
| mio | 1.2.3 | MIT |
| mockito（dev 依赖，仅测试） | 1.7.2 | MIT |
| once_cell | 1.21.4 | MIT OR Apache-2.0 |
| reqwest | 0.11.27 | MIT OR Apache-2.0 |
| serde | 1.0.229 | MIT OR Apache-2.0 |
| serde_json | 1.0.151 | MIT OR Apache-2.0 |
| serde_repr（dev 依赖，仅测试） | 0.1.21 | MIT OR Apache-2.0 |
| serde_yaml | 0.9.34+deprecated | MIT OR Apache-2.0 |
| signal-hook（dev 依赖，仅测试） | 0.3.18 | Apache-2.0/MIT |
| tempfile | 3.27.0 | MIT OR Apache-2.0 |
| tokio | 1.53.1 | MIT |
| tokio-cron-scheduler（dev 依赖，仅测试） | 0.9.4 | MIT/Apache-2.0 |
| winapi | 0.3.9 | MIT/Apache-2.0 |
| zip | 0.6.6 | MIT |
<!--RUST_DIRECT_END-->

多许可组件按 SPDX "OR" 语义，可在合规前提下选择其一履行：

- 若选项均为宽松许可（MIT / Apache-2.0 / Unlicense / BSD 等），选择非 copyleft 选项履行，如 `Apache-2.0 OR MIT` 按 MIT 履行；
- 若选项含 LGPL / GPL / AGPL 等 copyleft 许可，则选择非 copyleft 选项的前提是不触发该 copyleft 的适用条件（如动态链接、独立进程、不合并源码等），相应履行选择已按组标注，并在 `panshi.tool.license.audit` 中固化判定规则；
- 本文件记录的选择均基于当前分发方式（二进制 / 源码 / 动态链接 / 独立进程）作出，如分发方式改变须复核。

本表组件的多许可选项均含宽松许可，按上述原则选择非 copyleft 选项履行。**完整依赖闭包（含传递依赖）见下文「Rust 完整依赖闭包」（每行含 purl 与 sha256，可对接 SBOM/Dependency-Track），由脚本基于 `Cargo.lock` 机器生成，勿手抄。**

## Go

<!--GO_COMPONENTS_START-->
| 组件 | 版本 | 许可证 | 版权 / 主页 |
| --- | --- | --- | --- |
| github.com/cespare/xxhash/v2 | v2.3.0 | MIT | （Copyright (c) 2016 Caleb Spare） · <https://pkg.go.dev/github.com/cespare/xxhash/v2> |
| github.com/clipperhouse/displaywidth | v0.11.0 | MIT | （Copyright (c) 2025 Matt Sherman） · <https://pkg.go.dev/github.com/clipperhouse/displaywidth> |
| github.com/clipperhouse/uax29/v2 | v2.7.0 | MIT | （Copyright (c) 2020 Matt Sherman） · <https://pkg.go.dev/github.com/clipperhouse/uax29/v2> |
| github.com/fatih/color | v1.19.0 | MIT | （Copyright (c) 2013 Fatih Arslan） · <https://pkg.go.dev/github.com/fatih/color> |
| github.com/fsnotify/fsnotify | v1.10.1 | BSD-3-Clause | （Copyright © 2012 The Go Authors. All rights reserved；Copyright © fsnotify Authors. All rights reserved） · <https://pkg.go.dev/github.com/fsnotify/fsnotify> |
| github.com/gocarina/gocsv | v0.0.0-20261121203658-07c44a65ffb9 | MIT | （Copyright (c) 2014 Jonathan Picques） · <https://pkg.go.dev/github.com/gocarina/gocsv> |
| github.com/goccy/go-json | v0.12.0 | MIT | （Copyright (c) 2020 Masaaki Goshima） · <https://pkg.go.dev/github.com/goccy/go-json> |
| github.com/klauspost/cpuid/v2 | v2.4.2 | MIT | （Copyright (c) 2015 Klaus Post） · <https://pkg.go.dev/github.com/klauspost/cpuid/v2> |
| github.com/lestrrat-go/file-rotatelogs | v2.4.0+incompatible | MIT | （Copyright (c) 2014 lestrrat） · <https://pkg.go.dev/github.com/lestrrat-go/file-rotatelogs> |
| github.com/lestrrat-go/strftime | v1.2.1 | MIT | （Copyright (c) 2016 lestrrat） · <https://pkg.go.dev/github.com/lestrrat-go/strftime> |
| github.com/mattn/go-colorable | v0.1.16 | MIT | （Copyright (c) 2016 Yasuhiro Matsumoto） · <https://pkg.go.dev/github.com/mattn/go-colorable> |
| github.com/mattn/go-isatty | v0.0.24 | MIT | （Copyright (c) Yasuhiro MATSUMOTO） · <https://pkg.go.dev/github.com/mattn/go-isatty> |
| github.com/mattn/go-runewidth | v0.0.31 | MIT | （Copyright (c) 2016 Yasuhiro Matsumoto） · <https://pkg.go.dev/github.com/mattn/go-runewidth> |
| github.com/nxadm/tail | v1.4.11 | MIT | （Copyright (c) 2014 ActiveState） · <https://pkg.go.dev/github.com/nxadm/tail> |
| github.com/olekukonko/cat | v0.0.0-20250911104152-50322a0618f6 | MIT | （Copyright (c) 2025 Oleku Konko） · <https://pkg.go.dev/github.com/olekukonko/cat> |
| github.com/olekukonko/errors | v1.3.0 | MIT | （Copyright (c) 2025 Oleku Konko） · <https://pkg.go.dev/github.com/olekukonko/errors> |
| github.com/olekukonko/ll | v0.1.9 | MIT | （Copyright (c) 2025 Oleku Konko） · <https://pkg.go.dev/github.com/olekukonko/ll> |
| github.com/olekukonko/tablewriter | v1.1.5 | MIT | （Copyright (C) 2014 by Oleku Konko） · <https://pkg.go.dev/github.com/olekukonko/tablewriter> |
| github.com/petermattis/goid | v0.0.0-20261125140513-578663ba5663 | Apache-2.0 | <https://pkg.go.dev/github.com/petermattis/goid> |
| github.com/pkg/errors | v0.9.1 | BSD-2-Clause | （Copyright (c) 2015, Dave Cheney） · <https://pkg.go.dev/github.com/pkg/errors> |
| github.com/stretchr/testify | v1.12.1 | MIT | （Copyright (c) 2012-2020 Mat Ryer, Tyler Bunnell and contributors） · <https://pkg.go.dev/github.com/stretchr/testify> |
| go.uber.org/multierr | v1.11.0 | MIT | （Copyright (c) 2017-2021 Uber Technologies, Inc） · <https://pkg.go.dev/go.uber.org/multierr> |
| go.uber.org/zap | v1.28.0 | MIT | （Copyright (c) 2016-2024 Uber Technologies, Inc） · <https://pkg.go.dev/go.uber.org/zap> |
| go.yaml.in/yaml/v3 | v3.0.5 | Apache-2.0, MIT | （Copyright (c) 2006-2010 Kirill Simonov；Copyright (c) 2006-2011 Kirill Simonov） · <https://pkg.go.dev/go.yaml.in/yaml/v3> |
| golang.org/x/sys | v0.50.0 | BSD-3-Clause | （Copyright 2009 The Go Authors） · <https://pkg.go.dev/golang.org/x/sys> |
| golang.org/x/text | v0.44.0 | BSD-3-Clause | （Copyright 2009 The Go Authors） · <https://pkg.go.dev/golang.org/x/text> |
| google.golang.org/protobuf | v1.36.12 | BSD-3-Clause | （Copyright (c) 2018 The Go Authors. All rights reserved） · <https://pkg.go.dev/google.golang.org/protobuf> |
| gopkg.in/tomb.v1 | v1.0.0-20141024135613-dd632973f1e7 | BSD-3-Clause | （Copyright (c) 2010-2011 - Gustavo Niemeyer） · <https://pkg.go.dev/gopkg.in/tomb.v1> |
| gopkg.in/yaml.v3 | v3.0.1 | Apache-2.0, MIT | （Copyright (c) 2006-2010 Kirill Simonov；Copyright (c) 2006-2011 Kirill Simonov） · <https://pkg.go.dev/gopkg.in/yaml.v3> |
<!--GO_COMPONENTS_END-->

本仓 `go.mod` 当前未声明第三方模块，Go 组件表为空；新增依赖后 `labs-license-audit go`（读构建列表与本机模块缓存，缺源码时从 pkg.go.dev 补取，不改写 `go.mod` / `go.sum`）自动补齐本表并归档到 `third_party/licenses/go/<模块路径>/`。

## Java（pom.xml 声明依赖）

本仓当前没有 Java 源码（无 `pom.xml`），`java` 子命令只输出空组件清单，Java 组件表因此为空。

<!--JAVA_COMPONENTS_START-->
| 组件 | 版本 | 许可证 | 版权 / 主页 |
| --- | --- | --- | --- |
| disruptor（dev 依赖：仅测试/基准使用，不随 jar 分发） | 3.4.4 | Apache-2.0 | <http://lmax-exchange.github.com/disruptor> |
| junit-jupiter（dev 依赖：仅测试/基准使用，不随 jar 分发） | 5.8.2 | EPL-2.0 | <https://junit.org/junit5/> |
<!--JAVA_COMPONENTS_END-->

许可证全文：`third_party/licenses/java/<artifactId>-<version>/`（从本机 m2 仓库 jar 内提取）；依赖声明、源码 import 与归档由 `panshi.tool.license.audit` 的 `java` 子命令审计核验，见「归档目录」与「更新流程」。

## Rust 完整依赖闭包（机器生成）

生成/刷新命令（仓库根执行，`panshi.tool.license.audit` 工具按语言子命令）：

```text
labs-license-audit rust
```

组件按许可证分组如下（每组件附 purl 与 Cargo.lock checksum sha256，可与 CycloneDX SBOM / Dependency-Track 逐条对应；多许可组件按 SPDX "OR" 选择其一履行，选择依据与 copyleft 触发条件见各组标题与上文原则）：

<!--RUST_CLOSURE_START-->
### (Apache-2.0 OR MIT) AND BSD-3-Clause（各许可同时适用，同时履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| encoding_rs | 0.8.35 | pkg:cargo/encoding_rs@0.8.35 | 75030f3c4f45dafd7586dd6780965a8c7e8e285a5ecb86713e63a79c5b2766f3 |

### (MIT OR Apache-2.0) AND Unicode-3.0（各许可同时适用，同时履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| unicode-ident | 1.0.24 | pkg:cargo/unicode-ident@1.0.24 | e6e4313cd5fcd3dad5cafa179702e2b244f760991f45397d14d4ebf38247da75 |

### 0BSD（单一许可，按 0BSD 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| mock_instant | 0.6.1 | pkg:cargo/mock_instant@0.6.1 | 9bb517913cfcfb9eeda59f36020269075a152701a01606c612f547e4890be399 |

### 0BSD OR MIT OR Apache-2.0（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| adler2 | 2.0.1 | pkg:cargo/adler2@2.0.1 | 320119579fcad9c21884f5c4861d16174d0e06250625266f50fe6898340abefa |

### Apache-2.0（单一许可，按 Apache-2.0 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| ciborium | 0.2.2 | pkg:cargo/ciborium@0.2.2 | 42e69ffd6f0917f5c029256a24d0161db17cea3997d185db0d35926308770f0e |
| ciborium-io | 0.2.2 | pkg:cargo/ciborium-io@0.2.2 | 05afea1e0a06c9be33d539b876f1ce3692f4afea2cb41f740e7743225ed1c757 |
| ciborium-ll | 0.2.2 | pkg:cargo/ciborium-ll@0.2.2 | 57663b653d948a338bfb3eeba9bb2fd5fcfaecb9e199e87e1eda4d9e8b240fd9 |
| openssl | 0.10.81 | pkg:cargo/openssl@0.10.81 | 77823a27f0babb03091cb9ed9ef80af3b39dbc82f97e8fa530374b7dafd87a45 |
| similar | 2.7.0 | pkg:cargo/similar@2.7.0 | bbbb5d9659141646ae647b42fe094daf6c6192d1620870b449d9557f748b2daa |
| sync_wrapper | 0.1.2 | pkg:cargo/sync_wrapper@0.1.2 | 2047c6ded9c721764247e62cd3b03c09ffc529b2ba5b10ec482ae507a4a70160 |

### Apache-2.0 / MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| fnv | 1.0.7 | pkg:cargo/fnv@1.0.7 | 3f9eec918d3f24069decb9af1554cad7c880e2da24a9afd88aca000531ab82c1 |

### Apache-2.0 AND ISC（各许可同时适用，同时履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| ring | 0.17.14 | pkg:cargo/ring@0.17.14 | a4689e6c2294d81e88dc6261c768b63bc4fcdb852be6d1352498b114f61383b7 |

### Apache-2.0 OR BSL-1.0（按 Apache-2.0 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| ryu | 1.0.23 | pkg:cargo/ryu@1.0.23 | 9774ba4a74de5f7b1c1451ed6cd5285a32eddb5cccb8cc655a4e50009e06477f |

### Apache-2.0 OR ISC OR MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| hyper-rustls | 0.24.2 | pkg:cargo/hyper-rustls@0.24.2 | ec3efd23720e2049821a693cbc7e65ea87c72f1c58ff2f9522ff332b1491e590 |
| rustls | 0.21.12 | pkg:cargo/rustls@0.21.12 | 3f56a14d1f48b391359b22f731fd4bd7e43c97f3c50eee276f3aa09c94784d3e |
| rustls-pemfile | 1.0.4 | pkg:cargo/rustls-pemfile@1.0.4 | 1c74cae0a4cf6ccbbf5f359f08efdf8ee7e1dc532573bf0db71968cb56b1448c |
| sct | 0.7.1 | pkg:cargo/sct@0.7.1 | da046153aa2352493d6cb7da4b6e5c0c057d8a1d0a9aa8560baffdd945acd414 |

### Apache-2.0 OR MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| atomic-waker | 1.1.2 | pkg:cargo/atomic-waker@1.1.2 | 1505bd5d3d116872e7271a6d4e16d81d0c8570876c8de68093a09ac269d8aac0 |
| autocfg | 1.5.1 | pkg:cargo/autocfg@1.5.1 | f2032f911046de80f0a198e0901378627c33f59ea0ac00e363d481118bd70a53 |
| base64ct | 1.8.3 | pkg:cargo/base64ct@1.8.3 | 2af50177e190e07a26ab74f8b1efbfe2ef87da2116221318cb1c2e82baf7de06 |
| criterion | 0.5.1 | pkg:cargo/criterion@0.5.1 | f2b12d017a929603d80db1831cd3a24082f8137ce19c69e6447f54f5fc8d692f |
| encode_unicode | 1.0.0 | pkg:cargo/encode_unicode@1.0.0 | 34aa73646ffb006b8f5147f3dc182bd4bcb190227ce861fc4a4844bf8e3cb2c0 |
| equivalent | 1.0.2 | pkg:cargo/equivalent@1.0.2 | 877a4ace8713b0bcf2a4e7eec82529c029f1d0619886d18145fea96c3ffe5c0f |
| fastrand | 2.5.0 | pkg:cargo/fastrand@2.5.0 | da7c62ceae207dd37ea5b845da6a0696c799f85e97da1ab5b7910be3c1c80223 |
| idna_adapter | 1.2.2 | pkg:cargo/idna_adapter@1.2.2 | cb68373c0d6620ef8105e855e7745e18b0d00d3bdb07fb532e434244cdb9a714 |
| indexmap | 2.14.2 | pkg:cargo/indexmap@2.14.2 | cc4e190f5d26ca7051642629da2c52fc03bde85a03197c99408dcd291734c855 |
| pin-project-lite | 0.2.17 | pkg:cargo/pin-project-lite@0.2.17 | a89322df9ebe1c1578d689c92318e070967d1042b512afbe49518723f4e6d5cd |
| portable-atomic | 1.15.0 | pkg:cargo/portable-atomic@1.15.0 | 05c8b63e8d9609db387f0324918f81d68fe27748f084ef092fb35954d0539a85 |
| tinytemplate | 1.2.1 | pkg:cargo/tinytemplate@1.2.1 | be4d6b5f19ff7664e8c98d03e2139cb510db9b0a60b55f8e8709b689d939b6bc |
| utf8_iter | 1.0.4 | pkg:cargo/utf8_iter@1.0.4 | b6c140620e7ffbb22c2dee59cafe6084a59b5ffc27a8859a5f0d494b5d52b6be |
| uuid | 1.26.0 | pkg:cargo/uuid@1.26.0 | b5772d71c9be8a8a6ac2117d949c5b224c1b72241bb611d9a3012edcf8af7812 |

### Apache-2.0 WITH LLVM-exception OR Apache-2.0 OR MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| linux-raw-sys | 0.12.1 | pkg:cargo/linux-raw-sys@0.12.1 | 32a66949e030da00e8c7d4434b251670a91556f4144941d37452769c25d58a53 |
| rustix | 1.1.4 | pkg:cargo/rustix@1.1.4 | b6fe4565b9518b83ef4f91bb47ce29620ca828bd32cb7e408f0062e9930ba190 |
| wasi | 0.11.1+wasi-snapshot-preview1 | pkg:cargo/wasi@0.11.1+wasi-snapshot-preview1 | ccf3ec651a847eb01de73ccad15eb7d99f80485de043efb2f370cd654f4ea44b |
| wasip2 | 1.0.4+wasi-0.2.12 | pkg:cargo/wasip2@1.0.4+wasi-0.2.12 | b67efb37e106e55ce722a510d6b5f9c17f083e5fc79afc2badeb12cc313d9487 |
| wit-bindgen | 0.57.1 | pkg:cargo/wit-bindgen@0.57.1 | 1ebf944e87a7c253233ad6766e082e3cd714b5d03812acc24c318f549614536e |

### Apache-2.0/MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| signal-hook | 0.3.18 | pkg:cargo/signal-hook@0.3.18 | d881a16cf4426aa584979d30bd82cb33429027e42122b169753d6ef1085ed6e2 |

### BSD-2-Clause OR Apache-2.0 OR MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| zerocopy | 0.8.56 | pkg:cargo/zerocopy@0.8.56 | 556764e583adb45a9f8d413c2a147fa7e8d821e48e12b14fd560b607998b75eb |
| zerocopy-derive | 0.8.56 | pkg:cargo/zerocopy-derive@0.8.56 | f2ab42fc20575779bd240faa45f94a74256f755c0fa9e89f0ede20d91d0cdfc1 |

### BSD-3-Clause（单一许可，按 BSD-3-Clause 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| subtle | 2.6.1 | pkg:cargo/subtle@2.6.1 | 13c2bddecc57b384dee18652358fb23172facb8a2c51ccc10d74c157bdea3292 |
| zstd-sys | 2.1.0+zstd.1.5.7 | pkg:cargo/zstd-sys@2.1.0+zstd.1.5.7 | 0ef0a8027ec3ee71300ab3bcbcd0393f434aa72b91ca6d635a39941deae8eea0 |

### CC0-1.0（单一许可，按 CC0-1.0 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| constant_time_eq | 0.1.5 | pkg:cargo/constant_time_eq@0.1.5 | 245097e9a4535ee1e3e3931fcfcd55a796a44c643e8596ff6566d68f09b87bbc |
| encoding-index-japanese | 1.20141219.5 | pkg:cargo/encoding-index-japanese@1.20141219.5 | 04e8b2ff42e9a05335dbf8b5c6f7567e5591d0d916ccef4e0b1710d32a0d0c91 |
| encoding-index-korean | 1.20141219.5 | pkg:cargo/encoding-index-korean@1.20141219.5 | 4dc33fb8e6bcba213fe2f14275f0963fd16f0a02c878e3095ecfdf5bee529d81 |
| encoding-index-simpchinese | 1.20141219.5 | pkg:cargo/encoding-index-simpchinese@1.20141219.5 | d87a7194909b9118fc707194baa434a4e3b0fb6a5a757c73c3adb07aa25031f7 |
| encoding-index-singlebyte | 1.20141219.5 | pkg:cargo/encoding-index-singlebyte@1.20141219.5 | 3351d5acffb224af9ca265f435b859c7c01537c0849754d3db3fdf2bfe2ae84a |
| encoding-index-tradchinese | 1.20141219.5 | pkg:cargo/encoding-index-tradchinese@1.20141219.5 | fd0e20d5688ce3cab59eb3ef3a2083a5c77bf496cb798dc6fcdb75f323890c18 |
| encoding_index_tests | 0.1.4 | pkg:cargo/encoding_index_tests@0.1.4 | a246d82be1c9d791c5dfde9a2bd045fc3cbba3fa2b11ad558f27d01712f00569 |

### ISC（单一许可，按 ISC 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| rustls-webpki | 0.101.7 | pkg:cargo/rustls-webpki@0.101.7 | 8b6275d1ee7a1cd780b64aca7726599a1dbc893b1e64144529e55c3c2f745765 |
| untrusted | 0.9.0 | pkg:cargo/untrusted@0.9.0 | 8ecb6da28b8a351d773b68d5825ac39017e680750f980f3a1a85cd8dd28a47c1 |

### MIT（单一许可，按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| assert-json-diff | 2.0.2 | pkg:cargo/assert-json-diff@2.0.2 | 47e4f2b81832e72834d7518d8487a0396a28cc408186a2e8854c0f98011faf12 |
| block2 | 0.6.2 | pkg:cargo/block2@0.6.2 | cdeb9d870516001442e364c5220d3574d2da8dc765554b4a617230d33fa58ef5 |
| bytes | 1.12.1 | pkg:cargo/bytes@1.12.1 | fc652a48c352aef3ea3aed32080501cf3ef6ed5da78602a020c991775b0aff04 |
| cfg_aliases | 0.2.2 | pkg:cargo/cfg_aliases@0.2.2 | f079e83a288787bcd14a6aea84cee5c87a67c5a3e660c30f557a3d24761b3527 |
| console | 0.15.11 | pkg:cargo/console@0.15.11 | 054ccb5b10f9f2cbf51eb355ca1d05c2d279ce1804688d0db74b4733a5aeafd8 |
| crunchy | 0.2.4 | pkg:cargo/crunchy@0.2.4 | 460fbee9c2c2f33933d720630a6a0bac33ba7053db5344fac858d4b8952d77d5 |
| derive_more | 2.1.1 | pkg:cargo/derive_more@2.1.1 | d751e9e49156b02b44f9c1815bcb94b984cdcc4396ecc32521c739452808b134 |
| derive_more-impl | 2.1.1 | pkg:cargo/derive_more-impl@2.1.1 | 799a97264921d8623a957f6c3b9011f3b5492f557bbb7a5a19b7fa6d06ba8dcb |
| encoding | 0.2.33 | pkg:cargo/encoding@0.2.33 | 6b0d943856b990d12d3b55b359144ff341533e516d94098b1d3fc1ac666d36ec |
| generic-array | 0.14.7 | pkg:cargo/generic-array@0.14.7 | 85649ca51fd72272d7821adaf274ad91c288277713d9c18820d8499a7ff69e9a |
| h2 | 0.4.19 | pkg:cargo/h2@0.4.19 | ef8e5e5a340588f4452631496976cf8636d4a7ecf600239fdc27615d2530bc16 |
| http-body | 1.1.0 | pkg:cargo/http-body@1.1.0 | ca2a8f2913ee65f60facd6a5905613afaa448497a0230cc41ce022d93290bc2c |
| http-body-util | 0.1.5 | pkg:cargo/http-body-util@0.1.5 | 23169fe34a5fbcdd3f3862e78fb9b6fccd5f02a6dc6f732547005d45631ce71c |
| hyper | 1.11.1 | pkg:cargo/hyper@1.11.1 | 27b501faa50e7a26c3d3560ca625132f4078a17771f4810baf70475ae48cbe43 |
| hyper-util | 0.1.20 | pkg:cargo/hyper-util@0.1.20 | 96547c2556ec9d12fb1578c4eaf448b04993e7fb79cbaad930a656880a6bdfa0 |
| indicatif | 0.17.11 | pkg:cargo/indicatif@0.17.11 | 183b3088984b400f4cfac3620d5e076c84da5364016b4f49473de574b2586235 |
| is-terminal | 0.4.17 | pkg:cargo/is-terminal@0.4.17 | 3640c1c38b8e4e43584d8df18be5fc6b0aa314ce6ebf51b53313d4306cca8e46 |
| libmimalloc-sys | 0.1.49 | pkg:cargo/libmimalloc-sys@0.1.49 | 6a45a52f43e1c16f667ccfe4dd8c85b7f7c204fd5e3bf46c5b0db9a5c3c0b8e9 |
| libredox | 0.1.23 | pkg:cargo/libredox@0.1.23 | 8d8f1ea3f21fd3405dcaf6c9b5c1630af9afc422d9073ea39c5f6d6c772e08ed |
| mimalloc | 0.1.52 | pkg:cargo/mimalloc@0.1.52 | 2d4139bb28d14ad1facf21d5eb8825051b326e172d216b39f6d31df53cc97862 |
| mio | 1.2.3 | pkg:cargo/mio@1.2.3 | 4b18443e9c262bfe8fa82f51666e2642c53393f7e5c27b3e1aeab922cff5b9d8 |
| mockito | 1.7.2 | pkg:cargo/mockito@1.7.2 | 90820618712cab19cfc46b274c6c22546a82affcb3c3bdf0f29e3db8e1bb92c0 |
| nix | 0.31.3 | pkg:cargo/nix@0.31.3 | cf20d2fde8ff38632c426f1165ed7436270b44f199fc55284c38276f9db47c3d |
| nom | 7.1.3 | pkg:cargo/nom@7.1.3 | d273983c5a657a70a3e8f2a01329822f3b8c8172b73826411a55751e404a0a4a |
| number_prefix | 0.4.0 | pkg:cargo/number_prefix@0.4.0 | 830b246a0e5f20af87141b25c173cd1b609bd7779a4617d6ec582abaf90870f3 |
| objc2 | 0.6.4 | pkg:cargo/objc2@0.6.4 | 3a12a8ed07aefc768292f076dc3ac8c48f3781c8f2d5851dd3d98950e8c5a89f |
| objc2-encode | 4.1.0 | pkg:cargo/objc2-encode@4.1.0 | ef25abbcd74fb2609453eb695bd2f860d389e457f67dc17cafc8b8cbc89d0c33 |
| oorandom | 11.1.5 | pkg:cargo/oorandom@11.1.5 | d6790f58c7ff633d8771f42965289203411a5e5c68388703c06e14f24770b41e |
| openssl-sys | 0.9.117 | pkg:cargo/openssl-sys@0.9.117 | b47e7e6bb2c38cd930d25a23b40fa52e068c10e85f3e03a7f5ba5aaca5713695 |
| ordered-float | 2.10.1 | pkg:cargo/ordered-float@2.10.1 | 68f19d67e5a2795c94e73e0bb1cc1a7edeb2e28efd39e2e1c9b7a40c1108b11c |
| plotters | 0.3.7 | pkg:cargo/plotters@0.3.7 | 5aeb6f403d7a4911efb1e33402027fc44f29b5bf6def3effcc22d7bb75f2b747 |
| plotters-backend | 0.3.7 | pkg:cargo/plotters-backend@0.3.7 | df42e13c12958a16b3f7f4386b9ab1f3e7933914ecea48da7139435263a4172a |
| plotters-svg | 0.3.7 | pkg:cargo/plotters-svg@0.3.7 | 51bae2ac328883f7acdfea3d66a7c35751187f870bc81f94563733a154d7a670 |
| redox_syscall | 0.5.18 | pkg:cargo/redox_syscall@0.5.18 | ed2bf2547551a7053d6fdfafda3f938979645c44812fbfcda098faae3f1a362d |
| redox_users | 0.4.6 | pkg:cargo/redox_users@0.4.6 | ba009ff324d1fc1b900bd1fdb31564febe58a8ccc8a6fdbb93b543d33b13ca43 |
| schannel | 0.1.29 | pkg:cargo/schannel@0.1.29 | 91c1b7e4904c873ef0710c1f407dde2e6287de2bebc1bbbf7d430bb7cbffd939 |
| serde-value | 0.7.0 | pkg:cargo/serde-value@0.7.0 | f3a1a3341211875ef120e117ea7fd5228530ae7e7036a779fdc9117be6b3282c |
| simd-adler32 | 0.3.10 | pkg:cargo/simd-adler32@0.3.10 | 3a219298ac11a56ea9a6d2120044824d6f01aeb034955e7af7bc16858527deea |
| slab | 0.4.12 | pkg:cargo/slab@0.4.12 | 0c790de23124f9ab44544d7ac05d60440adc586479ce501c1d6d7da3cd8c9cf5 |
| synstructure | 0.13.2 | pkg:cargo/synstructure@0.13.2 | 728a70f3dbaf5bab7f0c4b1ac8d7ae5ea60a4b5549c8a5914361c99147a709d2 |
| tokio | 1.53.1 | pkg:cargo/tokio@1.53.1 | 202caea871b69668250d242070849eb495be178ed697a3e98aebce5bc81a0bed |
| tokio-macros | 2.7.2 | pkg:cargo/tokio-macros@2.7.2 | 78773a2a397f451582ce068015985c33193cf6dea8b74d2a639fe457b2f07b0e |
| tokio-native-tls | 0.3.1 | pkg:cargo/tokio-native-tls@0.3.1 | bbae76ab933c85776efabc971569dd6119c580d8f5d448769dec1764bf796ef2 |
| tokio-util | 0.7.19 | pkg:cargo/tokio-util@0.7.19 | 494815d09bf52b5548659851081238f0ca39ff638363907596da739561c62c52 |
| tower-service | 0.3.3 | pkg:cargo/tower-service@0.3.3 | 8df9b6e13f2d32c91b9bd719c00d1958837bc7dec474d94952798cc8e69eeec3 |
| tracing | 0.1.44 | pkg:cargo/tracing@0.1.44 | 63e71662fa4b2a2c3a26f570f037eb95bb1f85397f3cd8076caed2f026a6d100 |
| tracing-attributes | 0.1.31 | pkg:cargo/tracing-attributes@0.1.31 | 7490cfa5ec963746568740651ac6781f701c9c5ea257c58e057f3ba8cf69e8da |
| tracing-core | 0.1.36 | pkg:cargo/tracing-core@0.1.36 | db97caf9d906fbde555dd62fa95ddba9eecfd14cb388e4f491a66d74cd5fb79a |
| try-lock | 0.2.5 | pkg:cargo/try-lock@0.2.5 | e421abadd41a4225275504ea4d6566923418b7f05506fbc9c0fe86ba7396114b |
| typemap-ors | 1.0.0 | pkg:cargo/typemap-ors@1.0.0 | a68c24b707f02dd18f1e4ccceb9d49f2058c2fb86384ef9972592904d7a28867 |
| unsafe-any-ors | 1.0.0 | pkg:cargo/unsafe-any-ors@1.0.0 | e0a303d30665362d9680d7d91d78b23f5f899504d4f08b3c4cf08d055d87c0ad |
| unsafe-libyaml | 0.2.11 | pkg:cargo/unsafe-libyaml@0.2.11 | 673aac59facbab8a9007c7f6108d11f63b603f7cabff99fabf650fea5c32b861 |
| want | 0.3.1 | pkg:cargo/want@0.3.1 | bfa7760aed19e106de2c7c0b581b509f2f25d3dacaf737cb82ac61bc6d760b0e |
| winreg | 0.50.0 | pkg:cargo/winreg@0.50.0 | 524e57b2c537c0f9b1e69f1965311ec12182b4122e45035b1508cd24d2adadb1 |
| zip | 0.6.6 | pkg:cargo/zip@0.6.6 | 760394e246e4c28189f19d488c058bf16f564016aefac5d32bb1f3b51d5e9261 |
| zmij | 1.0.23 | pkg:cargo/zmij@1.0.23 | 29666d0abbfad1e3dc4dcf6144730dd3a3ab225bbbdac83319345b1b44ccfc1b |
| zstd | 0.11.2+zstd.1.5.2 | pkg:cargo/zstd@0.11.2+zstd.1.5.2 | 20cc960326ece64f010d2d2107537f26dc589a6573a316bd5b1dba685fa5fde4 |

### MIT OR Apache-2.0（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| aes | 0.8.4 | pkg:cargo/aes@0.8.4 | b169f7a6d4742236a0a00c541b845991d0ac43e546831af1249753ab4c3aa3a0 |
| android_system_properties | 0.1.6 | pkg:cargo/android_system_properties@0.1.6 | ae221649c9976a6f6c56ae1facf410f3ddb33cc661c4b7b61020a912d4237fbc |
| anes | 0.1.6 | pkg:cargo/anes@0.1.6 | 4b46cbb362ab8752921c97e041f5e366ee6297bd428a31275b9fcf1e380f7299 |
| anstyle | 1.0.14 | pkg:cargo/anstyle@1.0.14 | 940b3a0ca603d1eade50a4846a2afffd5ef57a9feac2c0e2ec2e14f9ead76000 |
| anyhow | 1.0.104 | pkg:cargo/anyhow@1.0.104 | 330a5ed07fa54e4702c9d6c4174f74427fc0ef6e214bbd677ae50a5099946470 |
| arc-swap | 1.9.2 | pkg:cargo/arc-swap@1.9.2 | c049c0be4daef0b145cb3555416b3b8ef5b7888a38aea1a3a155801fe7b0810b |
| base64 | 0.21.7 | pkg:cargo/base64@0.21.7 | 9d297deb1925b89f2ccc13d7635fa0714f12c87adce1c75356b39ca9b7178567 |
| bitflags | 2.13.1 | pkg:cargo/bitflags@2.13.1 | b588b76d00fde79687d7646a9b5bdf3cc0f655e0bbd080335a95d7e96f3587da |
| block-buffer | 0.10.4 | pkg:cargo/block-buffer@0.10.4 | 3078c7629b62d3f0439517fa394996acacc5cbc91c5a20d8c658e77abd503a71 |
| bumpalo | 3.20.3 | pkg:cargo/bumpalo@3.20.3 | 72f5acc6cb2ba439de613abc23857ec3d78374d8ed5ac84e9d11336e87da8649 |
| cast | 0.3.0 | pkg:cargo/cast@0.3.0 | 37b2a672a2cb129a2e41c10b1224bb368f9f37a2b16b612598138befd7b37eb5 |
| cc | 1.4.5 | pkg:cargo/cc@1.4.5 | 005ec2760ca554fae18df7a11195552ec576cd665632a881bc011d5bb2fd4d80 |
| cfg-if | 1.0.4 | pkg:cargo/cfg-if@1.0.4 | 9330f8b2ff13f34540b44e946ef35111825727b38d33286ef986142615121801 |
| chrono | 0.4.45 | pkg:cargo/chrono@0.4.45 | 1aa79e62e7697b8e29b513a68abacf485adcd1fe8284a4316c5ae868e6633327 |
| cipher | 0.4.4 | pkg:cargo/cipher@0.4.4 | 773f3b9af64447d2ce9850330c473515014aa235e6a783b02db81ff39e4a3dad |
| clap | 4.6.6 | pkg:cargo/clap@4.6.6 | 473c7e07f409a8d772161724aa8db6a765a2532a70f9667eeb7b49d3d02fbdca |
| clap_builder | 4.6.6 | pkg:cargo/clap_builder@4.6.6 | 7b48fea5a88e9ae728a2dcbedbfc0e730f7d60da42e1cb049a83c9fb8b789889 |
| clap_lex | 1.1.0 | pkg:cargo/clap_lex@1.1.0 | c8d4a3bb8b1e0c1050499d1815f5ab16d04f0959b233085fb31653fbfc9d98f9 |
| core-foundation | 0.10.1 | pkg:cargo/core-foundation@0.10.1 | b2a6cd9ae233e7f62ba4e9353e81a88df7fc8a5987b8d445b4d90c879bd156f6 |
| core-foundation-sys | 0.8.7 | pkg:cargo/core-foundation-sys@0.8.7 | 773648b94d0e5d620f64f280777445740e61fe701025087ec8b57f45c791888b |
| cpufeatures | 0.2.17 | pkg:cargo/cpufeatures@0.2.17 | 59ed5838eebb26a2bb2e58f6d5b5316989ae9d08bab10e0e6d103e656d1b0280 |
| crc32fast | 1.5.1 | pkg:cargo/crc32fast@1.5.1 | 8498c871161e1742aaa9d52551b2d6ebdd4c3d45a3be423e3728f33b955be550 |
| cron | 0.12.1 | pkg:cargo/cron@0.12.1 | 6f8c3e73077b4b4a6ab1ea5047c37c57aee77657bc8ecd6f29b0af082d0b0c07 |
| crossbeam-deque | 0.8.8 | pkg:cargo/crossbeam-deque@0.8.8 | 622f3fc73690be383c7214310406f28a90e6edeadc3cea882f9d71e495b9711a |
| crossbeam-epoch | 0.9.21 | pkg:cargo/crossbeam-epoch@0.9.21 | dc74980687109a3b14c72fd458107bf0baa1da1a1a805e178d15501ba9b86d9d |
| crossbeam-utils | 0.8.23 | pkg:cargo/crossbeam-utils@0.8.23 | a31eee39dddec8330830986fcd7625edb5a24ec90ea038215273bbc3adb08ac6 |
| crypto-common | 0.1.7 | pkg:cargo/crypto-common@0.1.7 | 78c8292055d1c1df0cce5d180393dc8cce0abec0a7102adb6c7b1eef6016d60a |
| deranged | 0.5.8 | pkg:cargo/deranged@0.5.8 | 7cd812cc2bc1d69d4764bd80df88b4317eaef9e773c75226407d9bc0876b211c |
| digest | 0.10.7 | pkg:cargo/digest@0.10.7 | 9ed9a281f7bc9b7576e61468ba615a66a5c8cfdff42420a70aa82701a3b1e292 |
| dirs | 4.0.0 | pkg:cargo/dirs@4.0.0 | ca3aa72a6f96ea37bbc5aa912f6788242832f75369bdfdadcb0e38423f100059 |
| dirs-sys | 0.3.7 | pkg:cargo/dirs-sys@0.3.7 | 1b1d1d91c932ef41c0f2663aa8b0ca0342d444d842c06914aa0a7e352d0bada6 |
| displaydoc | 0.2.7 | pkg:cargo/displaydoc@0.2.7 | c6232dd377dcc64799954cbd3a9bb882e9cdc1308ccd87b1c098f1fb2eaf82a8 |
| either | 1.18.0 | pkg:cargo/either@1.18.0 | 252afb9ae5eaa683babdc6a068b3f5726eb19e05070c731f9b2a23a7c3e8ed34 |
| env_logger | 0.10.2 | pkg:cargo/env_logger@0.10.2 | 4cd405aab171cb85d6735e5c8d9db038c17d3ca007a4d2c25f337935c3d90580 |
| errno | 0.3.14 | pkg:cargo/errno@0.3.14 | 39cab71617ae0d63f51a36d69f866391735b51691dbda63cf6f96d042b63efeb |
| find-msvc-tools | 0.1.12 | pkg:cargo/find-msvc-tools@0.1.12 | 3e0f1c7c3a72c66fd80abe965175f7523475c0489a87d3ff9d6e8c87d87a9d2d |
| flate2 | 1.1.10 | pkg:cargo/flate2@1.1.10 | 6e634e2e0ebac1ee034020da1ca582e17ffe4e0f5e985823721e168928136dcb |
| form_urlencoded | 1.2.2 | pkg:cargo/form_urlencoded@1.2.2 | cb4cb245038516f5f85277875cdaa4f7d2c9a0fa0468de06ed190163b1581fcf |
| futures-channel | 0.3.34 | pkg:cargo/futures-channel@0.3.34 | b1f9e3d69d39e4862ffed03ed071a76f9a13ba1d9109d355b0f0aa6b15e393c4 |
| futures-core | 0.3.34 | pkg:cargo/futures-core@0.3.34 | 92d699e522242e69e3003b94ecc1f960f3a5e015aa7c5d7486e65ad01dd94f5e |
| futures-io | 0.3.34 | pkg:cargo/futures-io@0.3.34 | 53c0fa8157de1303bfffdaa1cc2a673bfffb60102f76b0ef4441659124373fed |
| futures-sink | 0.3.34 | pkg:cargo/futures-sink@0.3.34 | 1944426bf7d03f1d14f708785e4b33efd750b36d48a157b836b3efc15ede8e1d |
| futures-task | 0.3.34 | pkg:cargo/futures-task@0.3.34 | cd417de3d1d015fc3bfd2b1ea46dfc7bab72ef86f1cc7cc9c78e728b34a6d1fd |
| futures-util | 0.3.34 | pkg:cargo/futures-util@0.3.34 | 0d50a92467f8ba5dd6e3ee5d4bd04d73ab2e4e1c44474a0674821dfce14b79bc |
| getrandom | 0.4.3 | pkg:cargo/getrandom@0.4.3 | 300e883d756b2e4ec94e02791f39b04b522276138852cfc41d9fb7e904106099 |
| half | 2.7.1 | pkg:cargo/half@2.7.1 | 6ea2d84b969582b4b1864a92dc5d27cd2b77b622a8d79306834f1be5ba20d84b |
| hashbrown | 0.17.1 | pkg:cargo/hashbrown@0.17.1 | ed5909b6e89a2db4456e54cd5f673791d7eca6732202bbf2a9cc504fe2f9b84a |
| hermit-abi | 0.5.3 | pkg:cargo/hermit-abi@0.5.3 | e17592d60ebacc7d5e169f4663c5f84f9161cc90328abcfe8456f41e4dfcb284 |
| hex | 0.4.3 | pkg:cargo/hex@0.4.3 | 7f24254aa9a54b5c858eaee2f5bccdb46aaf0e486a595ed5fd8f86ba55232a70 |
| hmac | 0.12.1 | pkg:cargo/hmac@0.12.1 | 6c49c37c09c17a53d937dfbb742eb3a961d65a994e6bcdcf37e7399d0cc8ab5e |
| http | 1.5.0 | pkg:cargo/http@1.5.0 | 918d3568bebf352712bc2ef3d46a8bcf1a75b373be6539de198e9105cbbf9ce0 |
| httparse | 1.10.1 | pkg:cargo/httparse@1.10.1 | 6dbf3de79e51f3d586ab4cb9d5c3e2c14aa28ed23d180cf89b4df0454a69cc87 |
| httpdate | 1.0.3 | pkg:cargo/httpdate@1.0.3 | df3b46402a9d5adb4c86a0cf463f42e19994e3ee891101b1841f30a545cb49a9 |
| humantime | 2.4.0 | pkg:cargo/humantime@2.4.0 | 15cdd26707701c53297e2fa6afb323d55fbc1d0810c3aec078ae3ef0424c3c15 |
| iana-time-zone | 0.1.65 | pkg:cargo/iana-time-zone@0.1.65 | e31bc9ad994ba00e440a8aa5c9ef0ec67d5cb5e5cb0cc7f8b744a35b389cc470 |
| iana-time-zone-haiku | 0.1.2 | pkg:cargo/iana-time-zone-haiku@0.1.2 | f31827a206f56af32e590ba56d5d2d085f558508192593743f16b2306495269f |
| idna | 1.1.0 | pkg:cargo/idna@1.1.0 | 3b0875f23caa03898994f6ddc501886a45c7d3d62d04d2d90788d47be1b1e4de |
| inout | 0.1.4 | pkg:cargo/inout@0.1.4 | 879f10e63c20629ecabbb64a8010319738c66a5cd0c29b02d63d272b03751d01 |
| ipnet | 2.12.2 | pkg:cargo/ipnet@2.12.2 | 791930b43c0d5973160d90a8f3894509f2b273430f5c5c73b668636d0287c5c0 |
| itoa | 1.0.18 | pkg:cargo/itoa@1.0.18 | 8f42a60cbdf9a97f5d2305f08a87dc4e09308d1276d28c869c684d7777685682 |
| jobserver | 0.1.35 | pkg:cargo/jobserver@0.1.35 | 1c00acbd29eabad4a2392fa0e921c874934dbbf4194312ad20f04a0ed67a3cb3 |
| js-sys | 0.3.105 | pkg:cargo/js-sys@0.3.105 | ce57d20d1ea864ce2ac172ab472d409214f4fd359f0b2a2775abdf522e2af99e |
| libc | 0.2.189 | pkg:cargo/libc@0.2.189 | 3eaf3ede3fee6db1a4c2ee091bf8a8b4dccdc6d17f656fb07896ee72867612f2 |
| lock_api | 0.4.14 | pkg:cargo/lock_api@0.4.14 | 224399e74b87b5f3557511d98dff8b14089b3dadafcab6bb93eab67d3aace965 |
| log | 0.4.34 | pkg:cargo/log@0.4.34 | f9f8bd3e56ce4dfc153cf470fffbfa98c7620958b312ca5c3a4b8d5181fd13c6 |
| log4rs | 1.4.0 | pkg:cargo/log4rs@1.4.0 | 3e947bb896e702c711fccc2bf02ab2abb6072910693818d1d6b07ee2b9dfd86c |
| memmap2 | 0.9.11 | pkg:cargo/memmap2@0.9.11 | d1219ed1b7f229ee7104d281dd01d6802fe28bb6e95d292942c4daacdeb798c0 |
| mime | 0.3.17 | pkg:cargo/mime@0.3.17 | 6877bb514081ee2a7ff5ef9de3281f14a4dd4bceac4c09388074a6b5df8a139a |
| native-tls | 0.2.18 | pkg:cargo/native-tls@0.2.18 | 465500e14ea162429d264d44189adc38b199b62b1c21eea9f69e4b73cb03bbf2 |
| num-conv | 0.2.2 | pkg:cargo/num-conv@0.2.2 | 521739c6d2bac4aa25192232afe6841231376b2b26d4d9fae5ecf8ca5772e441 |
| num-derive | 0.3.3 | pkg:cargo/num-derive@0.3.3 | 876a53fff98e03a936a674b29568b0e605f06b29372c2489ff4de23f1949743d |
| num-traits | 0.2.19 | pkg:cargo/num-traits@0.2.19 | 071dfc062690e90b734c0b2273ce72ad0ffa95f0c74596bc250dcfd960262841 |
| once_cell | 1.21.4 | pkg:cargo/once_cell@1.21.4 | 9f7c3e4beb33f85d45ae3e3a1792185706c8e16d043238c593331cc7cd313b50 |
| openssl-probe | 0.2.1 | pkg:cargo/openssl-probe@0.2.1 | 7c87def4c32ab89d880effc9e097653c8da5d6ef28e6b539d313baaacfbafcbe |
| parking_lot | 0.12.5 | pkg:cargo/parking_lot@0.12.5 | 93857453250e3077bd71ff98b6a65ea6621a19bb0f559a85248955ac12c45a1a |
| parking_lot_core | 0.9.12 | pkg:cargo/parking_lot_core@0.9.12 | 2621685985a2ebf1c516881c026032ac7deafcda1a2c9b7850dc81e3dfcb64c1 |
| password-hash | 0.4.2 | pkg:cargo/password-hash@0.4.2 | 7676374caaee8a325c9e7a2ae557f216c5563a171d6997b0ef8a65af35147700 |
| pbkdf2 | 0.11.0 | pkg:cargo/pbkdf2@0.11.0 | 83a0692ec44e4cf1ef28ca317f14f8f07da2d95ec3fa01f86e4467b725e60917 |
| percent-encoding | 2.3.2 | pkg:cargo/percent-encoding@2.3.2 | 9b4f627cb1b25917193a259e49bdad08f671f8d9708acfd5fe0a8c1455d87220 |
| pkg-config | 0.3.34 | pkg:cargo/pkg-config@0.3.34 | f6b464fbc74e149a392436b17d523f769e057cb6877f6a5c4618bc6f11800548 |
| powerfmt | 0.2.0 | pkg:cargo/powerfmt@0.2.0 | 439ee305def115ba05938db6eb1644ff94165c5ab5e9420d1c1bcedbba909391 |
| ppv-lite86 | 0.2.21 | pkg:cargo/ppv-lite86@0.2.21 | 85eae3c4ed2f50dcfe72643da4befc30deadb458a9b590d720cde2f2b1e97da9 |
| proc-macro2 | 1.0.107 | pkg:cargo/proc-macro2@1.0.107 | 985e7ec9bb745e6ce6535b544d84d6cd6f7ad8bd711c398938ae983b91a766d9 |
| quote | 1.0.47 | pkg:cargo/quote@1.0.47 | 1fbf4db142a473a8d80c26bbf18454ed458bf8d26c8219c331daecfdbd079001 |
| rand | 0.9.5 | pkg:cargo/rand@0.9.5 | b9ef1d0d795eb7d84685bca4f72f3649f064e6641543d3a8c415898726a57b41 |
| rand_chacha | 0.9.0 | pkg:cargo/rand_chacha@0.9.0 | d3022b5f1df60f26e1ffddd6c66e8aa15de382ae63b3a0c1bfc0e4d3e3f325cb |
| rand_core | 0.9.5 | pkg:cargo/rand_core@0.9.5 | 76afc826de14238e6e8c374ddcc1fa19e374fd8dd986b0d2af0d02377261d83c |
| rayon | 1.12.0 | pkg:cargo/rayon@1.12.0 | fb39b166781f92d482534ef4b4b1b2568f42613b53e5b6c160e24cfbfa30926d |
| rayon-core | 1.13.0 | pkg:cargo/rayon-core@1.13.0 | 22e18b0f0062d30d4230b2e85ff77fdfe4326feb054b9783a3460d8435c8ab91 |
| regex | 1.13.1 | pkg:cargo/regex@1.13.1 | f020237b6c8eed93db2e2cb53c00c60a8e1bc73da7d073199a1180401450218d |
| regex-automata | 0.4.18 | pkg:cargo/regex-automata@0.4.18 | ad8553b9b26413251cbf30e620595c7a41b3887f03da04579c0e6b0d6a06b4b2 |
| regex-syntax | 0.8.11 | pkg:cargo/regex-syntax@0.8.11 | d6f6ff9a378485b298a5286656da665ba74413d36db0979633275d2e708145d4 |
| reqwest | 0.11.27 | pkg:cargo/reqwest@0.11.27 | dd67538700a17451e7cba03ac727fb961abb7607553461627b97de0b89cf4a62 |
| rustc_version | 0.4.1 | pkg:cargo/rustc_version@0.4.1 | cfcb3a22ef46e85b45de6ee7e79d063319ebb6594faafcf1c225ea92ab6e9b92 |
| rustversion | 1.0.23 | pkg:cargo/rustversion@1.0.23 | cf54715a573b99ac80df0bc206da022bcd442c974952c7b9720069370852e21f |
| scopeguard | 1.2.0 | pkg:cargo/scopeguard@1.2.0 | 94143f37725109f92c262ed2cf5e59bce7498c01bcc1502d7b9afe439a4e9f49 |
| security-framework | 3.7.0 | pkg:cargo/security-framework@3.7.0 | b7f4bc775c73d9a02cde8bf7b2ec4c9d12743edf609006c7facc23998404cd1d |
| security-framework-sys | 2.17.0 | pkg:cargo/security-framework-sys@2.17.0 | 6ce2691df843ecc5d231c0b14ece2acc3efb62c0a398c7e1d875f3983ce020e3 |
| semver | 1.0.28 | pkg:cargo/semver@1.0.28 | 8a7852d02fc848982e0c167ef163aaff9cd91dc640ba85e263cb1ce46fae51cd |
| serde | 1.0.229 | pkg:cargo/serde@1.0.229 | 4148590afebada386688f18773da617792bf2ef03ffc1e4cbd2b1d45b023e0ba |
| serde_core | 1.0.229 | pkg:cargo/serde_core@1.0.229 | 67dca2c9c51e58a4791a4b1ed58308b39c64224d349a935ab5039aa360942a48 |
| serde_derive | 1.0.229 | pkg:cargo/serde_derive@1.0.229 | e7a5d71263a5a7d47b41f6b3f06ba276f10cc18b0931f1799f710578e2309348 |
| serde_json | 1.0.151 | pkg:cargo/serde_json@1.0.151 | c841b55ecdae098c80dcae9cf767f6f8a0c2cdb3416bbef72181df4d0fe73f14 |
| serde_repr | 0.1.21 | pkg:cargo/serde_repr@0.1.21 | 8d3b1629de253c70a0508c3899572da79ca359fdab27c7920ff00406df418906 |
| serde_yaml | 0.9.34+deprecated | pkg:cargo/serde_yaml@0.9.34+deprecated | 6a8b1a1a2ebf674015cc02edccce75287f1a0130d394307b36743c2f5d504b47 |
| sha1 | 0.10.7 | pkg:cargo/sha1@0.10.7 | a978451301f4db1d02937a4ab3ccce137717b81826e79b7d49ffe3244a13c3b8 |
| sha2 | 0.10.9 | pkg:cargo/sha2@0.10.9 | a7507d819769d01a365ab707794a4084392c824f54a7a6a7862f8c3d0892b283 |
| shlex | 2.0.1 | pkg:cargo/shlex@2.0.1 | f8fadd59c855ef2080decdef8ff161eb6661b86933c9d82e5ba29dc602a55aba |
| signal-hook-registry | 1.4.8 | pkg:cargo/signal-hook-registry@1.4.8 | c4db69cba1110affc0e9f7bcd48bbf87b3f4fc7c61fc9155afd4c469eb3d6c1b |
| smallvec | 1.16.0 | pkg:cargo/smallvec@1.16.0 | b9be42f50aa861c555654aa3a37f52f4b1074bacf4e48fe0ef7fa584e80f1f0f |
| socket2 | 0.6.5 | pkg:cargo/socket2@0.6.5 | c3d1e2c7f27f8d4cb10542a02c49005dbd6e93095799d6f3be745fae9f8fedd4 |
| stable_deref_trait | 1.2.1 | pkg:cargo/stable_deref_trait@1.2.1 | 6ce2be8dc25455e1f91df71bfa12ad37d7af1092ae736f3a6cd0e37bc7810596 |
| syn | 3.0.5 | pkg:cargo/syn@3.0.5 | 12df2e0110f65b775f769bb17ef989067a1d931b2eb822bd4346631eeada89f9 |
| system-configuration | 0.5.1 | pkg:cargo/system-configuration@0.5.1 | ba3a3adc5c275d719af8cb4272ea1c4a6d668a777f37e115f6d11ddbc1c8e0e7 |
| system-configuration-sys | 0.5.0 | pkg:cargo/system-configuration-sys@0.5.0 | a75fb188eb626b924683e3b95e3a48e63551fcfb51949de2f06a9d91dbee93c9 |
| tempfile | 3.27.0 | pkg:cargo/tempfile@3.27.0 | 32497e9a4c7b38532efcdebeef879707aa9f794296a4f0244f6f69e9bc8574bd |
| thiserror | 2.0.20 | pkg:cargo/thiserror@2.0.20 | ec86235f5fcc2a73650310756d2ac5b138a5780bbbdfae3eeccec992c435ba4f |
| thiserror-impl | 2.0.20 | pkg:cargo/thiserror-impl@2.0.20 | bc04cd3e1236dd4a98afca4569f2deb3f120e5422a4023be2cb683f8486292af |
| thread-id | 5.1.0 | pkg:cargo/thread-id@5.1.0 | 2010d27add3f3240c1fef7959f46c814487b216baee662af53be645ba7831c07 |
| time | 0.3.55 | pkg:cargo/time@0.3.55 | cdb87b95ec50ddfa440816d227a17b2ccbdda963a316a727fda0fc4334f7d134 |
| time-core | 0.1.9 | pkg:cargo/time-core@0.1.9 | 9e1c906769ad99c88eaa54e728060edef082f8e358ff32030cb7c7d315e81109 |
| typenum | 1.20.1 | pkg:cargo/typenum@1.20.1 | b6f5e870be6c3b371b77fe0ee0bafb859fa4964b4404c27de1d380043c4dda20 |
| unicode-segmentation | 1.13.3 | pkg:cargo/unicode-segmentation@1.13.3 | c6f5d3c3b1bf09027a88a6bc961fc00497d651009560b5463668dc81b0fa87a8 |
| unicode-width | 0.2.2 | pkg:cargo/unicode-width@0.2.2 | b4ac048d71ede7ee76d585517add45da530660ef4390e49b098733c6e897f254 |
| unicode-xid | 0.2.6 | pkg:cargo/unicode-xid@0.2.6 | ebc1c04c71510c7f702b52b7c350734c9ff1295c464a03335b00bb84fc54f853 |
| url | 2.5.8 | pkg:cargo/url@2.5.8 | ff67a8a4397373c3ef660812acab3268222035010ab8680ec4215f38ba3d0eed |
| wasm-bindgen | 0.2.128 | pkg:cargo/wasm-bindgen@0.2.128 | aecb87a33d3b0c5e3b7aa46336eaf486cffafbd281b195e4c8b80d50df2351bf |
| wasm-bindgen-futures | 0.4.78 | pkg:cargo/wasm-bindgen-futures@0.4.78 | 6ef4c5d3d2cdf5c54f4231181768f5510842e350db025faf1f7163b1030ed928 |
| wasm-bindgen-macro | 0.2.128 | pkg:cargo/wasm-bindgen-macro@0.2.128 | a690d511e3c1a8b3a55e33511e3c2c00c78415cd23650f32b808627f5696b9ed |
| wasm-bindgen-macro-support | 0.2.128 | pkg:cargo/wasm-bindgen-macro-support@0.2.128 | 411e4887f0071ef2d2164a9d5fdf2d20efbef78fccd3a78b0c10a1dc5295e48a |
| wasm-bindgen-shared | 0.2.128 | pkg:cargo/wasm-bindgen-shared@0.2.128 | 81941cd78d0c92026c33e5e01312845a4cb1e9af3407f9134b100dd03144103e |
| web-sys | 0.3.105 | pkg:cargo/web-sys@0.3.105 | 9fbddc4a036f00ec4f18c83445bd3115cb306a91da554919a099d9222fe4a7f8 |
| web-time | 1.1.0 | pkg:cargo/web-time@1.1.0 | 5a6580f308b1fad9207618087a65c04e7a10bc77e02c8e84e9b00dd4b12fa0bb |
| windows-core | 0.62.2 | pkg:cargo/windows-core@0.62.2 | b8e83a14d34d0623b51dce9581199302a221863196a1dde71a7663a4c2be9deb |
| windows-implement | 0.60.2 | pkg:cargo/windows-implement@0.60.2 | 053e2e040ab57b9dc951b72c264860db7eb3b0200ba345b4e4c3b14f67855ddf |
| windows-interface | 0.59.3 | pkg:cargo/windows-interface@0.59.3 | 3f316c4a2570ba26bbec722032c4099d8c8bc095efccdc15688708623367e358 |
| windows-link | 0.2.1 | pkg:cargo/windows-link@0.2.1 | f0805222e57f7521d6a62e36fa9163bc891acd422f971defe97d64e70d0a4fe5 |
| windows-result | 0.4.1 | pkg:cargo/windows-result@0.4.1 | 7781fa89eaf60850ac3d2da7af8e5242a5ea78d1a11c49bf2910bb5a73853eb5 |
| windows-strings | 0.5.1 | pkg:cargo/windows-strings@0.5.1 | 7837d08f69c77cf6b07689544538e017c1bfcf57e34b4c0ff58e6c2cd3b37091 |
| windows-sys | 0.61.2 | pkg:cargo/windows-sys@0.61.2 | ae137229bcbd6cdf0f7b80a31df61766145077ddf49416a728b02cb3921ff3fc |
| windows-targets | 0.52.6 | pkg:cargo/windows-targets@0.52.6 | 9b724f72796e036ab90c1021d4780d4d3d648aca59e491e6b98e725b84e99973 |
| windows_aarch64_gnullvm | 0.52.6 | pkg:cargo/windows_aarch64_gnullvm@0.52.6 | 32a4622180e7a0ec044bb555404c800bc9fd9ec262ec147edd5989ccd0c02cd3 |
| windows_aarch64_msvc | 0.52.6 | pkg:cargo/windows_aarch64_msvc@0.52.6 | 09ec2a7bb152e2252b53fa7803150007879548bc709c039df7627cabbd05d469 |
| windows_i686_gnu | 0.52.6 | pkg:cargo/windows_i686_gnu@0.52.6 | 8e9b5ad5ab802e97eb8e295ac6720e509ee4c243f69d781394014ebfe8bbfa0b |
| windows_i686_gnullvm | 0.52.6 | pkg:cargo/windows_i686_gnullvm@0.52.6 | 0eee52d38c090b3caa76c563b86c3a4bd71ef1a819287c19d586d7334ae8ed66 |
| windows_i686_msvc | 0.52.6 | pkg:cargo/windows_i686_msvc@0.52.6 | 240948bc05c5e7c6dabba28bf89d89ffce3e303022809e73deaefe4f6ec56c66 |
| windows_x86_64_gnu | 0.52.6 | pkg:cargo/windows_x86_64_gnu@0.52.6 | 147a5c80aabfbf0c7d901cb5895d1de30ef2907eb21fbbab29ca94c5b08b1a78 |
| windows_x86_64_gnullvm | 0.52.6 | pkg:cargo/windows_x86_64_gnullvm@0.52.6 | 24d5b23dc417412679681396f2b49f3de8c1473deb516bd34410872eff51ed0d |
| windows_x86_64_msvc | 0.52.6 | pkg:cargo/windows_x86_64_msvc@0.52.6 | 589f6da84c646204747d1270a2a5661ea66ed1cced2631d546fdfb155959f9ec |

### MIT OR Apache-2.0 OR LGPL-2.1-or-later（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| r-efi | 6.0.0 | pkg:cargo/r-efi@6.0.0 | f8dcc9c7d52a811697d2151c701e0d08956f92b0e24136cf4cf27b57a6a0d9bf |

### MIT OR Zlib OR Apache-2.0（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| miniz_oxide | 0.9.1 | pkg:cargo/miniz_oxide@0.9.1 | b63fbc4a50860e98e7b2aa7804ded1db5cbc3aff9193adaff57a6931bf7c4b4c |

### MIT/Apache-2.0（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| bzip2 | 0.4.4 | pkg:cargo/bzip2@0.4.4 | bdb116a6ef3f6c3698828873ad02c3014b3c85cadb88496095628e3ef1e347f8 |
| bzip2-sys | 0.1.13+1.0.8 | pkg:cargo/bzip2-sys@0.1.13+1.0.8 | 225bff33b2141874fe80d71e07d6eec4f85c5c216453dd96388240f96e1acc14 |
| criterion-plot | 0.5.0 | pkg:cargo/criterion-plot@0.5.0 | 6b50826342786a51a89e2da3a28f1c32b06e387201bc2d19791f622c673706b1 |
| ctrlc | 3.5.2 | pkg:cargo/ctrlc@3.5.2 | e0b1fab2ae45819af2d0731d60f2afe17227ebb1a1538a236da84c93e9a60162 |
| destructure_traitobject | 0.2.0 | pkg:cargo/destructure_traitobject@0.2.0 | 3c877555693c14d2f84191cfd3ad8582790fc52b5e2274b40b59cf5f5cea25c7 |
| filetime | 0.2.29 | pkg:cargo/filetime@0.2.29 | 5c287a33c7f0a620c38e641e7f60827713987b3c0f26e8ddc9462cc69cf75759 |
| foreign-types | 0.3.2 | pkg:cargo/foreign-types@0.3.2 | f6f339eb8adc052cd2ca78910fda869aefa38d22d5cb648e6485e4d3fc06f3b1 |
| foreign-types-shared | 0.1.1 | pkg:cargo/foreign-types-shared@0.1.1 | 00b0228411908ca8685dba7fc2cdd70ec9990a6e753e89b6ac91a84c40fbaf4b |
| hyper-tls | 0.5.0 | pkg:cargo/hyper-tls@0.5.0 | d6183ddfa99b85da61a140bea0efc93fdf56ceaa041b37d553518030827f9905 |
| itertools | 0.10.5 | pkg:cargo/itertools@0.10.5 | b0fd2260e829bddf4cb6ea802289de2f86d6a7a690192fbe91b3f46e0f2c8473 |
| log-mdc | 0.1.0 | pkg:cargo/log-mdc@0.1.0 | a94d21414c1f4a51209ad204c1776a3d0765002c76c6abcb602a6f09f1e881c7 |
| minimal-lexical | 0.2.1 | pkg:cargo/minimal-lexical@0.2.1 | 68354c5c6bd36d73ff3feceb05efa59b6acb7626617f4962be322a825e61f79a |
| openssl-macros | 0.1.1 | pkg:cargo/openssl-macros@0.1.1 | a948666b637a0f465e8564c73e89d4dde00d72d4d473cc972f390fc3dcee7d9c |
| serde_urlencoded | 0.7.1 | pkg:cargo/serde_urlencoded@0.7.1 | d3491c14715ca2294c4d6a88f15e84739788c1d030eed8c110436aafdaa2f3fd |
| tokio-cron-scheduler | 0.9.4 | pkg:cargo/tokio-cron-scheduler@0.9.4 | de2c1fd54a857b29c6cd1846f31903d0ae8e28175615c14a277aed45c58d8e27 |
| tokio-rustls | 0.24.1 | pkg:cargo/tokio-rustls@0.24.1 | c28327cf380ac148141087fbfb9de9d7bd4e84ab5d2c28fbc911d753de8a7081 |
| vcpkg | 0.2.15 | pkg:cargo/vcpkg@0.2.15 | accd4ea62f7bb7a82fe23066fb0957d48ef677f6eeb8215f372f52e48bb32426 |
| version_check | 0.9.5 | pkg:cargo/version_check@0.9.5 | 0b928f33d975fc6ad9f86c8f283853ad26bdd5b10b7f1542aa2fa15e2289105a |
| winapi | 0.3.9 | pkg:cargo/winapi@0.3.9 | 5c839a674fcd7a98952e593242ea400abe93992746761e38641405d28b00f419 |
| winapi-i686-pc-windows-gnu | 0.4.0 | pkg:cargo/winapi-i686-pc-windows-gnu@0.4.0 | ac3b87c63620426dd9b991e5ce0329eff545bccbbb34f3be09ff6fb6ab51b7b6 |
| winapi-x86_64-pc-windows-gnu | 0.4.0 | pkg:cargo/winapi-x86_64-pc-windows-gnu@0.4.0 | 712e227841d057c1ee1cd2fb22fa7e5a5461ae8e48fa2ca79ec42cfc1931183f |
| zstd-safe | 5.0.2+zstd.1.5.2 | pkg:cargo/zstd-safe@5.0.2+zstd.1.5.2 | 1d2a5585e04f9eea4b2a3d1eca508c4dee9592a89ef6f450c11719da0726f4db |

### MPL-2.0（单一许可，按 MPL-2.0 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| colored | 3.1.1 | pkg:cargo/colored@3.1.1 | faf9468729b8cbcea668e36183cb69d317348c2e08e994829fb56ebfdfbaac34 |
| webpki-roots | 0.25.4 | pkg:cargo/webpki-roots@0.25.4 | 5f20c57d8d7db6d3b86154206ae5d8fba62dd39573114de97c2cb0578251f8e1 |

### Unicode-3.0（单一许可，按 Unicode-3.0 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| icu_collections | 2.3.0 | pkg:cargo/icu_collections@2.3.0 | fa68d21081c4a05d5a901a1c62add574c77048b6a1c67be3b50ce0b60d4ca513 |
| icu_locale_core | 2.3.0 | pkg:cargo/icu_locale_core@2.3.0 | d56e28588da92eee5c3201a6eff33fabdd49b62269c8938d4ff050ce4d900deb |
| icu_normalizer | 2.3.0 | pkg:cargo/icu_normalizer@2.3.0 | 12f9cf5f235641ed274641dd81c3f28d870e276763d0797aeeab72317b1c646f |
| icu_normalizer_data | 2.3.0 | pkg:cargo/icu_normalizer_data@2.3.0 | 1563da1ed3e0b3bf3d74c9b85917ac9c56464d2f57242270c09c9e752f8021a0 |
| icu_properties | 2.3.0 | pkg:cargo/icu_properties@2.3.0 | 7e7ca276ad3145661a65914e6daf131ca5120cd3dcee8f8f3214b8875184a148 |
| icu_properties_data | 2.3.0 | pkg:cargo/icu_properties_data@2.3.0 | e590f038c1464a96894fd6d10127e90a8be4509f56ff7ecef851b15cee0b7caa |
| icu_provider | 2.3.1 | pkg:cargo/icu_provider@2.3.1 | d27bbb9d3abbefac45d55f647c9de1d44aafcd1186eb91879afef17c396c3e73 |
| litemap | 0.8.3 | pkg:cargo/litemap@0.8.3 | 47d9d19d1d6efa0109d2f65ff4c85cddd50bd572e5a00127ab10987290bcefae |
| potential_utf | 0.1.6 | pkg:cargo/potential_utf@0.1.6 | d83eb9bc6d8e5cf568e7a1101d60ee05e81ed50ea106026f3d18deeb046d7661 |
| tinystr | 0.8.4 | pkg:cargo/tinystr@0.8.4 | b1e27c91459209c2986af3dcf603a5a74a4368754ce37414f59acc971167f643 |
| writeable | 0.6.4 | pkg:cargo/writeable@0.6.4 | 3ad82d2a33cdc9674dc7465672f271e096168fcdbe0f799d9e6db8c5892679dc |
| yoke | 0.8.3 | pkg:cargo/yoke@0.8.3 | 709fe23a0424b6a435d82152b1bd3fdfb0833487d5fa90d05d42762a9891fef5 |
| yoke-derive | 0.8.2 | pkg:cargo/yoke-derive@0.8.2 | de844c262c8848816172cef550288e7dc6c7b7814b4ee56b3e1553f275f1858e |
| zerofrom | 0.1.8 | pkg:cargo/zerofrom@0.1.8 | 0ec05a11813ea801ff6d75110ad09cd0824ddba17dfe17128ea0d5f68e6c5272 |
| zerofrom-derive | 0.1.7 | pkg:cargo/zerofrom-derive@0.1.7 | 11532158c46691caf0f2593ea8358fed6bbf68a0315e80aae9bd41fbade684a1 |
| zerotrie | 0.2.5 | pkg:cargo/zerotrie@0.2.5 | 4ea269c3bd32f0a32c321907a2ae912ba6f4649bb0fc764a15627e99a7095a3f |
| zerovec | 0.11.8 | pkg:cargo/zerovec@0.11.8 | bb0464e17806c1d976d5cba29399c7f08e516e279e2ba493f63123b5fca67dd8 |
| zerovec-derive | 0.11.6 | pkg:cargo/zerovec-derive@0.11.6 | 34df6fc39dbd26ddc9c10e6a2984476e13acce22e64e4487636ef494369225da |

### Unlicense OR MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| aho-corasick | 1.1.5 | pkg:cargo/aho-corasick@1.1.5 | c982642fa9e8606056828ee9a8505737230110bb1099153c79efe865c59d12ba |
| byteorder | 1.5.0 | pkg:cargo/byteorder@1.5.0 | 1fd0f2584146f6f2ef48085050886acf353beff7305ebd1ae69500e27c67f64b |
| memchr | 2.8.3 | pkg:cargo/memchr@2.8.3 | cf8baf1c55e62ffcace7a9f06f4bd9cd3f0c4beb022d3b367256b91b87513d98 |
| termcolor | 1.4.1 | pkg:cargo/termcolor@1.4.1 | 06794f8f6c5c898b3275aebefa6b8a1cb24cd2c6c79397ab15774837a0bc5755 |
| winapi-util | 0.1.11 | pkg:cargo/winapi-util@0.1.11 | c2a7b1c03c876122aa43f3020e6c3c3ee5c05081c9a00739faf7503aeba10d22 |

### Unlicense/MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| csv | 1.4.0 | pkg:cargo/csv@1.4.0 | 52cd9d68cf7efc6ddfaaee42e7288d3a99d613d4b50f76ce9827ae0c6e14f938 |
| csv-core | 0.1.13 | pkg:cargo/csv-core@0.1.13 | 704a3c26996a80471189265814dbc2c257598b96b8a7feae2d31ace646bb9782 |
| same-file | 1.0.6 | pkg:cargo/same-file@1.0.6 | 93fc1dc3aaa9bfed95e02e6eadabb4baf7e3078b0bd1b4d7b6b0b68378900502 |
| walkdir | 2.5.0 | pkg:cargo/walkdir@2.5.0 | 29790946404f91d9c5d06f9874efddea1dc06c5efe94541a7d6863108e3a5e4b |

### Zlib（单一许可，按 Zlib 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| zlib-rs | 0.6.7 | pkg:cargo/zlib-rs@0.6.7 | 34b31d188d9d685a4f9c7b46d6e36631b07058d2cfe190267adce54dc230bf12 |

### Zlib OR Apache-2.0 OR MIT（按 MIT 履行）

| 组件 | 版本 | purl | sha256 |
| --- | --- | --- | --- |
| dispatch2 | 0.3.1 | pkg:cargo/dispatch2@0.3.1 | 1e0e367e4e7da84520dedcac1901e4da967309406d1e51017ae1abfb97adbd38 |
<!--RUST_CLOSURE_END-->

## 归档目录 `third_party/licenses/`（按语言拆分）

- `third_party/licenses/python/<name>-<version>/`：Python 组件许可证副本。生成/刷新：`labs-license-audit python`（pyproject 声明 + site-packages dist-info 导出；wheel 未随附许可证文件者已按上游官方 LICENSE 文本人工补录）；
- `third_party/licenses/rust/<crate>-<version>/`：Rust 组件许可证副本。生成/刷新：`labs-license-audit rust`（取自 cargo registry 缓存；上游 `.crate` 未随包分发许可证文件者已按 upstream 官方文本人工补录）；
- 同一组件库存在多个版本时只保留与声明一致的最高版本目录：`labs-license-audit <lang>` 刷新时删除被淘汰版本的 `<name>-<version>/`（人工补录的文本在升级前请自行确认）；目录缺失 / 同库多版本并存 / 版本与声明不符的三类情形会写入根 `THIRD-PARTY-TODO.md` 待办，人工清理后重跑即消失；依赖移除或改名后，其归档目录与表行一并消失（按声明面自动清理）；待办文案按成因给动作——归档版本高于声明说明本次扫描所用环境比归档时旧，先在发布所用环境重跑；库已不在声明里则确认后删除全部目录；
- C++ / Go / Java 侧当前无第三方组件，`third_party/licenses/` 下相应目录不存在；新增依赖后由同一命令生成。

生成脚本输出的"缺失组件"清单表示本地无缓存、需人工补录的项目——**发布二进制产物前必须补齐**（当前归档：python 17 + rust 39 = 56 个组件目录，无缺失）。

## 更新流程

新增或升级第三方依赖后：

1. 更新对应清单文件（`Cargo.toml` / `go.mod` / `conanfile.py` / `pyproject.toml` / `pom.xml`）；
2. Python：新增 pip 依赖须同步声明于 `pyproject.toml [project].dependencies`，`labs-license-audit python` 核验声明与源码 import 并刷新 `third_party/licenses/python/`（如有）；
3. C++：待 conan2 完成新依赖解析/缓存后，`labs-license-audit cpp` 刷新 `third_party/licenses/cpp/`；
4. Rust：`labs-license-audit rust` 刷新闭包表格（含 purl/sha256）与 `third_party/licenses/rust/` 归档；
5. Go：`labs-license-audit go` 刷新 `third_party/licenses/go/`（读构建列表与本机模块缓存，缺源码时从 pkg.go.dev 补取，不改写 `go.mod` / `go.sum`）；
6. Java：新增 maven 依赖须同步声明于 `pom.xml <dependencies>`，`labs-license-audit java` 从本机 m2 仓库提取 jar 内许可证并刷新 `third_party/licenses/java/`（如有）；
7. 各语言脚本新报的"缺失/待人工补录"项补齐后再发版；
8. 版本发布时，各语言产物随附 `THIRD-PARTY-NOTICES.md` 与本语言归档 `third_party/licenses/<lang>/`；仓库根 `THIRD-PARTY-NOTICES.md` 与 `third_party/licenses/` 完整归档随源码发布。

## SBOM 与合规映射

本项目每次发布产物均生成机器可读 SBOM（CycloneDX JSON/XML，由发布流程的 SBOM 工具生成，随产物分发而非常驻仓库），与 `THIRD-PARTY-NOTICES.md` 共同构成软件供应链合规证据：

| 证据类型 | 文件 | 用途 |
| --- | --- | --- |
| 法律声明 | `THIRD-PARTY-NOTICES.md` | 人类可读的许可证义务与履行选择说明 |
| 机器证据 | CycloneDX SBOM（JSON/XML） | 依赖闭包、版本、purl、哈希 |
| 原始数据 | `Cargo.lock` / `pyproject.toml` / `conanfile.py` / `go.mod` | 可复现构建（哈希与解析基线） |
| 审计工具 | `panshi.tool.license.audit` | 生成与校验本文件机器区段 |

对应关系：

- Rust 闭包区段已内嵌每条依赖的 `pkg:cargo/<crate>@<version>` 与 `sha256`（取自 `Cargo.lock` checksum），SBOM 中相同组件可通过同一 `purl` 与本文件条目一一对应，供 Dependency-Track 等工具精确匹配；
- 其它语言（Python / C++ / Go / Java）直接依赖表以「组件名 + 版本」为对应键，SBOM 的 `purl` 命名规则由各语言生态规范决定（如 `pkg:conan/...`、`pkg:golang/...`、`pkg:pypi/...`、`pkg:maven/...`）；
- 本文件是「法律义务台账」，SBOM 是「机器证据」，两者互补而非替代：许可选择依据（含 copyleft 触发条件判断）只在本文件记录，组件闭包与哈希以 SBOM / 锁文件为准。

## 审计工具

本文件的机器区段与 `third_party/licenses/` 归档由 `labs-license-audit` 生成与校验。该工具随 `panshi` 发行（PyPI 包名 `panshi`，MIT 许可），源码在 <https://codeberg.org/quant1x/foundations> 的 `panshi/tool/license/audit/`；它对任意项目通用——在目标仓库根目录执行，扫描的是该仓库自己的清单文件：

- 安装：`pip install panshi`，安装后即得 `labs-license-audit` 命令，也可用 `python -m panshi.tool.license.audit` 调用；在 panshi 源码树内作业时用 `pip install -e .`；
- 调用：`labs-license-audit <lang>...`，`<lang>` 取 `python` / `cpp` / `rust` / `go` / `java`；不带语言时按 python → c++ → rust → go → java 全跑；
- 数据来源：本机缓存（site-packages / conan 缓存 / cargo registry 源码缓存 / Go 模块缓存 / `~/.m2/repository`），Go 侧缓存缺源码或许可证文件时从 pkg.go.dev 补取，其余全程离线；
- 产物：刷新本文件的围栏区段、把各组件许可证全文归档到 `third_party/licenses/<lang>/<name>-<version>/`（同一库只留最高版本，旧版本目录随刷新清理）、把机器无法确定的事项写入根 `THIRD-PARTY-TODO.md`（无待办自动删除）；
- 退出码：`0` 指定对象全部成功且无待办；`1` 存在缺失 / 待人工处理项或工具链错误（其余语言继续执行）；
- 补录：缺许可证全文时用 `python -m panshi.tool.license.audit.fetch <url> <落盘路径>` 从上游取回，GitHub 来源自动加代理前缀，落盘后按响应的 `Last-Modified` 对齐文件时间；
- 工具定位、边界与各语言扫描口径见源码仓库中的模块文档 `panshi/tool/license/audit/README.md`。
