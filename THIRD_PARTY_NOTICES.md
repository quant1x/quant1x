# 第三方许可证与归属声明

本文件用于满足第三方组件的署名与许可证保留要求，随源码发布。

## 0. 分发形态

Quant1X **仅发布源代码，不发布二进制发行版**（不提供预编译的 `q1x`、静态库、安装包，Python 侧也只发布纯 Python 源码包/轮子，不含 C/C++ 产物与字体等非 Python 资源）。

因此：

- LGPL 等许可证中针对**二进制分发**的义务（提供可重链接的目标文件、随二进制附带库源码等）**不适用于本仓库的发布行为**；
- 本仓库仍需履行**源码分发**相关的义务：保留其实际分发的第三方源码的版权声明与许可证文本（如 `quant1x/io/csv-reader.h`、`third_party/` 下的头文件）；
- Rust crate 通过 Cargo `include` 白名单发布，只包含 Rust 源码与根目录文档，不包含 `third_party/`、C++ 头文件等非 Rust 发布物；本文件中的 C++ 许可证信息供仓库使用者与下游二进制分发者参考；
- 下游用户若基于本源码构建并**发布二进制**，相关义务由该分发者承担；本文件第 3 节可作为其检查清单。

## 1. 项目许可证与例外

Quant1X 原创代码采用 **MIT License**（见根目录 `LICENSE`）。

以下文件属于第三方代码的衍生作品，**不适用 MIT，按原许可证分发**：

