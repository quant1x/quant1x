# Quant1X 量化交易实验室

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Go](https://img.shields.io/badge/Go-1.25-blue.svg)](https://golang.org/)
[![Rust](https://img.shields.io/badge/Rust-1.90+-orange.svg)](https://www.rust-lang.org/)
[![Python](https://img.shields.io/badge/Python-3.12+-yellow.svg)](https://www.python.org/)
[![CMake](https://img.shields.io/badge/CMake-3.30+-green.svg)](https://cmake.org/)
![tag](https://img.shields.io/github/tag/quant1x/quant1x.svg?style=flat)
![Crates.io](https://img.shields.io/crates/d/quant1x.svg)

Quant1X 是一个多语言量化交易框架，支持 C++、Go、Rust、Python，提供完整的量化交易解决方案，包括数据处理、策略开发、回测引擎、实时交易等功能。

## ✨ 特性

- **多语言支持**：C++20、Go 1.25、Rust 1.90+、Python 3.12+
- **高性能**：优化的算法和数据结构，支持 SIMD 指令集
- **模块化设计**：可插拔的组件架构，易于扩展
- **实时数据**：支持 Level1 市场数据
- **策略框架**：内置多种技术指标和策略模板
- **回测引擎**：高效的回测系统，支持多资产组合
- **网络通信**：内置 HTTP/WebSocket 客户端，支持多种协议

## 📋 环境要求

### 系统要求

- 64位操作系统 (Windows/Linux/macOS)
- 至少 8GB RAM，推荐 16GB+
- 支持 AVX2 指令集的 CPU

### 语言版本

| 语言 | 版本要求 | 推荐版本 |
|------|----------|----------|
| Python | 3.12+ | 3.12.x |
| Go | 1.25+ | 1.25.x |
| Rust | 1.88+ | 1.90+ (2021) |
| C++ | C++20 | GCC 14.3+/Clang 18+/MSVC 14.3+ |

## 🚀 快速开始


# 1. 环境配置
## 1.1 默认均为64位操作系统
| python | golang | rust       | c++                         |
|:-------|:-------|:-----------|:----------------------------|
| 3.12.x | 1.25.x | 1.88+/2024 | gcc14.3+/clang18+/msvc14.3+ |

## 1.2 环境安装推荐使用brew
安装brew时需要注意避免使用root权限 


# 2. python

python的运行环境可能存在多个版本冲突的问题，那么怎么来解决多版本的共存的问题呢？使用pyenv。

## 2.1 安装pyenv
```shell
brew install pyenv
```
### 2.1.1 查看已安装的版本
```shell
pyenv versions
```
我的电脑返回以下版本信息
```text
  system
  3.8.16
* 3.10.10 (set by /Users/${USERNAME}/.pyenv/version)
```
### 2.1.2 查看可安装的版本
```shell
pyenv install -l
```
### 2.1.3 安装指定版本的python, 本文指定3.12.9或3.12.x更新版本
```shell
pyenv install 3.12.9
```

### 2.1.4 pip类库管理工具
安装python完成之后, python类库管理工具pip已经默认安装完成了

### 2.1.5 python基础工具

| 工具  | 功能                           |
|:----|:-----------------------------|
| pip | 类似maven、gradle、go mod的类库管理工具 |
| pip-autoremove| 自动删除类库所有依赖库                  |
|pipreqs| 项目/类库交叉依赖检测                  |

### 2.1.6 pip 源配置
windows
```shell
cd ~\AppData\Roaming\pip
notepad.exe pip.ini
```
*nix
```shell
cd ~/.pip
vim pip.conf
```
输入以下内容
```text
[global]
index-url = https://pypi.tuna.tsinghua.edu.cn/simple
[install]
trusted-host = https://pypi.tuna.tsinghua.edu.cn
```

## 2.2 依赖库
python环境中依赖管理的配置文件为requirements.txt，类似java的maven pom.xml、golang的go.mod。

### 2.2.1 安装项目依赖的库
```shell
pip install -r requirements.txt
```

### 2.2.2 检测项目依赖输出到requirements.txt
```shell
pip freeze > requirements.txt
```

### 2.2.3 交叉依赖

```shell
pip install pipreqs
pipreqs ./ --encoding utf8
```

## 2.3 上传package到PyPi

### 2.3.1 安装或更新setuptools、wheel、twine
```shell
pip install --upgrade setuptools wheel twine
```

### 2.3.2 打包并生成tar.gz和whl文件
```shell
python setup.py sdist bdist_wheel
```

### 2.3.3 上传package到PyPi的测试环境
```shell
twine upload --repository testpypi dist/*
```

### 2.3.4 上传package到PyPi的正式环境
```shell
twine upload dist/*
```

## 2.4 Matplotlib中文乱码问题解决方案

### 2.4.1 编写如下代码，获取matplotlib包所在的配置文件的路径
```python
import matplotlib
matplotlib.matplotlib_fname() #输出matplotlib包所在的配置文件的路径
```

### 2.4.2 根据上面的路径打开文件夹（根据自己实际的输出路径去操作）
我选择了SimHei中文字体, 复制到fonts/ttf/目录下
```shell
cp -r /Users/${USERNAME}/Library/Fonts/SimHei.ttf fonts/ttf/
```

### 2.4.3 编辑2.4.1获得路径matplotlibrc文件
#### 2.4.3.1 找到 #font.sans-serif，去掉前面的#，并在：后面写上在准备工作加入的中文字体的名称SimHei
#### 2.4.3.2 找到#axes.unicode_minus，去掉前面的#，并在：改为False
### 2.4.4 控制台切换到~/.matplotlib目录, 删除tex.cache文件和fontList.json文件
```shell
cd ~/.matplotlib
rm -rf *
```


# 3. golang 开发环境

环境设定
```shell
go env -w GO111MODULE=on
go env -w GOPROXY=https://goproxy.cn,direct
go env -w GOPRIVATE=gitee.com
```

# 4. Rust 开发环境

```bash
# 安装 Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# 更新到最新版本
rustup update
```
# 5. c/c++ 开发环境

## 🤝 贡献

欢迎贡献代码！请遵循以下步骤：

1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 开发规范

- 遵循各语言的编码规范
- 添加单元测试
- 更新文档
- 确保所有测试通过

## 📄 许可证

本项目采用 Apache 2.0 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 📚 相关链接

- [文档](docs/)
- [示例](examples/)
- [问题跟踪](https://gitee.com/quant1x/quant1x/issues)
- [讨论区](https://gitee.com/quant1x/quant1x/discussions)

---

**注意**: 本项目仅用于学习和研究目的，不构成投资建议。在实际交易前，请充分了解风险并咨询专业人士。
