# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]

## [0.6.17] - 2025-09-07
### Changed
- 修复头文件兼容问题

## [0.6.16] - 2025-09-07
### Changed
- 修订msvc的版本号
- update changelog

## [0.6.15] - 2025-09-07
### Changed
- 补全原q2x的测试代码，可能存在兼容问题，先保留
- update changelog

## [0.6.14] - 2025-09-07
### Changed
- Merge branch '0.6.x'
- update changelog

## [0.6.13] - 2025-09-07
### Changed
- 剔除绘图的库
- update changelog

## [0.6.12] - 2025-09-07
### Changed
- 剔除对cista库的依赖
- update changelog

## [0.6.11] - 2025-09-07
### Changed
- 安装头文件排除__pycache__目录
- update changelog

## [0.6.10] - 2025-09-07
### Changed
- 调整测试代码
- update changelog

## [0.6.9] - 2025-09-07
### Changed
- 新增安装应用q2x的配置
- update changelog

## [0.6.8] - 2025-09-06
### Changed
- 修订安装quant1x库的头文件路径
- 修订安装quant1x库的头文件路径
- update changelog

## [0.6.7] - 2025-09-06
### Changed
- 调整静态库后缀
- update changelog

## [0.6.6] - 2025-09-06
### Changed
- 修订setup.py, 删除废弃的rst格式历史信息文档
- update changelog

## [0.6.5] - 2025-09-04
### Changed
- 调整测试代码
- update changelog

## [0.6.4] - 2025-09-04
### Changed
- 实验PyProject.toml配置模式
- 新增机器学习的聚类模块
- 删除废弃的尝试go1.25新特征go-import meta
- 优化k-means++代码
- 调整测试代码
- update changelog

## [0.6.3] - 2025-09-04
### Changed
- 更新c++第三方库版本到1.36.0
- update changelog

## [0.6.2] - 2025-09-04
### Changed
- git仓库忽略cmake构建的临时目录
- cmake变量名前缀, 统一quant1x_
- update changelog

## [0.6.1] - 2025-09-02
### Changed
- 更新c++第三方库xsimd版本到13.2.0(eb3bacbe1012bf79a1ad68f2aec53b3e051bc14d)
- update changelog
- 修订README文件, 开发语言支持python/go/c++。rust视情况而定, 部分实现可以参考q1x-rust。后续q1x为前缀的项目会归档后放弃维护
- 更新c++第三方库版本到1.36.0

## [0.6.0] - 2025-09-02
### Changed
- 新增go模块
- 修订git仓库需要忽略的文件类型或目录
- 新增c++的实现代码
- 新增clangd配置
- 新增c++第三方库argparse
- 新增c++第三方库xtensor
- 新增c++第三方库asio
- 新增c++第三方库backward-cpp
- 新增c++第三方库backward-cpp
- 新增c++第三方库cista
- 新增c++第三方库spdlog
- 新增c++第三方库boost.pfr
- 新增c++第三方库protobuf
- 新增c++第三方库bs::thread-pool
- 新增c++第三方库mio
- 新增c++第三方库csv2
- 新增c++第三方库date,fmt
- 新增c++第三方库indicators
- 新增c++第三方库magic_enum
- 新增c++第三方库croncpp
- 新增c++第三方库nlohmann json
- 新增c++第三方库inja
- 新增c++第三方库robin-map
- 新增二进制协议定义文件
- git仓库忽略.cache目录
- 新增二进制文件转c++代码的python脚本
- 新增c++README.md
- 补全c++实现的其它代码
- 新增go项目模块文件
- update changelog

## [0.5.9] - 2025-08-31
### Changed
- 更新依赖库numpy版本
- update changelog

## [0.5.8] - 2025-08-31
### Changed
- 更新依赖库版本
- update changelog

## [0.5.7] - 2025-08-31
### Changed
- 修改相对路径的问题
- update changelog

## [0.5.6] - 2025-08-31
### Changed
- 调整发布脚本中引用q1x-base为quant1x
- update changelog

## [0.5.5] - 2025-08-31
### Changed
- 新增.gitattributes, 为bat设置单独的回车换行
- update changelog

## [0.5.4] - 2025-08-31
### Changed
- Merge branch '0.5.x' of https://gitee.com/quant1x/quant1x into 0.5.x
- update changelog