| 文件 | 来源 | 许可证 |
|---|---|---|
| `quant1x/io/csv-reader.h` | [fast-cpp-csv-parser](https://github.com/ben-strasser/fast-cpp-csv-parser) | BSD-3-Clause |
| `quant1x/io/csv-reader.md` | fast-cpp-csv-parser 文档 | BSD-3-Clause |

`quant1x/io/csv-writer.h` 为 Quant1X 原创实现（写入 CSV、字段引号转义），按项目 MIT 许可证分发。

### 1.1 fast-cpp-csv-parser

- Copyright: (2012-2015) Ben Strasser <code@ben-strasser.net>
- License: BSD-3-Clause
- 本仓库中的修改：重命名 include guard、增加 `#pragma once`、代码格式化；解析逻辑未改动。
- 完整许可证文本：`quant1x/io/csv-reader.LICENSE`

```text
Copyright (c) 2015, ben-strasser
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of fast-cpp-csv-parser nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## 2. third_party 内置组件

`third_party/include` 下的头文件按各自许可证分发，许可证原文位于 `third_party/LICENSE.*`：

| 组件 | 许可证文件 |
|---|---|
| argparse | `third_party/LICENSE.argparse` |
| backward-cpp | `third_party/LICENSE.backward-cpp` |
| cpp-httplib | `third_party/LICENSE.cpp-httplib` |
| croncpp | `third_party/LICENSE.croncpp` |
| csv2 | `third_party/LICENSE.csv2` |
| date | `third_party/LICENSE.date` |
| fmt | `third_party/LICENSE.fmt` |
| indicators | `third_party/LICENSE.indicators` |
| magic_enum | `third_party/LICENSE.magic_enum` |
| mio | `third_party/LICENSE.mio` |
| Boost.PFR | `third_party/LICENSE.pfr` |
| protobuf | `third_party/LICENSE.protobuf` |
| robin-map | `third_party/LICENSE.robin-map` |
| spdlog | `third_party/LICENSE.spdlog` |
| BS::thread_pool | `third_party/LICENSE.thread-pool` |
| xsimd | `third_party/LICENSE.xsimd` |
| xtensor | `third_party/LICENSE.xtensor` |
| xtl | `third_party/LICENSE.xtl` |

## 3. Conan 管理的依赖

C++ 构建通过 Conan 2 获取 openssl、libiconv、zlib、minizip、yaml-cpp、mimalloc、cpr、capnproto、benchmark、catch2、gtest 等依赖。这些组件不随本仓库源码分发，其许可证由 Conan 包携带；本仓库只在构建说明与本节中列明依赖及其许可证，供下游构建者参考。

**下游若发布二进制或安装包**，必须：

1. 汇总本次构建实际使用的 Conan 包及其许可证；
2. 将许可证文本随发布物一并提供（例如放入安装包的 `share/licenses/`）；
3. 重点复核 **libiconv（LGPL-2.1-or-later）** 等弱 copyleft 组件在静态链接场景下的义务（提供可重新链接的目标文件/源码，或改用动态链接）；
4. 不在产品名称或宣传中使用第三方项目名称暗示背书（BSD-3 第 3 条）。

### 3.1 libiconv（LGPL-2.1-or-later）

本项目仅通过 Conan 引用 `libiconv/1.19`，**不分发其源码或二进制**。以下内容适用于基于本源码构建并发布二进制的一方。Conan Center 配方未对该版本应用补丁。

| 项目 | 内容 |
|---|---|
| 许可证 | LGPL-2.1-or-later（`COPYING.LIB`） |
| 源码 | https://ftp.gnu.org/gnu/libiconv/libiconv-1.19.tar.gz |
| SHA-256 | `88dd96a8c0464eca144fc791ae60cd31cd8ee78321e67397e25fc095c4a19aa6` |
| 修改 | 无（直接使用上游源码构建） |

LGPL 的义务针对**分发最终二进制**这一行为，与依赖是通过 vcpkg 还是 Conan 获取无关。发布包含 libiconv 的可执行文件时，必须同时提供：

1. 上述 libiconv 源码（建议直接随包附带 tarball，而不是只写 URL）；
2. LGPL-2.1 许可证文本与版权声明；
3. 让用户能够替换 libiconv 并重新链接的机制：
   - **推荐**：将 libiconv 改为动态链接（Conan 选项 `libiconv/*:shared=True`），随包提供 DLL/so 与导入库；
   - 静态链接时：随包提供 libiconv 源码，以及本程序的完整源码或目标文件与可复现的构建说明，使用户可以重新链接。

获取 Conan 配方与源码信息：

```bash
conan cache path libiconv/1.19          # 配方目录（含 conandata.yml）
conan install . -of build --build=missing -o "libiconv/*:shared=True"   # 动态链接方案
```

生成发布包时，建议在 `share/licenses/quant1x/libiconv/` 下放置：`COPYING.LIB`、`README.source`、`libiconv-1.19.tar.gz`，以及本项目的源码标签/提交号。

## 4. 下游二进制分发检查清单

本项目不发布二进制；以下清单供基于本源码构建并对外分发二进制的一方使用：

- [ ] 源码包保留 `LICENSE`、本文件、`quant1x/io/csv-reader.LICENSE`、`third_party/LICENSE.*`
- [ ] 二进制包/安装包包含本文件及上述许可证文本
- [ ] `quant1x/io/csv-reader.h` 保留 upstream 版权声明与 BSD-3 文本
- [ ] 修改过的第三方文件标注修改说明
- [ ] 发布说明中列出新增/升级的第三方组件及其许可证
- [ ] 不包含 SimHei 等中易／微软自带字体（见第 5 节）

## 5. 已移除的受限资产（历史留存说明）

`quant1x/util/SimHei.ttf` 为**中易黑体（SimHei）v3.02**，版权属 ZHONGYI Electronic Co.（北京中易中标电子信息技术有限公司），随 Windows 授权给终端用户在本机使用，**未授予再分发权**。

该文件此前曾存在于本仓库，现已移除：

- 当前工作树不再包含该文件，也不再进入 Python 源码包与轮子；
- 代码侧引用（`FONT_SimHei` 常量、`quant1x/util/font.py`）已一并删除，绘图仅按字体族名使用，中文字体由使用者自行安装；
- 受已发布标签与协作成本所限，**该文件仍存在于历史提交中**（自引入提交 `009bf0f` 起）。克隆完整历史或下载早期版本标签的源码归档时，仍可能获得该文件。

**声明**：任何派生版本、镜像站点或下游发行物均不得重新引入、复制或以其他方式分发该字体文件。项目自本声明起不再主动分发该资产，并在发现重新引入时予以移除。

需要内置中文字体时，请改用可再分发的替代方案：思源黑体 / Noto Sans CJK SC（SIL OFL-1.1）、文泉驿正黑（GPLv2 + 字体例外），并按其许可证附带声明。