## [0.5.3] - 2025-08-31
### Changed
- 发布脚本设置可执行属性
- 删除废弃的代码
- 复制q1x-base代码
- update changelog

## [0.5.2] - 2025-08-21
### Changed
- 优化发布pypi脚本
- update changelog

## [0.5.1] - 2025-08-21
### Changed
- 优化setup.py
- update changelog

## [0.5.0] - 2025-08-21
### Changed
- 调整git忽略条目

## [0.1.13] - 2025-08-21
### Changed
- 从原89k项目迁移到本项目, 旨在研究量化的可能
- update changelog

## [0.1.12] - 2025-08-21
### Changed
- 删除全部的子模块
- update changelog

## [0.1.11] - 2025-08-21
### Changed
- update changelog
- 更新python支持版本到3.12.x, go版本到1.25.x
- Merge branch 'master' of https://gitee.com/quant1x/quant1x
- 更新版本
- update changelog

## [0.1.10] - 2023-09-28
### Changed
- 链接 go版本量化引擎(data)

## [0.1.9] - 2023-09-27
### Changed
- 链接 go版本量化引擎(data)

## [0.1.8] - 2023-09-27
### Changed
- 新增pip换国内源的说明
- 调整README

## [0.1.7] - 2023-09-21
### Changed
- 更新 模块版本

## [0.1.6] - 2023-09-21
### Changed
- 链接 python版本通达信网络数据工具库
- 链接 python版本通达信网络数据工具库
- 链接 python版本通达信网络数据工具库

## [0.1.5] - 2023-09-21
### Changed
- 链接 python版本通达信网络数据工具库

## [0.1.4] - 2023-09-21
### Changed
- 链接 python版本公式指标

## [0.1.3] - 2023-09-21
### Changed
- 调整文档说明中版本号的描述

## [0.1.2] - 2023-09-21
### Changed
- 调整文档说明中版本号的描述

## [0.1.1] - 2023-09-21
### Changed
- 增加README

## [0.1.0] - 2023-09-21
### Changed
- Initial commit
- 链接 量化交易需要的工具库
- 链接 迅投win64工具库
- 链接 go版本的通达信数据网络工具库
- 链接 python win64版本的简易交易客户端


[Unreleased]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.17...HEAD
[0.6.17]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.16...v0.6.17
[0.6.16]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.15...v0.6.16
[0.6.15]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.14...v0.6.15
[0.6.14]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.13...v0.6.14
[0.6.13]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.12...v0.6.13
[0.6.12]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.11...v0.6.12
[0.6.11]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.10...v0.6.11
[0.6.10]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.9...v0.6.10
[0.6.9]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.8...v0.6.9
[0.6.8]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.7...v0.6.8
[0.6.7]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.6...v0.6.7
[0.6.6]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.5...v0.6.6
[0.6.5]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.4...v0.6.5
[0.6.4]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.3...v0.6.4
[0.6.3]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.2...v0.6.3
[0.6.2]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.1...v0.6.2
[0.6.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.6.0...v0.6.1
[0.6.0]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.9...v0.6.0
[0.5.9]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.8...v0.5.9
[0.5.8]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.7...v0.5.8
[0.5.7]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.6...v0.5.7
[0.5.6]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.5...v0.5.6
[0.5.5]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.4...v0.5.5
[0.5.4]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.3...v0.5.4
[0.5.3]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.2...v0.5.3
[0.5.2]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.1...v0.5.2
[0.5.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.5.0...v0.5.1
[0.5.0]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.13...v0.5.0
[0.1.13]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.12...v0.1.13
[0.1.12]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.11...v0.1.12
[0.1.11]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.10...v0.1.11
[0.1.10]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.9...v0.1.10
[0.1.9]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.8...v0.1.9
[0.1.8]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.7...v0.1.8
[0.1.7]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.6...v0.1.7
[0.1.6]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.5...v0.1.6
[0.1.5]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.4...v0.1.5
[0.1.4]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.3...v0.1.4
[0.1.3]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.2...v0.1.3
[0.1.2]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.1...v0.1.2
[0.1.1]: https://gitee.com/quant1x/quant1x.git/compare/v0.1.0...v0.1.1

[0.1.0]: https://gitee.com/quant1x/quant1x.git/releases/tag/v0.1.0
