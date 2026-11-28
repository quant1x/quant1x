# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]

## [0.9.0] - 2026-11-28
### Changed
- fix[cpp]：修复 csv2 引号字段解析并补 Conan 回归用例
  - 新增 third_party/recipes/csv2 本地配方：csv2 0.2 打包时把 read_value 改为先剥离字段最外层引号，再折叠内部 ""
  - 补丁在 build 目录的副本上定点替换，幂等且不改 source 缓存；结构变化时抛 ConanException 提示重新核对
  - 新增 test_package：覆盖含逗号、双引号、单引号、换行与空字段的解析
  - 验证：conan create third_party/recipes/csv2（test_package 输出 csv2 quote test passed）
  - 工单 #35
- feat[python]：新增 num 数值规格层
  - 新增 quant1x/num：dtype/NaN、窗口归一化、逐元素、逻辑、归约、滚动、EWM、移位/差分、填充、对齐、回归、小数位与矩阵构造
  - 口径写入 num.md：完整窗口才计算、NaN 传播、顺序累加、总体方差、均值中心化回归、半程远离零
  - 新增 tests/test_num.py 覆盖窗口不足、窗口含 NaN、空输入、回归退化与舍入
  - 验证：python tests/test_num.py（16 passed）
  - 工单 #35
- feat[python]：新增 pandas 列容器门面
  - 新增 Series / DataFrame / Rolling / EWM：按位置对齐，数值算子全部转调 quant1x.num
  - README.md 与 pandas.md 记录容器边界、跨语言对照与 TODO
  - 根 README 项目结构补充 num/ 与 pandas/
  - 新增 tests/test_pandas_facade.py 覆盖运算符、位置对齐、rolling/ewm 转发与列读写
  - 验证：python tests/test_pandas_facade.py（7 passed）
  - 工单 #35
- refactor[python]：formula 收敛到可移植口径并落地对拍样本
  - 数值算子改走 quant1x.num：滚动完整窗口、EWM 跳 NaN、顺序归约
  - SLOPE/FORCAST 改均值中心化最小二乘闭式解，不再依赖 numpy.polyfit 的 LAPACK 实现
  - RD 改为半程远离零；FILTER 返回 0/1 序列，修复 bool 入参在 NumPy 2.x 下抛 TypeError
  - 新增 tests/fixtures/formula：57 组公式样本与 120 根 K 线的五个指标样本，由 Python spec 生成
  - tests/test_formula_parity.py 断言重新生成与签入文件逐字节一致
  - tests/fixtures 下的 csv 加入 .gitignore 白名单
  - 验证：python tests/test_formula_parity.py（2 passed）
  - 工单 #35
- chore[cpp]：C/C++ 依赖全部迁移到 Conan 并支持 Meson
  - 删除 third_party 内置依赖（含 protobuf 静态库），全部改为 conanfile.py 声明
  - cmake 模块改用 imported target：spdlog 取 header-only，xsimd/xtensor、asio、protobuf、backward 等重写
  - 进度条改用组织内 mpb（QUANT1X_MPB_DIR，默认 ../mpb），cache/tracker 的进度条改写
  - 新增 meson.build：Conan MesonToolchain + PkgConfigDeps，构建库与 q1x
  - 补齐去内嵌头文件后暴露的标准库包含（map/optional/mutex/algorithm）
  - csv2 引号回归用例随构建迁移接入
  - 验证：conan install；cmake 配置与 quant1x 构建；meson compile
  - 工单 #35
- feat[cpp]：实现 formula 与 indicators 的 C++ 版本并对拍
  - 新增 quant1x/formula/formula.h：xtensor 容器 + 全量 0/1/2 级公式函数，Series/Bools 与 NumPy 对标
  - 新增 quant1x/indicators 的 cdtd/f89k/hjk/ma1x/p2fb C++ 实现与 support.h，逐行对照 Python
  - 新增 tests/tdd-formula-parity.cpp 与 tests/tdd-indicators.cpp，读同一份 fixture 逐位比较
  - formula.md 与 indicators/README.md 记录统一口径、跨语言差异与已知坑（== 整表比较、&& / ||、ADL 限定）
  - CMake 与 Meson 登记两个对拍目标
  - 验证：cmake 构建后 formula 57 组、indicators 五个指标逐位通过；meson test 2/2
  - 工单 #35
- feat[python]：补齐 num 序列-标量算子与数值转布尔
  - elementwise 新增 add_scalar / sub_scalar / mul_scalar / div_scalar /
    maximum_scalar / minimum_scalar，与二元算子的 NaN 口径一致
  - logic 新增 to_bools：非零且非 NaN 为真
  - dtype.to_scalar 空序列返回 NAN（此前会 IndexError），与 Rust / C++ 端口一致
  - tests/test_num.py 补 4 个用例：标量四则、极值 NaN、数值转布尔、空序列收敛
  - 验证：python tests/test_num.py → 20 passed

  这三个 API 此前只有 Rust / C++ 端口实现，Python spec 缺位；按「Python 是 spec」
  先补齐再推广。工单 #35。
- feat[rust]：落地 num 数值规格层的 Rust 对齐实现
  - 新增 quant1x/num/ 下 13 个模块：dtype / window / elementwise / logic /
    reduce / rolling / ewm / shift / fillna / align / regression / decimals / matrix
  - 容器用 Vec<f64> / Vec<bool>，窗口用 Window::{Scalar, Series}，全程显式循环
  - 与 Python spec 同口径：完整窗口、NaN 传播、顺序累加、总体方差、
    均值中心化回归、半程远离零；关键字规避 ref→ref_value、where→where_select
  - lib.rs 注册 pub mod num
  - 验证：cargo check --lib 通过

  本层是 quant1x/formula 与 indicators 的数值底座；C++ 对齐实现在下一提交。工单 #35。
- feat[cpp]：落地 num 数值规格层的 C++ 实现与逐位对拍
  - 新增 quant1x/num/ 下 13 个模块头与 num.h 汇总头，命名空间 quant1x::num
  - 容器 std::vector<double> / std::vector<bool>，全程显式循环，不依赖 xtensor
    的广播与比较运算符，与 Python spec 逐位一致
  - 命名：reduce.std → stddev（std 会遮蔽 std:: 命名空间）；另提供
    where_select / ref_value 别名与 Rust 对齐
  - 新增对拍：tests/fixtures/num/generate.py 由 Python spec 生成 163 组样本，
    tests/tdd-num-parity.cpp 逐位比较（NaN 视为相等）
  - CMake / Meson 登记 app-tdd-num-parity；num 头文件登记入 quant1x-headers
  - num.md / README.md 更新为 python→c++→rust→go→zig 五列对照
  - 验证：app-tdd-num-parity 163/163；app-tdd-formula-parity 57 组；
    app-tdd-indicators；CMake ctest 3/3、Meson test 3/3；fixture 二次生成 md5 一致

  工单 #35。
- feat[rust]：落地 formula 与 indicators 的 Rust 对齐实现并对拍
  - 新增 quant1x/formula/mod.rs：0/1/2 级全部公式函数，与 C++ formula.h 逐函数对应
    - 容器 Vec<f64> / Vec<bool>，全程显式循环；复用 quant1x::num 的窗口与基础算子
    - 命名：ref→ref_value、const→const_series、if→if_select、std→stddev（Rust 关键字/标准库冲突）
    - ZTPRICE 沿用十进制字符串相乘 + 按分四舍五入，与 C++ 一致
  - 新增 quant1x/indicators/mod.rs：cdtd / f89k / hjk / ma1x / p2fb，输入 &[Bar]
  - lib.rs 注册 pub mod formula / indicators
  - 新增对拍：tests/formula_parity.rs（读 cases.csv，57 组）、
    tests/indicators_parity.rs（读 indicators/*.csv 与 bars.csv，5 指标）
  - formula.md / indicators/README.md 更新 rust 列与状态
  - 验证：cargo build 无警告；cargo test --test formula_parity 57 组通过；
    cargo test --test indicators_parity 5 指标通过

  Go 复用组织级 quant1x/num，Zig 为扩展，均未在本轮落地。工单 #35。
- feat[go]：落地 num 的 Go 实现并移除外部 github.com/quant1x/num 依赖
  - 新增 quant1x/num/*.go：与 Python spec 同语义的 Go 对齐实现
    - 容器 []float64 / []bool，全程显式循环；覆盖 dtype / window / elementwise /
      logic / reduce / rolling / ewm / shift / fillna / align / regression / decimals / matrix
    - mulOnly 用 //go:noinline 阻断 FMA 收缩，保证 GOAMD64>=v3 下归约与递推逐位稳定
    - builtin 保留 SetAvx2Enabled / GetAvx2Enabled（本包不做向量化，只存开关）
  - 移除 go.mod / go.sum 对 github.com/quant1x/num 的依赖；command 与 learn/cluster
    改用内部 github.com/quant1x/quant1x/quant1x/num
  - 新增 quant1x/num/num_parity_test.go：读 cases.csv，逐值对拍；Go 的 math.Log /
    math.Pow 与 NumPy 至多 1 ULP 差异，这两类按相对误差 1e-12 比较，其余逐位相等
  - num.md / README.md 更新 Go 列与状态；注明已不再依赖外部 num

  说明：本仓库从未依赖 github.com/quant1x/pandas（go.mod 无此条目，Go 侧无引用）；
  如后续新增 Go pandas 能力将放在内部 quant1x/pandas 包，不引入外部模块。

  验证：go vet ./quant1x/num/ 通过；go test ./quant1x/num/ 通过；
  go build ./quant1x/command/ ./quant1x/learn/cluster/ 通过；go mod tidy 已移除外部 num。工单 #35。
- feat[go]：落地 formula 与 indicators 的 Go 对齐实现并对拍
  - 新增 quant1x/formula/formula_go.go、rolling_go.go：0/1/2 级全部公式函数，
    与 C++ formula.h、Rust formula/mod.rs 逐函数对应
    - 容器 []float64 / []bool，复用内部 quant1x/num 的滚动与归约
    - Go 导出名 CamelCase，与其它语言的 snake_case 一一对应（Ref / Sum / IfSelect / Std）
    - Wma / DmaSeries 用 noinline mulOnly 阻断 FMA 收缩
    - Ztprice 沿用十进制字符串相乘 + 按分四舍五入
  - 新增 quant1x/indicators/indicators_go.go：Cdtd / F89k / Hjk / Ma1x / P2fb，输入 []schema.Bar
  - 新增对拍：quant1x/formula/formula_parity_test.go（57 组）、
    quant1x/indicators/indicators_parity_test.go（5 指标读 bars.csv 与指标列）
  - formula.md / indicators/README.md 更新 Go 列与状态为「已对齐 c++/rust/go（缺 zig）」
  - 验证：go vet 通过；go test ./quant1x/num/ ./quant1x/formula/ ./quant1x/indicators/ 全部通过；
    go build ./quant1x/... 通过

  Go 的 math.Log / math.Pow 与 NumPy 至多 1 ULP 差异，对拍按相对误差 1e-12 比较，其余逐位相等。工单 #35。
- feat[zig]：落地 num 数值规格层的 Zig 扩展实现
  - 新增 quant1x/num/num.zig：与 Python spec 同语义的 Zig 对齐实现（扩展角色）
    - 容器 []f64 / []bool，调用方传 allocator；覆盖 dtype / elementwise / logic /
      reduce / rolling / ewm / shift / diff / fillna / padTo / regression / decimals /
      zeros / ones / linspace
    - mulOnly 阻断 FMA 收缩，保证递推与归约末位稳定
    - align 是 Zig 关键字，改用 padTo（对应 Python 的 align / pad_to）
  - 新增 quant1x/num/num_test.zig：10 个用例覆盖窗口不足、含 NaN、空输入、ewm 种子、
    小数位半程远离零、移位越界、条件选择、回归、对齐与等分
  - num.md / README.md 更新 Zig 列与状态为「扩展，核心算子已对齐，矩阵构造与极值位置待补」
  - AGENTS.md 增加 zig test 命令

  说明：Zig 为扩展角色（当前只承载小功能），matrix 的 shape/concat1d/transpose2d/arange
  与 rolling 的 hhvbars/llvbars 尚未补，已在 num.md §1 标注。

  验证：zig test quant1x/num/num_test.zig → 10/10 passed。工单 #35。
- docs[multi]：注明 num 的 align/pad_to/repeat 为跨语言参考算法
  - align.py 模块文档改为「跨语言对齐的参考算法」：说明 NumPy 已提供等价能力
    （repeat ≈ np.resize，align ≈ 切片 + np.pad），本包再写一遍是为给 C++/Rust/Go/Zig
    一个逐位可复刻的口径；明确「本项目 Python 代码本身不调用」「应用层请勿调用，
    列对齐用 quant1x.pandas 的 Series.align」
  - num.md 在功能表后补同名说明段落；README.md 的文件表标注为参考算法
  - elementwise.py 的引导注释改为「对齐口径参考 num.align，应用层不要调用」
  - C++ align.h / Rust align.rs / Go reduce.go / Zig num.zig 各加一行同样的说明

  不改任何实现与行为。验证：python tests/test_num.py 20 passed；go test ./quant1x/num/
  通过；zig test quant1x/num/num_test.zig 10/10。工单 #35。
- feat[rust]：落地 pandas Series 的可移植实现（含 Rolling / EWM）
  - 新增 quant1x/pandas/series.rs：按 quant1x/pandas/series.py 的契约用 Vec<f64> / Vec<bool>
    复刻同一语义，数值算子全部委托 crate::num
    - 覆盖: 构造、name/dtype/len、get/set、slice、copy、to_f64/to_bool、shift/diff/cumsum/fill_na、
      apply、四则（add/sub/mul/div 与标量版）、比较（gt/gte/lt/lte/equal/not_equal）、
      逻辑（and/or/not）、归约（sum/mean/median/min/max/argmin/argmax/var/std）、rolling、ewm
    - Go 无运算符重载，故契约以命名方法为准；Python 的运算符一一映射到这些方法
  - pandas/mod.rs 仅新增 series 子模块声明与再导出（保留原 parse_frequency）
  - 新增 tests/series_test.rs：6 个用例覆盖移位/差分/累计、四则与比较、逻辑、归约、
    滚动与 EWM、fill_na 与切片
  - pandas.md 的 rust 列更新为 series.rs

  说明：C++ 侧 series.h 仍是 ta 命名空间下的原型（仅 shift/align/==），与 Python 契约有差距，
  列为后续；Go / Zig 的 Series 亦待补。

  验证：cargo build 无警告；cargo test --test series_test 6/6 通过；python tests/test_pandas_facade.py
  7 passed。工单 #37。
- feat[python]：新增 num.ndarray 容器（对标 np.ndarray）
  - 新增 quant1x/num/ndarray.py：一维 NDArray 容器，numpy 承载存储与互转，语义全部委托
    quant1x.num 的自由函数
    - 构造 zeros/ones/full/arange/linspace；属性 size/shape/ndim/dtype/values
    - 序列协议：len / 迭代 / getitem（int 或 slice）/ setitem / slice / copy / astype
    - 四则 add/sub/mul/div（数组或标量）、maximum/minimum、abs/sqrt/log/pow
    - 比较 gt/gte/lt/lte/equal/not_equal（返回 bool NDArray）、逻辑 logic_and/or/not
    - 归约 sum/mean/median/min/max/argmin/argmax/var/std/cumsum
    - shift/diff/fill_na、rolling_*、ewm_mean
    - Python 额外提供运算符糖（+ - * / > >= < <= == != & | ~），一一映射到同名方法，
      方便 Go 等无运算符重载的语言照抄同一张方法表
  - 新增 quant1x/num/ndarray.md：跨语言契约（方法表 / 语义边界 / 各语言状态）
  - __init__.py 导出 NDArray；num.md、README.md 登记容器行
  - 新增 tests/test_ndarray.py：14 个用例覆盖构造、四则、比较逻辑、归约、移位差分、
    滚动、EWM、切片填充与类型转换

  说明：本轮只落 Python spec；C++ / Rust / Go / Zig 的 ndarray 待补（按生产顺位 c++/rust 优先）。
  验证：python tests/test_ndarray.py 14 passed；test_num 20 passed；test_pandas_facade 7 passed；
  test_formula_parity 通过。工单 #37。
- feat[rust]：落地 num.ndarray 容器（对标 numpy.ndarray）
  - 新增 quant1x/num/ndarray.rs：NdArray 容器，Vec<f64> / Vec<bool> 复刻 ndarray.py 的方法表，
    算子全部委托 crate::num
    - 构造 new/from_bools/zeros/ones/full/arange/linspace
    - dtype/len/to_f64/to_bool/get/set/slice/copy/astype
    - 四则 add/sub/mul/div（数组与标量）、maximum/minimum、abs/sqrt/log/pow
    - 比较 gt/gte/lt/lte/equal/not_equal（含标量版）、逻辑 logic_and/or/not
    - 归约 sum/mean/median/min/max/argmin/argmax/var/std/cumsum
    - shift/diff/fill_na、rolling_*、ewm_mean
  - num/mod.rs 注册 pub mod ndarray 与再导出
  - 新增 tests/ndarray_test.rs：6 个用例
  - ndarray.md 的 Rust 状态更新为已落地

  说明：C++ / Go / Zig 的 ndarray 待补（生产顺位 c++ 随后）。
  验证：cargo build 无警告；cargo test --test ndarray_test 6/6。工单 #37。
- feat[cpp]：落地 num.ndarray 容器（对标 numpy.ndarray）
  - 新增 quant1x/num/ndarray.h：NdArray 容器（namespace quant1x::num），
    std::vector<double> / std::vector<bool> 复刻 ndarray.py / ndarray.rs 的方法表，
    算子全部委托 quant1x::num 的自由函数
    - 构造 zeros/ones/full/arange/linspace；属性 is_bool/dtype/size/ndim/shape
    - to_vector/to_bools/get/set/slice/copy/astype
    - 四则 add/sub/mul/div（数组与标量）、maximum/minimum、abs/sqrt/log/pow
    - 比较 gt/gte/lt/lte/equal/not_equal（含标量版）、逻辑 logic_and/or/not
    - 归约 sum/mean/median/min_value/max_value/argmin/argmax/variance/stddev/cumsum
    - shift/diff/fill_na、rolling_*、rolling_apply、ewm_mean
    - 命名差异: Python 的 std 方法在 C++ 命名为 stddev（避免遮蔽 std:: 命名空间）
  - num.h 汇总头纳入 ndarray.h；CMake quant1x-headers 登记
  - 新增 tests/tdd-num-ndarray.cpp（6 用例 45 断言），tests/CMakeLists.txt 以 catch2 登记

  说明: Go / Zig 的 ndarray 待补。
  验证: cmake 构建 catch2-tdd-num-ndarray 无警告；运行 45 assertions in 6 test cases 全过。工单 #37。
- refactor[cpp]：ta::Series 迁移到 quant1x::pandas::Series
  - 重写 quant1x/pandas/series.h：删除 ta 命名空间下的模板原型，新增 spec 对齐的
    quant1x::pandas::Series（以 quant1x::num::NdArray 为底座，float64 / bool）
    - name/dtype/size/empty、values/to_bools、get/set、copy/slice、apply
    - shift/diff/cumsum/fill_na
    - 四则（add/sub/mul/div 与标量）、比较（gt/gte/lt/lte/equal/not_equal、标量版）、
      逻辑（and/or/not）
    - 归约 sum/mean/median/min_value/max_value/argmin/argmax/variance/stddev
    - rolling(window).sum/mean/stddev/min_value/max_value/apply、ewm(alpha)/ewm_span
    - 运算符糖（+ - * / > >= < <= == != & | ~）映射到同名方法
  - 重写 quant1x/pandas/ewm.h：ExponentialMovingWindow 改为非模板、基于新 Series；
    保留 com/span/half_life/alpha 与 adjust / ignore_na，新增便捷 free 函数 ewm(Series, 参数)
  - formula.h 不再靠 series.h 间接引入 xtensor，改为自包含包含 quant1x/base/simd.h
  - 迁移 tests/tdd-series.cpp 与 tests/tdd-dataframe-ewm.cpp 到新 Series
  - pandas.md / README.md 更新 C++ 状态：Series/EWM 已收敛到 quant1x::pandas，DataFrame 仍原型
  - 全仓已无 ta::Series 引用

  验证：catch2-tdd-series 10 assertions / 16 cases 通过；catch2-tdd-dataframe-ewm 通过；
  catch2-tdd-formula、catch2-tdd-history 编译通过。工单 #37。
- feat[zig]：落地 num.ndarray 容器（对标 numpy.ndarray）
  - 新增 quant1x/num/ndarray.zig：NdArray 容器（union 存 []f64 / []bool），
    方法表与 ndarray.py / ndarray.h / ndarray.rs 一致，算子委托 num.zig
    - 构造 initFloat/initBool/zeros/ones/full/arange/linspace；deinit 释放
    - dtype/len/isEmpty/toF64/toBools/get/set/copy/slice/astype
    - 四则（数组与标量）、maximum/minimum、abs/sqrt/log/pow
    - 比较 gt/gte/lt/lte/equal/notEqual（含标量版）、逻辑 logicAnd/logicOr/logicNot
    - 归约 sum/mean/median/minValue/maxValue/argmin/argmax/variance/stddev/cumsum
    - shift/diff/fillNa、rollingSum/Mean/Std/Min/Max/Avedev、ewmMean
  - num.zig 补 arange（等差序列）
  - 新增 quant1x/num/ndarray_test.zig：5 个用例
  - ndarray.md 的 Zig 状态更新为已落地；AGENTS.md 增加 zig test 命令

  说明：Go 的 ndarray 与 Go / Zig 的 Series 仍待补。
  验证：zig test quant1x/num/ndarray_test.zig 5/5；zig test quant1x/num/num_test.zig 10/10。工单 #37。
- feat[zig]：落地 pandas Series 的可移植实现（含 Rolling / EWM）
  - 新增 quant1x/pandas/series.zig：以 quant1x/num/ndarray.zig 的 NdArray 为底座，
    方法表对齐 series.py / series.h / series.rs；Zig 无运算符重载，契约以命名方法为准
    - name/dtype/len/isEmpty/isBool、values/toBools、get/set、copy/slice、apply
    - shift/diff/cumsum/fillNa
    - 四则（add/sub/mul/div 与标量）、比较（gt/gte/lt/lte/equal/notEqual、标量版）、
      逻辑（logicAnd/logicOr/logicNot）
    - 归约 sum/mean/median/minValue/maxValue/argmin/argmax/variance/stddev
    - rolling(window).sum/mean/stddev/minValue/maxValue、ewm(alpha)/ewmSpan
  - 新增 quant1x/pandas/series_test.zig：6 个用例
  - series.zig 通过模块名 num_ndarray 引用 ndarray（Zig 跨目录 import 受限）；
    AGENTS.md 记录命令：
    zig test --dep num_ndarray -Mroot=quant1x/pandas/series_test.zig
             -Mnum_ndarray=quant1x/num/ndarray.zig
  - pandas.md / README.md 更新 Zig 列与状态为「已对齐 python/c++/rust/zig（go 待补）」

  说明：Go 的 ndarray 与 Series 仍待补。
  验证：上述 zig test 命令 6/6 通过；zig test quant1x/num/ndarray_test.zig 5/5；
  zig test quant1x/num/num_test.zig 10/10。工单 #37。
- feat[go]：落地 num.ndarray 与 pandas Series 容器
  - 新增 quant1x/num/ndarray.go：NdArray 容器（floats / bools 两套切片），
    方法表与 ndarray.py / ndarray.h / ndarray.rs / ndarray.zig 一致，算子委托内部 num
    - 构造 NewNdArray/NewNdArrayBools 与 NdArrayZeros/Ones/Full/Arange/Linspace
    - Dtype/Len/IsEmpty/IsBool/ToF64/ToBools/GetF64/GetBool/SetF64/SetBool/Copy/Slice/Astype
    - 四则（数组与标量）、Maximum/Minimum、Abs/Sqrt/Log/Pow/Neg
    - 比较 Gt/Gte/Lt/Lte/Equal/NotEqual（含标量版）、逻辑 LogicAnd/LogicOr/LogicNot
    - 归约 Sum/Mean/Median/MinValue/MaxValue/Argmin/Argmax/Variance/Stddev/Cumsum
    - Shift/Diff/FillNa、RollingSum/Mean/Std/Min/Max/Avedev/Apply、EwmMean
  - 新增 quant1x/pandas/series.go：Series 基于 num.NdArray，列名随切片与运算传递，
    方法表与 series.py / series.h / series.rs / series.zig 一致；Go 无运算符重载，
    契约一律以命名方法为准（Add/Sub/…、Gt/Gte/…、And/Or/Not）
  - 新增 quant1x/num/ndarray_test.go（5 个用例）与 quant1x/pandas/series_test.go（6 个用例）
  - ndarray.md / pandas.md / pandas README 的 Go 列更新为已落地；
    pandas.md 路线图去掉「Rust / Go / Zig 未落地」的旧结论
  - 语义一致性：数值口径全部走内部 num（窗口不足、NaN 传播、移位越界、EWM 种子均无本地算法），
    与 python spec 及其它语言实现逐位一致
  - 验证：go build ./quant1x/num/ ./quant1x/pandas/、go vet、go test ./quant1x/num/ ./quant1x/pandas/、
    gofmt -l 无输出

  工单 #37
- feat[rust+go+zig]：落地 pandas.DataFrame 可移植容器
  - 新增 quant1x/pandas/dataframe.md：DataFrame 跨语言契约（列名 → Series 的有序表，
    只做按列名存取；列序按插入序、列必须等长、不做隐式补位）
  - 新增 quant1x/pandas/dataframe.rs：Rust 实现（from_columns / columns / shape / len /
    get / select / set / copy，Display 输出形状与列名）
  - 新增 quant1x/pandas/dataframe.go + dataframe_test.go：Go 实现，Column 结构承载
    「列名 + 数据」，DataFrameFromColumns 保序构造；df[name] 的三种写法对应
    Get / Set / Select
  - 新增 quant1x/pandas/dataframe.zig + dataframe_test.zig：Zig 实现，ArrayList 持有列，
    deinit 释放；格式化走 Zig 0.17 的 `{f}` 说明符
  - 新增 tests/dataframe_test.rs：7 个用例（列序与形状、取列与子集、原位替换、追加、
    列长校验、深拷贝、文本表示），Go / Zig 各对齐同一批用例
  - pandas.md / README 的 DataFrame 行更新为已对齐 python/rust/go/zig；AGENTS.md 补
    Zig dataframe 测试命令
  - 语义一致性：三个实现与 dataframe.py 同批口径；Python 的 pandas 隐式补 NaN 未沿用，
    统一改为拒绝长度不符，避免各语言分叉
  - 验证：cargo test --test dataframe_test（7 passed）、go test ./quant1x/pandas/（13 passed）、
    zig test dataframe_test.zig（7/7）
  - 未迁移：C++ 的 dataframe.h 仍是异构列原型（含字符串列与 Boost.PFR 反射构造），
    在 dataframe.md §4 记录差异与迁移方向

  工单 #37
- refactor[cpp]：DataFrame 迁移到 quant1x::pandas 并保留 pandas 风格打印
  - 重写 quant1x/pandas/dataframe.h：列改为 Series（float64 / bool），
    取消旧的 std::variant 异构列存储、全局作用域类与 ta 命名空间依赖
    - 接口：row_count / column_count / shape / columns / contains / get / select /
      set / copy，与 dataframe.py 及 rust / go / zig 同一张方法表
    - from_struct_vector 保留：按字段类型分流，算数字段（含 int / float）→ Series(double)、
      bool → Series(bool)、std::string → 字符串展示列、其它类型跳过
    - 字符串列（set_strings / get_strings）只用于展示与 IO，对照 pandas 的 object dtype，
      不参与数值算子
  - to_string 对齐 pandas 的表格版式
    - 索引列 + 全右对齐（pandas 默认 justify='right'）、列间两个空格
    - 浮点按列统一小数位（至少一位，如 10.0 / 10.50）；量级超出 [1e-4, 1e16) 走科学计数法；
      NaN / inf 用 pandas 写法
    - 超过 60 行显示首尾各 5 行，中间索引列 `..`、数据列 `...` 省略，末尾附
      `[N rows x M columns]`；未截断无页脚；空表输出 Empty DataFrame / Columns / Index
  - 调用点迁移：quant1x/factors/history.cpp 改为 df.get("open")->values()；
    user/strategy-no0.cpp 补 quant1x::pandas 限定
  - 测试：tests/tdd-dataframe-release.cpp 去掉内嵌的 DataFrame 副本，改为针对真实头文件的
    6 个用例（结构体字段分流、打印列序、70 行截断、列操作、空表、真实 Bar 数据）；
    tests/tdd-bars.cpp 恢复此前因类被删而 #if 0 的两条 DataFrame 用例，本地无数据时 SKIP
  - 文档：dataframe.md 增 §4 字符串列、§5 打印口径，状态表 C++ 改为已落地；
    pandas.md / README 的 DataFrame 行同步
  - 语义一致性：数值列与 python / rust / go / zig 完全同口径（列长不符一律报错）；
    C++ 多出的字符串列已单独标注为展示扩展
  - 验证：cmake --build build-msvc --config Release --target quant1x catch2-tdd-dataframe-release
    catch2-tdd-bars catch2-tdd-dataframe-data；ctest -R "tdd-dataframe|tdd-bars|tdd-series"
    （tdd-dataframe-data 的 Case-insensitive CSV loading 依赖本地缓存，缺 sh603338.csv 时失败，
    该文件本轮未改动）

  工单 #37
- chore[cpp]：新增 .clangd 声明 C++20 并接入 Meson 编译数据库
  - 现象：IDE 的 clangd 静态检查把大量 `namespace quant1x::pandas` 误报为
    “nested namespace definition is a C++17 extension”，一次 512 处
  - 根因：仓库根目录没有 compile_commands.json，clangd 按默认参数解析，
    `clangd --check` 的 cc1 参数是 `-std=c++14`；项目基线是 C++20
    （CMake CMAKE_CXX_STANDARD=20、Meson cpp_std=c++20）
  - 新增 .clangd：CompilationDatabase 指向 build-meson/build（Meson 产出，
    含全部 include / define），再用 Add: [-std=c++20] 兜底数据库缺失的情况
  - BUILDING.md 新增 §2.12「IDE 与静态检查（clangd / clang-tidy）」：说明数据库来源、
    改用 CMake 时的做法（Ninja + -DCMAKE_EXPORT_COMPILE_COMMANDS=ON）、
    clang-tidy -p build-meson/build 的用法，并注明不要为迁就工具拆开嵌套命名空间
  - 不改动任何 C++ 代码：全部是工具配置问题
  - 验证：clangd --check=quant1x/pandas/dataframe.h 与 quant1x/num/ndarray.h
    均加载 compile_commands.json、带 -std=c++20，`C++17 extension` 与
    `file not found` 各 0 处

  工单 #38
- chore[cpp]：C++ 测试按模块合并目标并接入 Meson 测试
  - tests/CMakeLists.txt 重写：64 个一文件一可执行改为 4 个 Catch2 分组可执行
    （catch2-num / catch2-data / catch2-runtime / catch2-contrib），
    每个 exe 只链接一次 1 GB 级 quant1x 静态库
    - 全部测试目标 EXCLUDE_FROM_ALL，新增 `tests` 聚合目标；默认构建只编库与 q1x
    - include(Catch) 启用 catch_discover_tests，用例粒度不变：每条 TEST_CASE 一个 ctest 条目
    - app-* / gtest-* 保持独立（对拍程序的"断言"是输出比对，gtest 框架不同）
  - meson.build 增加测试：同一份分组清单，catch2-with-main / gtest_main 走 pkg-config，
    全部 build_by_default: false —— `meson compile` 不构建，`meson test` 按需构建并运行
  - 修复并入分组后暴露的编译问题
    - tdd-asio.cpp：两个 TEST_CASE 重名，其一改名
    - tdd-all.cpp / tdd-cron.cpp：test_once 补 static（否则同组重复定义）
    - tdd-datasets.cpp：XTENSOR_USE_XSIMD 已由构建系统定义，补 #ifndef 防止 C4005
    - test-iconv.cpp：size_t → int 显式收敛（/WX 下 C4267 视为错误）
    - test-multibar.cpp / tdd-backtest-release.cpp：进度条从 indicators 迁到 mpb
    - tdd-strategies.cpp：删除已废弃的 indicators 头（相关代码早已注释）
    - tdd-backtest-ashare.cpp：去掉多余的 test.h，补 <map> 与 <windows.h>
  - test_fp_growth.cpp / tdd-backtest-release.cpp 自带 main，移出 Catch2 分组改为 app 目标
    （其 main 会顶掉 Catch2WithMain，导致 --list-tests 失效）
  - 暂未纳入分组并在 BUILDING.md §2.7 记录原因：tdd-snapshot.cpp（capnp 头触发
    mswsockdef.h 的 UCHAR 语法错误）、tdd-ta.cpp（gtest，仍按旧 ta::Series 接口编写）
  - 验证
    - cmake --build build-msvc --config Release --target tests → 通过
    - ctest -N → 282 条（num 97 / data 77 / runtime 78 / contrib 24 + app 6）
    - ctest -R "rolling|formula|ndarray|series|dataframe" → 19/19 通过（1 条 SKIP）
    - meson setup --reconfigure、meson compile（只编库与 q1x）、meson test catch2-num
      构建并运行成功；该组含依赖本地缓存的用例，缺数据时失败属既有情况
- fix[cpp]：修正 capnp 与 asio 的 CONST 宏冲突，tdd-snapshot 重新纳入测试分组
  - 根因：capnp/schema.h 里 `#undef CONST`（Windows 的 minwindef.h 把 CONST 定义为
    const，而 capnp 的枚举有同名成员），同一 TU 随后经 quant1x/trader/tracker.h 引入
    asio → mswsock.h → mswsockdef.h，SDK 头里的 `extern CONST UCHAR sockaddr_size[AF_MAX];`
    因 CONST 已被 undef 报 C2146（错误位置指向 UCHAR，容易误判为 winsock 头顺序问题）
  - 处理：在 tdd-snapshot.cpp 顶部先引入 quant1x/io/base.h（asio 入口），让 mswsockdef.h
    在 CONST 仍有效时完成处理；capnp 的 undef 只影响它之后的头
    - capnp 自己在 schema.h 注释里给的就是这个方案（"change the header ordering"）
  - tests/CMakeLists.txt 与 meson.build 把 tdd-snapshot.cpp 放回 catch2-data 分组
  - BUILDING.md §2.7 的排除清单只剩 tdd-ta.cpp（gtest，旧 ta::Series 接口）
  - 验证：--target tests 通过；ctest -N 287 条（原 282 + snapshot 5 条）
- refactor[go]: 更新go依赖库版本
- chore[multi]：重新执行许可证合规检查并统一声明文件命名
  - 用 labs-license-audit 重跑全语言合规检查，产物落到工具约定的
    THIRD-PARTY-NOTICES.md（连字符）：python / cpp / rust / go / java 五段机器区段刷新
  - go 段此前报 “go: updates to go.mod needed”：工具按每个模块 <module>/... 展开，
    只读模式无法解析未直接 import 的间接模块；改用 GOFLAGS=-mod=mod 后通过，
    实测不修改 go.mod / go.sum（工作区保持干净）
  - 把原手写 THIRD_PARTY_NOTICES.md 的正文并入新文件：分发形态（只发源码）、
    项目许可证与例外（csv-reader.h BSD-3）、Conan 依赖说明（csv2 本地配方引号修复、
    mpb、libiconv LGPL 履行）、下游二进制检查清单、SimHei 历史说明
  - 修正工具模板残留：正文里的 “panshi 本体” 改为 Quant1X；删掉与表格矛盾的
    “C++/Go 组件表为空”“无第三方组件” 等模板句；归档计数按实际目录填写
  - 删除旧 THIRD_PARTY_NOTICES.md，同步 4 处引用：Cargo.toml（include）、
    CMakeLists.txt（安装规则）、pyproject.toml（license-files）、README.md
  - 新增 third_party/licenses/（632 个许可证文件，4.2 MB，按语言/组件归档）
  - THIRD-PARTY-TODO.md 加入 .gitignore（工具生成、无待办时自动删除，不入库）
  - 验证：labs-license-audit cpp python rust 重跑后，围栏外正文逐字节不变
    （工具只重建围栏内表格）；go.mod / go.sum 无改动

  剩余：THIRD-PARTY-TODO.md 当前 165 条待办，主要是各组件「版权 / 主页」列需人工补录。
- docs[multi]：修正 go 许可证扫描的正确做法
  - go-licenses 按每个模块 <module>/... 展开，只读模式会因间接模块不在 require 列表
    而中止；加 -mod=mod 能扫描，但会把测试引用的 13 条间接依赖写进 go.mod / go.sum
  - 因此改为在 git archive 导出的副本里扫描，再把 Go 区段与 third_party/licenses/go/
    拷回；仓库根 go.mod / go.sum 保持提交态（本轮已还原误写入的内容）
  - 验证：副本内 GOFLAGS=-mod=mod labs-license-audit 全语言退出码 0，
    THIRD-PARTY-TODO.md 164 条待办；仓库 go.mod / go.sum 无改动
- chore[go]：为许可证扫描补齐 go.mod 间接依赖声明
  - 合规优先：labs-license-audit 的 go 段按每个模块 <module>/... 展开，只读模式无法解析
    测试引用的间接模块；按 GOFLAGS=-mod=mod 在仓库根直接扫描，接受工具补写的依赖声明
  - go.mod 新增 13 条 // indirect（creack/pty、davecgh/go-spew、golang/protobuf、
    google/go-cmp、kr/pty、kr/text、lestrrat-go/envload 等），go.sum 同步 28 条校验和
  - THIRD-PARTY-NOTICES.md：把「在副本里扫描」改为「仓库根直跑、增量随提交入库」，
    写明这是合规扫描的固有代价
  - 验证：五个语言子命令退出码均为 0；go build ./quant1x/... 通过；
    go test ./quant1x/{num,formula,indicators,pandas}/ 全部 ok
- fix[go]：回退许可证审计写入的伪依赖声明
  - labs-license-audit 的 go 段按「每个模块 <module>/...」展开，-mod=mod 下会把模块图里
    所有传递模块写成 // indirect，包括我们从不构建的测试链依赖
  - 典型链路：gopkg.in/yaml.v3 →（测试依赖）check.v1 → kr/pretty → kr/text → creack/pty；
    go mod why github.com/creack/pty 明确返回 main module does not need module，
    go mod tidy 也会把这些行全部删掉
  - 因此 go.mod / go.sum 回到真实依赖状态（撤销 43 行审计增量，含上一轮提交的 13 条与
    本轮新增的 goldmark / x/net / x/telemetry / xerrors）
  - THIRD-PARTY-NOTICES.md 的 go 段操作说明改为「在 git archive 副本里扫描后拷回」，
    并写明伪依赖的来源与判断方法；组件表保留这些传递模块（从严披露）
  - 验证：go build ./quant1x/... 通过；go test ./quant1x/{num,formula,indicators,pandas}/ 全 ok；
    git diff 5db60de -- go.mod go.sum 为空
- chore[go]：按真实依赖刷新 go 段许可证数据
  - labs-license-audit 默认只读模式跑通（不加 -mod=mod），go.mod / go.sum 无改动
  - Go 组件表收敛到真实依赖 46 行：之前因 -mod=mod 被写进模块图的测试链模块
    （creack/pty、kr/pty、kr/text、goldmark、x/net、x/telemetry 等）随 require 回退一并消失
  - 归档同步清理 13 个不再被扫描的许可证副本（go-spew、go-cmp、difflib、goldmark、
    x/net、x/telemetry 及 yaml 的 NOTICE）
  - 补录 github.com/google/go-cmp 的版权/主页（取自模块缓存 LICENSE 首行）
  - yaml 两个包的许可证表达式按本次扫描更新为 Apache-2.0, MIT
  - 待办从 164 条降到 124 条；go 段剩 4 条（go-spew / go-difflib 缺本地模块源码缓存）
  - 验证：go build ./quant1x/... 通过；git diff go.mod go.sum 为空
- chore[go]：go 段待办清零（补录 go-spew / go-difflib）
  - 本次扫描把 go-spew、go-difflib 的许可证文本补齐并归档：
    third_party/licenses/go/github.com/{davecgh/go-spew,pmezard/go-difflib}/LICENSE
  - 按归档文本补录两行的「版权 / 主页」：Dave Collins（ISC）、Patrick Mezard（BSD-3-Clause）
  - labs-license-audit go 退出码 0，THIRD-PARTY-TODO.md 中 go 条目清零（文件随之删除）
  - 全量复跑：五个语言退出码均为 0，全局待办 119 条（cpp 43 / python 26 / rust 19 /
    java 4 / rust 闭包注文 27），go 段 0 条
  - go.mod / go.sum 无改动（git diff 为空）
- chore[cpp]：把 boost 换成独立的 pfr/2.2.0
  - conanfile.py：移除 boost/1.90.0 与 header_only 选项，改为 requires pfr/2.2.0
  - CMakeLists.txt：find_package(boost) → find_package(pfr)，链接 boost::boost → pfr::pfr
  - meson.build：dependency('boost-headers') → dependency('pfr')
  - 代码侧无需改动：该包仍以 <boost/pfr.hpp> 与 boost::pfr 命名空间提供，
    实测 tuple_size_v / names_as_array / get_name / for_each_field 行为一致
  - BUILDING.md：依赖表与说明同步（记录头文件路径仍是 boost/ 前缀）
  - 验证：conan install（CMake/Meson 两套）成功；CMake 配置通过；
    app-tdd-formula-parity（57 组）、app-tdd-indicators 通过；
    catch2-num / catch2-data 构建通过；yaml 用例 13 断言通过；
    Meson 解析到 pfr 2.2.0
  - 另外：catch2-data 的 [json] 两条用例断言全部通过，但进程在退出阶段崩溃（rc=127），
    用同一序列化路径写的独立探针正常退出，属退出期问题，与本次依赖替换无关，另行排查
- chore[multi]：按审计输出更新第三方声明与许可证归档
  - THIRD-PARTY-NOTICES.md 由 labs-license-audit 重建：python / cpp / rust / go / java
    五段机器区段按当前依赖刷新，Rust 闭包带 purl 与 sha256
  - third_party/licenses/ 同步：新增 cpp（pfr、asio、csv2、xtensor 等）、java、
    python/xgboost、rust 若干组件；移除已不在模块图中的 go 传递依赖
    （creack/pty、kr/text、goldmark、x/net、x/telemetry 等）
  - 说明：文件中不再包含此前手写的「分发形态 / 项目许可证与例外 / libiconv 义务 /
    SimHei 说明」章节，这些内容可从提交 ea7aa1f 取回
- chore[python]：新增 pyrightconfig.json 排除构建产物
  - basedpyright 枚举工作区超过 10 秒：target（23G）、build-msvc（3.6G）、
    vcpkg_installed（970M）、build-meson（567M）、temp（250M）等目录与 Python 无关
  - 排除 build / build-msvc / build-meson / target / temp / output / vcpkg_installed /
    .cache / .ruff_cache / .zig-cache / third_party/licenses（632 个许可证文本目录）
  - 保留 third_party/recipes 与 quant1x、tests、scripts、tools 的检查范围
- chore[cpp]：xtensor 升级到 0.27.2（本地配方）
  - 0.27.2 未收录于 Conan Center（当前只到 0.27.1），新增 third_party/recipes/xtensor：
    按 CCI 配方结构编写 conanfile.py，conandata.yml 固定 0.27.2 源码与 sha256
    （4421d2f7…7ac6d70）
  - 首次构建前需执行 conan create third_party/recipes/xtensor
  - conanfile.py 依赖由 xtensor/0.27.1 改为 0.27.2，xtl/0.8.0、xsimd/13.2.0、
    nlohmann_json/3.11.3 保持不变
  - BUILDING.md 依赖表更新，并把「本地配方」说明扩到 csv2 / xtensor 两项
  - 验证：conan create 成功（75 个 .hpp + LICENSE）；CMake 与 Meson 两套输出均解析到
    0.27.2（xtensor.pc Version: 0.27.2，meson clearcache 后显示 YES 0.27.2）；
    CMake 构建 0 错误，app-tdd-formula-parity 57 组、app-tdd-indicators 通过；
    catch2-num / catch2-data 构建通过；Meson 全量重编（96 步）通过
- chore[cpp]：修掉 CMakeLists 的行长告警（C0301）
  - 新增 .cmake-format.yaml：format.line_width = 120，cmake-lint 的 C0301 按此行宽判定
    （lint.max_line_length 在 cmake_format 0.6 里不被识别，只认 format.line_width）
  - CMakeLists.txt：把两条超过 120 字符的注释（CMAKE_CXX_FLAGS_DEBUG 示例）折成两行；
    补回文件末尾换行（C0304）
  - 验证：cmake-lint CMakeLists.txt 的 C0301 / C0304 均为 0；CMake 配置仍可正常生成
  - 说明：cmake-lint 另外报 143 条 C0307（缩进）等既有风格问题，与本次行长告警无关，
    需要时可用 cmake-format 统一格式化
- chore[cpp]：清掉 CMakeLists.txt 的 cmake-lint 风格告警（C0307/C0303/C0103/C0111）
  - .cmake-format.yaml：补 format.tab_size = 4，与仓库统一的 4 空格缩进对齐，C0307（缩进）127 → 0；
    不跑 cmake-format -i，CMakeLists.txt 的写法与折行保持原样
  - CMakeLists.txt：清 7 处行尾空白（C0303）；get_all_linked_targets 补 docstring（C0111）
  - CMakeLists.txt：C0103（命名）按站点写 # cmake-lint: disable 说明，不重命名既有变量与 CACHE 键：
    argparse_VERSION / mimalloc_VERSION 沿用 find_package 的 <包名>_VERSION 约定，
    quant1x-headers / quant1x-sources 是 BUILDING.md 登记的清单名，lib_suffix / quant1x_DEBUG_POSTFIX
    为内部变量
  - 验证：cmake-lint CMakeLists.txt 为 0 条；cmake -S . -B build-msvc 改动前后配置日志逐行一致，
    DEBUG_POSTFIX 仍为 -release-msvc-win；git diff --check 无空白错误
  - 工单：#40
- refactor[python]：以 quant1x/io 为准裁撤 quant1x/net 并统一日志后端
  - 删除 quant1x/net（conn / endpoint / handler / tcp_client_pool）：它是 quant1x/io 连接池的副本，
    逻辑等价但已与 io 出现日志后端、告警级别、日志内容三处分歧；io 才是跨语言 spec 所在
    （connection_pool.h/.rs/.go、endpoint.h/.rs/.go、operation_handler.h/.rs/.go）
  - quant1x/contrib/data/tdx/{client,protocol,config}.py：导入改到 quant1x.io.connection_pool
    与 quant1x.io.operation_handler
  - quant1x/io/connection_pool.py：日志从 stdlib logging 换成 quant1x.log.logger（loguru），
    占位符 %s 改为 {}；删除未使用的 Tuple 导入；端点耗尽仍为 warning
  - quant1x/log/logger.py：配置收进 LoggingSetup(ThreadSafeSingletonABC) 单例，替代
    getattr(__logger, "_quant1x_configured") 的模块级判断，进程内只注册一组 sink
  - quant1x/base/singleton.py：去掉 logging 与全部日志调用（含导入时执行的 logging.basicConfig）
  - quant1x/runtime/scheduler.py：原先 logger 被覆盖为静音的 apscheduler logger，本模块日志被整体吞掉；
    改回 quant1x.log，apscheduler 自带日志用 _apscheduler_logger 单独静音
  - quant1x/__init__.py：_logger 换成 quant1x.log.logger（实测无循环导入），占位符改为 {}
  - tests/test_connection_pool.py：修正导入名（OperationHandler -> NetworkOperationHandler）；
    is_open() / endpoint 断言换成句柄公开 API 的回声往返，用例由收集失败恢复为通过
  - quant1x/encoding/README.md：协议编解码目录指向 quant1x/io
  - 验证：python -m pytest tests/test_connection_pool.py -q（1 passed）；
    import quant1x / quant1x.io.connection_pool / tdx.client / runtime.scheduler 均通过；
    8 线程并发取日志单例后 handlers 仍为 2、get_instance_count() 为 1；
    grep "quant1x.net" 无结果（CHANGELOG 历史条目除外）
  - 工单：#41
- chore[python]：conanfile.py 收窄 requires / options 的静态类型
  - ConanFile 把 requires / options 声明为类属性 None，真正对象由 conan 在加载配方时
    注入实例属性，静态检查于是在这两处报 31 个 reportOptionalCall / reportOptionalSubscript
  - 类体内以 `if TYPE_CHECKING` 声明这两个属性为 Any：只做类型收窄，不生成类属性，
    运行期行为不变（配方模块可正常加载，两属性仍由 conan 注入）
  - 验证：pyright conanfile.py 0 error；ruff check conanfile.py 通过
- chore[multi]：Rust 依赖闭包组标题改用三级标题
  - 用更新后的 labs-license-audit 重新生成：闭包渲染改为输出 `### <表达式>（<注文>）`，
    27 个许可证分组标题由整行加粗改为真标题，消除 markdownlint MD036（emphasis-as-heading）
  - 解析侧同时兼容旧文件里的 `**…**` 组标题，故重建继承了原有分组顺序、履行注文与组件行；
    third_party/licenses/ 无变化
  - 验证：重跑 `python -m panshi.tool.license.audit rust` 退出码 0 且 diff 不增长（幂等）；
    git diff 仅标题行变化（27 增 27 删）；文件诊断只剩既有的 MD034（机器生成的版权列裸 URL）
- chore[cpp]：xtensor 本地配方消除 Pylance / Ruff 告警
  - ConanFile 把 requires / tool_requires / info / conan_data 声明为类属性 None，真正
    对象由 conan 注入实例属性；options 在配方里是 dict、运行期却是 Options 对象。
    静态检查因此在两处报 23 个 reportOptionalCall / reportOptionalSubscript /
    reportAttributeAccessIssue
  - 类体内以 `if TYPE_CHECKING` 声明上述注入属性为 Any；options / default_options 加
    ClassVar[Any]，一并消除 RUF012（mutable class default）
  - source_folder / package_folder 是基类返回 Optional 的 property，不能在被继承方重标类型，
    故在使用处 cast(str, ...) 收窄（source() 的 destination、package() 内绑定一次供两次 join）
  - 导入按 ruff 重排（I001）：os 提到首行、新增 typing、conan.tools.cmake 成员按字典序；
    仓库根有 conan/（profiles）目录，ruff 把 conan 判为 first-party，`from conan import ConanFile`
    因此单独成段排在最后，这是该目录结构下过的了 I001 的稳定形态
  - 验证：pyright 对该文件 0 error；ruff check 通过；配方类可正常加载，
    name / version / options / default_options 取值与改前一致
- chore[cpp]：csv2 测试包配方收窄 requires / cpp 的静态类型
  - ConanFile 把 requires / cpp 声明为类属性 None，真正对象由 conan 注入实例属性，
    静态检查因此在 `self.requires(...)` 与 `self.cpp.build.bindirs[0]` 两处报
    reportOptionalCall / reportOptionalMemberAccess
  - 类体内以 `if TYPE_CHECKING` 声明这两个属性为 Any：只做类型收窄，运行期不生成属性
  - 导入按 ruff 重排（I001）：os 提到首行、新增 typing；`CMakeToolchain` 仅以
    generators 字符串形式使用、类未引用，随 F401 移除，行为不变
  - 验证：pyright 对该文件 0 error；ruff check 通过
- fix[java]：DistributedIdTest 改用 Java 8 可用的 orElseThrow 形态
  - `Optional.orElseThrow()` 无参版本自 Java 10 才有，本项目 java.version=8，
    该调用按编译合规性判为错误（ECJ 报 method not applicable）
  - 改为 `orElseThrow(() -> new AssertionError("state file not found after restart"))`，
    与 Java 8 兼容，同时把「重启后状态文件缺失」这一失败原因写清
  - 验证：mvnw -B test-compile 通过；mvnw -B -Dtest=DistributedIdTest test → Tests run: 5,
    Failures: 0, Errors: 0；IDE 诊断 0 条
- release version 0.9.0

## [0.8.19] - 2026-11-24
### Changed
- chore[python]: 调整依赖版本
- release version 0.8.19

## [0.8.18] - 2026-11-19
### Changed
- chore[python]：将最低 Python 版本提升到 3.12
  - pyproject.toml 的 requires-python 改为 >=3.12，classifiers 移除 3.8 / 3.9 / 3.10 / 3.11
  - CONTRIBUTING.md §2.1 与 BUILDING.md §1 的版本基线同步为 3.12+；README 原已是 Python 3.12+
  - 依赖中的 numpy / scikit-learn / mlxtend 实际要求 Python >=3.11，旧下限已不可安装
  - 关联工单 #34
  - 验证：TOML 解析确认 requires-python 与 classifiers；README / CONTRIBUTING / BUILDING 无 3.8 残留
- release version 0.8.18

## [0.8.17] - 2026-11-19
### Changed
- fix[python]：对齐 scikit-learn 与 mlxtend 的版本约束
  - pyproject.toml 的 scikit-learn 约束由 ~=1.7.1 提升到 ~=1.8.0，满足 mlxtend 0.25.0 的 scikit-learn>=1.8.0
  - 现有 numpy / pandas / scipy / matplotlib / joblib 的 ~= 范围已能容纳 mlxtend 的补丁下限，未改动
  - 关联工单 #33
  - 验证：pip install --dry-run 在 Python 3.13 下同时解析出 scikit-learn 1.8.0、mlxtend 0.25.0、numpy 2.3.5、pandas 2.3.3、scipy 1.16.3、matplotlib 3.10.9、joblib 1.5.3
- release version 0.8.17

## [0.8.16] - 2026-11-19
### Changed
- chore[rust]：全仓执行 cargo fmt 统一代码格式
  - 对全仓 Rust 源文件执行 cargo fmt，覆盖 quant1x/、tests/、examples/、tools/ 共 82 个文件
  - 仅换行、缩进与 import 排序等格式调整，无逻辑、接口、依赖与版本变更
  - 关联工单 #32
  - 验证：cargo fmt --check；cargo test --lib globmatch（8 项通过）
- release version 0.8.16

## [0.8.15] - 2026-11-19
### Changed
- add[license]: 新增可忽略源文件头部版权信息的配置文件
- feat[python+cpp+rust+go+zig]：在 base 落地锚定 glob 匹配
  - 新增 quant1x/base/globmatch.py 作为 spec，同步落地 globmatch.hpp / .cpp、globmatch.rs、globmatch.go、globmatch.zig 与模块文档 globmatch.md
  - 语义与 Python 3.13 PurePath.full_match()、bash globstar 一致：整条锚定、** 跨段、其余段按 * / ? / [...] 通配，尾段 /** 要求至少一个条目，只做匹配不访问文件系统
  - 五种语言使用同一批样本逐条对拍，覆盖 globstar 零/多段、锚定、单段通配、点文件与大小写、尾段 /**、空输入与任一命中
  - C++ 用例按仓库约定落在 tests/tdd-globmatch.cpp；登记根 CMakeLists.txt、tests/CMakeLists.txt 与 quant1x/base/mod.rs
  - 关联工单 #30
  - 验证：python -m quant1x.base.globmatch_test；cargo test --lib globmatch；go test ./quant1x/base/；zig test quant1x/base/globmatch_test.zig；ctest --test-dir build-msvc -C Release -R globmatch
- chore[multi]：统一项目元数据作者并补充 README 中文名
  - pyproject.toml 的 authors 名称统一为 Quant1X Team，保留 wangfengxy@sina.cn
  - Cargo.toml 的 authors 改为 Quant1X Team <wangfengxy@sina.cn>
  - README 标题补充 Quant1X 的中文名「鲲腾」
  - 两处许可证保持 MIT，未改动版本号、tag 与代码逻辑
  - 关联工单 #31
  - 验证：两个清单经 TOML 解析确认 authors / license 取值；三个文件均为 UTF-8 无 BOM、LF
- release version 0.8.15

## [0.8.14] - 2026-11-17
### Changed
- fix[python]：修掉 factors 与测试里残留的 quant1x.std 引用
  - quant1x/factors/share_holder.py、quant1x/factors/financial_report.py、tests/test_share_holder.py：from quant1x import std 改为 from quant1x import base，std.get_quarter_by_date 改为 base.get_quarter_by_date
  - 0.7.56 把 quant1x.std 并入 quant1x.base，Python 侧这 3 个文件漏改，quant1x.std 已不存在，导入即失败
  - 语义一致：get_quarter_by_date 是同一实现（quant1x/base/time.py），参数与返回值不变
  - 验证：grep 无 import std / std.get_quarter_by_date 残留；三个模块导入成功并返回值正确；test_get_quarter_by_date 通过；py_compile 通过
  - 工单：#26
- feat[python]：补涨停比例与涨停价，修好 hjk/p2fb 的 basic 依赖
  - quant1x/data/market.py：新增 limit_up_ratio(code, name="")——沪深主板 10%、创业板/科创板 20%、北交所 30%、主板 ST 5%、创业板/科创板 ST 20%、港股/美股 0；6 位 43/83/87/92 前缀兜底北交所（sse_rules 的 "8" 兜底会把老北交所代码误判为上交所债券）
  - quant1x/formula/formula.py：新增 ZTPRICE(price, ratio)——前收 ×（1+比例），Decimal ROUND_HALF_UP 到分；支持标量/Series/ndarray，NaN 原样保留
  - quant1x/indicators/hjk.py、p2fb.py：去掉 from quant1x import basic，改调 limit_up_ratio / ZTPRICE（对应原注释 ZTBL:="BASIC.涨停比例"、ZTJ:=ZTPRICE(...)）
  - 新增 quant1x/data/market_test.py：11 项覆盖各板块、主板 ST、创业板/科创板 ST 仍 20%、无涨跌幅市场，以及 9.55*1.10=10.505 → 10.51 的 half-up 边界
  - 语义一致：指标其余公式步骤未动；ST 比例需要名称，指标只传 code 时按板块基础比例
  - 验证：python -m unittest quant1x.data.market_test（11 passed）；cdtd/f89k/hjk/ma1x/p2fb 用合成 OHLCV 跑通；py_compile 通过；ruff 无 F821 新增（指标文件的 F403/F405 为既有 import * 风格）
  - 工单：#27
- fix[python]：涨停比例改为 ST 统一 5%、科创板 30%
  - quant1x/data/market.py：limit_up_ratio 口径调整为 ST 统一 5%（优先于板块判断）、创业板 20%、科创板 30%、北交所 30%、其它主板 10%
  - 常量拆分为 LIMIT_RATIO_CHINEXT / LIMIT_RATIO_STAR / LIMIT_RATIO_BSE / LIMIT_RATIO_ST
  - quant1x/data/market_test.py：拆分创业板与科创板用例，ST 用例覆盖主板、创业板、科创板、北交所
  - 语义一致：ZTPRICE 与指标公式未动
  - 验证：python -m unittest quant1x.data.market_test（11 passed）；cdtd/f89k/hjk/ma1x/p2fb 合成数据跑通；py_compile 通过
  - 工单：#28
- fix[python]：科创板涨停比例改回 20%
  - quant1x/data/market.py：LIMIT_RATIO_STAR 0.30 → 0.20，北交所 LIMIT_RATIO_BSE 保持 0.30
  - quant1x/data/market_test.py：科创板用例断言改为 0.20
  - 语义一致：ST 统一 5%、创业板 20%、其它主板 10% 不变
  - 验证：python -m unittest quant1x.data.market_test（11 passed）；hjk/p2fb 合成数据跑通；py_compile 通过
  - 工单：#29
- release version 0.8.14

## [0.8.13] - 2026-11-17
### Changed
- chore[go]：升级 num 到 0.8.4，移除 gox 依赖
  - go.mod / go.sum：github.com/quant1x/num 0.8.3 → 0.8.4；该版本不再引用 gox，github.com/quant1x/gox 从间接依赖中移除，go.sum 同步删掉它的校验和
  - golang.org/x/sys 0.49.0 → 0.50.0
  - 语义一致：只动依赖清单，未改任何源码
  - 验证：go mod tidy -diff 无差异；grep quant1x/gox go.mod go.sum 无结果；go build ./...；go vet ./quant1x/command/ ./quant1x/base/ ./quant1x/encoding/yaml/ ./quant1x/config/ ./tools/tail/；go test ./quant1x/command/ ./quant1x/base/ ./quant1x/encoding/yaml/ ./quant1x/config/
  - 工单：#10
- chore[python]：把 % 拼接改为格式化字符串
  - .githooks/commit_msg_check.py / text_check.py、quant1x/io/connection_pool.py、quant1x/net/tcp_client_pool.py：%s / %d 改成 str.format()
  - quant1x/contrib/data/tdx/level1/ext/__init__.py：日期与时间戳的 %d-%02d-%02d 拼接改成 str.format()
  - quant1x/formula/formula.ipynb：9 个 cell 的 print("... %d" % (N)) 改成 f-string
  - 语义一致：只改字符串构造方式，取值、判断与输出文案不变（%-9s 对齐改为 {:<9}）
  - 验证：ruff check --select UP031 . 无告警（只剩 market.py 两条 invalid-syntax，属 Python 3.8 基线与 match 语句，不在本单）；改动过的 .py 全部 py_compile 通过；.githooks 两个脚本手工运行均通过
  - 工单：#11
- chore[python]：format 调用改为 f-string
  - .githooks/commit_msg_check.py、.githooks/text_check.py、quant1x/io/connection_pool.py、quant1x/net/tcp_client_pool.py：str.format() 改为 f-string
  - quant1x/contrib/data/tdx/level1/ext/__init__.py：日期与时间戳的 str.format() 改为带宽度说明符的 f-string（{month:02d} 等）
  - 语义一致：只改字符串构造方式，取值、判断与输出文案不变
  - 验证：ruff check --select UP031,UP032 . 除 market.py 的两条 invalid-syntax 外无告警；改动过的 .py 全部 py_compile 通过；.githooks 两个脚本手工运行通过
  - 工单：#12
- chore[python]：消除 .githooks 的未使用解包与 subprocess 缺 check 告警
  - .githooks/commit_msg_check.py：detect_mojibake 返回的三元组里 enc 未使用，改名为 _enc
  - .githooks/text_check.py：_git() 的 subprocess.run 显式传 check=False，失败仍由 returncode 判断后抛 RuntimeError
  - 语义一致：无行为变化
  - 验证：ruff check --select RUF059,PLW1510 .githooks 无告警；两个脚本 py_compile 通过并手工运行退出码 0
  - 工单：#13
- fix[python]：text_check 改用 capture_output，并分开取文本与字节输出
  - .githooks/text_check.py：subprocess.run 改 capture_output=True；拆出 _run_git / _git / _git_bytes，让“取文本”和“取原始字节”各有明确返回类型，消除 split("\0")、split(b"\x0a")、startswith(b"+++") 三处 bytes/str 混用的类型错误；残留的 .format 改成 f-string
  - scripts/run_lmax_benchmark.py：subprocess.run 显式 check=False，失败仍由输出匹配判断
  - 语义一致：命令与失败判定逻辑不变
  - 验证：ruff check --select UP022,PLW1510,UP031,UP032 . 无告警；pyright .githooks/text_check.py 0 errors；py_compile 通过；两个钩子脚本手工运行通过
  - 工单：#14
- refactor[python]：market 的 match 语句改为 if/elif，恢复 3.8 基线兼容
  - quant1x/data/market.py：detect_instrument_type 的 match exchange 改为字典查表；detect_instrument 的 match code_len 改为 if/elif，逐分支与原 case 对齐（case _ 对应 else）
  - 未使用的解包变量加下划线前缀（RUF059）
  - 语义一致：分支互斥关系、返回值与规则选择顺序不变；3.8 下不再语法错误
  - 验证：ruff check（py38 target）无语法告警与 RUF059/UP031/UP032 告警；pyright quant1x/data/market.py 0 errors；py_compile 通过
  - 工单：#14
- chore[python]：协议解析里未使用的解包变量加下划线前缀
  - quant1x/contrib/data/tdx/（financial_report、level1/ext/__init__、level1/fund、level1/std/security_quote、level1/std/xdxr_info、level1/ext/xdxr_hkex）
  - quant1x/factors/（f10、financial_report）、quant1x/learn/regime/regime_switching_framework.py、tests/test_connection_pool.py
  - 都是 struct.unpack / 多返回值里没用到的占位字段，改名成 _name，不改取值顺序与逻辑
  - 验证：ruff check --select RUF059 . 无告警；涉及文件全部 py_compile 通过
  - 工单：#14
- fix[cpp]：main 的 Argument 改用赋值构造，消除缺失字段告警
  - main.cpp：--verbose / --debug / --version / --pipe 的 Argument 由聚合初始化改为先默认构造再赋值，不再跳过 names 等字段
  - 语义一致：登记到解析器的参数与原来完全相同
  - 验证：cmake --build build-msvc --config Release --target q1x 通过；q1x --help 退 0、update --help 退 0、--no-such 退 2
  - 工单：#15
- chore[python]：补 from __future__ import annotations
  - main.py 与 18 个模块补 future 导入：argparser（complete、derive 及其用例）、base（dotenv、expandvars、hotreload）、contrib/data/tdx（config、datasource、level1/ext、xdxr）、data（datasource、ticker_rules/rule）、core/decode、version、scripts/run_lmax_benchmark
  - 这些文件用 PEP 585 / PEP 604 写法（list[str]、X | Y），3.8 基线下需要延迟求值
  - 语义一致：注解不再在运行期求值，typing.get_type_hints 仍按模块命名空间解析（derive 与 dataclass_utils 的用法已由用例覆盖）
  - 验证：ruff check --select FA102 . 无告警；改动文件 py_compile 通过；argparser 的 parser/derive/help/complete 四个 Python 用例全通过；pyright main.py 0 errors
  - 工单：#15
- test[go]：基准循环改用 b.Loop()
  - quant1x/base/filesystem_test.go、quant1x/data/meta/timestamp_test.go、quant1x/distributed/{id,id128,id64}/id_test.go、quant1x/encoding/binary/struc/bench_test.go：41 处 for i := 0; i < b.N; i++ 改为 for b.Loop()
  - RunParallel 里的 pb.Next() 保持原样：b.Loop() 不适用于并行基准
  - 语义一致：循环体不使用下标变量；b.Loop() 在首次迭代自动重置计时，循环前的 setup 不再计入耗时
  - 验证：go build ./...；go vet（struc 的 Fields.String 告警为既有）；go test -run '^$' -bench . -benchtime=1x 在 base / data/meta / distributed/id{,128,64} / encoding/binary/struc 六个包通过
  - 工单：#16
- fix[python]：logger 改为导出公开名字，去掉 dunder 跨模块导入
  - quant1x/log/__init__.py：from .logger import __logger as logger 改成 from .logger import logger
  - 原因：__logger 是模块私有 dunder 名字，跨模块导入会被静态检查判为 unknown import symbol；logger.py 末尾已有 logger = __logger 的公开导出，__all__ 也是 logger
  - 语义一致：绑定的仍是同一个 loguru Logger 实例
  - 验证：pyright quant1x/log/__init__.py 0 errors；from quant1x.log import logger 实测导入成功（本机缺 python-dotenv，用临时桩补齐依赖链，桩不入库）
  - 工单：#17
- fix[python]：ext 协议的 serialize_request_body 返回真正的 bytes
  - quant1x/contrib/data/tdx/level1/ext/__init__.py：6 处声明 -> bytes 却返回 bytearray 的方法改为返回 bytes —— padding / pkg / body 包一层 bytes()，两处 bytearray.fromhex 改成 bytes.fromhex
  - 语义一致：字节内容与长度不变；调用方 protocol.py 只做 len() 与字节拼接，不修改返回值
  - 验证：pyright quant1x/contrib/data/tdx/level1/ext/__init__.py 0 errors；全包 pyright 不再有 bytearray 报告；py_compile 通过
  - 工单：#18
- chore[go]：忽略本地 go.work（gox 已不在依赖图）
  - .gitignore 增加 go.work 与 go.work.sum：这两类文件是本机多模块编排，不入库
  - 背景：gopls 提示某个文件位于模块缓存 gox@v1.27.1 内、建议加 go.work；但 go mod graph / go list -m all 里已无 quant1x/gox，模块缓存目录只读且路径随版本变化，不应写进仓库
  - 验证：.gitignore 含这两条；git status 干净
  - 工单：#19
- fix[python]：清理未使用变量，指标模块按公式保留步骤
  - quant1x/indicators：9 处 pandas.eval('A & B') 改成直接表达式，变量引用改为静态可查；其余中间序列是公式移植的算法步骤，pyproject 为 quant1x/indicators/*.py 豁免 F841 并写明理由
  - 其它文件的死赋值逐处清理：base/filesystem.py、base/system.py（删掉永远不返回的“方法2”调试扫描）、tdx/level1/{auction,ext/__init__,ext/hk_f10,ext/xdxr_hkex,std/security_quote,money,sector,xdxr}.py、learn/regime、runtime/ringbuffer_test.py、tests/test_share_holder.py、tests/test_xdxr.py
  - 有副作用的读取保留：struct.unpack 的长度校验与 fh.read(384) 的文件指针推进改成 _ = ...；money.py 的 split_number 注释不再提已删的 ratio_shares
  - 语义一致：删除项均无副作用
  - 验证：ruff check --select F841 . 全绿；改动文件 py_compile 通过；cdtd / f89k / ma1x 用合成 OHLCV 跑通并返回 DataFrame
  - 工单：#20
- fix[python]：补齐 rich 与 python-dotenv 依赖
  - pyproject.toml：dependencies 增加 rich~=15.0.0（main.py 的进度条与终端输出）与 python-dotenv~=1.0（quant1x/config/config.py 读取 .env）
  - 这两项此前没有声明：按 pyproject 建出的环境一跑 main.py 就 ModuleNotFoundError: dotenv，静态检查也会报 rich.console / rich.progress 无法解析
  - 语义一致：只补依赖声明，不改代码
  - 验证：python main.py --help 退 0；update --help 退 0；--no-such 退 2；update 无插件打印 Nothing to update. 退 0；pyright main.py 0 errors
  - 工单：#21
- refactor[python]：main 的进度条改用 tqdm，去掉 rich 依赖
  - main.py：移除 rich.console / rich.progress，进度改用已有依赖 tqdm：外层按 plugin、内层按股票，两级进度条；console.print 改成 print
  - pyproject.toml：删除 rich 依赖
  - 语义一致：plugin 与股票两级进度、成功/失败计数、SIGINT 中断与收尾提示都不变
  - 验证：python main.py --help 退 0；update --help 退 0；--no-such 退 2；update 无插件打印 Nothing to update. 退 0；py_compile 通过
  - 工单：#22
- fix[python]：修正 formula_plus 的导入路径与弃用装饰器
  - quant1x/formula/formula_plus.py：from formula import SUM, DMA, REF 改为 from quant1x.formula import ...
  - 同文件的 deprecated.sphinx.deprecated 换成文件内的 warnings 装饰器，不再依赖未声明的 Deprecated 包；调用被标记函数仍发 DeprecationWarning
  - 语义一致：函数行为不变，弃用提示保留「函数名 + 起始版本 + 原因」
  - 验证：python -c "import quant1x.formula.formula_plus" 成功；调用 HHV 触发 DeprecationWarning；py_compile 通过
  - 工单：#22
- fix[python]：补齐 mlxtend 依赖
  - pyproject.toml：dependencies 增加 mlxtend~=0.25（quant1x/learn/fpgrowth/fp_growth_example.py 的 FP Growth 示例）
  - 之前未声明，包内该示例 import 即失败
  - 语义一致：只补依赖声明
  - 验证：python -m quant1x.learn.fpgrowth.fp_growth_example 退出码 0，正常输出频繁项集与关联规则
  - 工单：#23
- fix[python]：__init__ 的版本回退链改为记录异常
  - quant1x/__init__.py：_get_version() 的 3 处 except ...: pass 改成 logger.debug，带上异常与「回退到下一步」的说明
  - 日志用标准库 logging：quant1x.log 反向依赖 quant1x.config，包初始化阶段引入会成环
  - 语义一致：三级回退顺序与最终 0.0.0-dev 兜底不变，只多 debug 级日志（默认不输出）
  - 验证：import quant1x 取到 0.8.13.dev11；ruff check --select S110 quant1x/__init__.py 无告警；默认规则通过
  - 工单：#24
- chore[python]：可忽略的异常标注 S110，裸 except 收窄为 Exception
  - 55 处「失败可忽略」的 except ...: pass 补 # noqa: S110 - 原因：文件时间与缓存标记、套接字选项与关闭连接、dotenv/文件探测、可选因子字段、临时文件清理、本地缓存读取失败后重取
  - 3 处失败会影响结果的改为 logger.warning：data/meta/calendar.py 两处更新交易日历缓存、tdx/instruments.py 写证券列表缓存
  - 7 处裸 except: 收窄为 except Exception:（base/time.py、ext/hk_f10.py×2、tdx/money.py、data/schema/adjustment.py、learn/regime、runtime/scheduler.py），KeyboardInterrupt / SystemExit 不再被吞
  - labs/akshare.ipynb 的裸 except 同样收窄并标注
  - 语义一致：除新增 warning 与不再吞 BaseException 外，处理路径不变
  - 验证：ruff check --select S110,E722 . 全绿；改动文件 py_compile 通过；22 个相关模块导入成功（factors/{f10,financial_report,share_holder} 因 quant1x.std 缺失导入失败，属既有问题）
  - 工单：#25
- release version 0.8.13

## [0.8.12] - 2026-11-16
### Changed
- refactor[go]：将 gox/encoding/yaml 迁入 quant1x/encoding/yaml
  - 新增 quant1x/encoding/yaml：Unmarshal（按结构体 tag 的 default 补默认值）/ Marshal / Node，语义与 gox v1.27.1 的 encoding/yaml 一致
  - quant1x/config（trader_parameter.go、config_yaml_test.go）与 quant1x/core（base.go、decode.go）的导入改指仓内路径
  - 新增文档：quant1x/encoding/README.md、quant1x/encoding/yaml/README.md、quant1x/encoding/yaml/yaml.md
  - 语义一致：default 三条口径、空文档 no-op、非指针错误文案、Node 与 yaml.v3 的 UnmarshalYAML 兼容性均保持不变
  - 验证：go build ./...；go vet ./quant1x/encoding/yaml/ ./quant1x/config/ ./quant1x/core/；go test ./quant1x/encoding/yaml/ ./quant1x/config/
  - 工单：#3
- refactor[go]：factors 时间解析改用 base，去掉 gox/api.ParseTime 依赖
  - quant1x/base/time.go：日期格式表补 060102 / 20060102150405 / 2006-01-02 15:04:05.000，覆盖 gox 原有的 6 / 8 / 10 / 14 / 19 / 23 位输入
  - quant1x/factors/option.go：3 处 api.ParseTime 改走 base.ParseDate + time.UnixMilli（0 视为无法解析），删除 gox/api 导入
  - quant1x/base/time_test.go、quant1x/factors/option_test.go：补新格式与非法输入用例
  - go.mod：github.com/quant1x/gox 由直接依赖降为间接依赖，仅剩 quant1x/num 的传递链
  - 语义一致：ParseDate 只扩格式不回退，既有输入返回值不变；「无效的交易日期」文案与落零值时间的行为不变
  - 验证：go build ./...；go vet ./quant1x/base/ ./quant1x/factors/；go test ./quant1x/base/；go test ./quant1x/factors/ -run TestParseTradeDate
  - 工单：#4
- fix[go]：修复 Export 漏关闭文件句柄，并让 embed 用例不再污染工作区
  - quant1x/base/embed.go：Export 补 defer src.Close() 与显式 output.Close()，避免句柄泄漏导致 Windows 上导出文件无法删除或覆盖
  - quant1x/base/embed_test.go：导出目标由 embed.go.copy 改为 embed_test.out（命中 .gitignore 的 *.out），补断言与 t.Cleanup 清理
  - 删除入库的测试产物 quant1x/base/embed.go.copy
  - 语义一致：Export 签名与导出结果不变，仅补齐资源释放
  - 验证：go test ./quant1x/base/ -run TestEmbed；go test ./quant1x/base/；go build ./...；go vet ./quant1x/base/
  - 工单：#5
- chore[rust]：清理 cargo publish 的目标发现与 homepage 告警
  - Cargo.toml：include 补 /examples/*.rs 与 /tests/*.rs，示例与集成测试源码随之入包
  - 删除 tests/Cargo.toml 与 tests/Cargo.lock：空依赖的嵌套 crate 会把 tests/ 整体排除在父包之外，导致 include 对 /tests/*.rs 失效；该 crate 无引用，根目录 cargo test 本就通过 autotests 运行这两个集成测试
  - Cargo.toml：删除与 repository 重复的 homepage，消除 redundant_homepage 告警
  - 语义一致：不改任何 .rs 代码、不碰版本号与 tag；examples/ tests/ 下的非 Rust 资产仍不入包
  - 验证：cargo package --allow-dirty 零 warning（133 files）；cargo package --list --allow-dirty 含 5 个新增 .rs；cargo test --test calendar_integration --no-run 与 --test fetch_securities --no-run 通过
  - 工单：#6
- refactor[go]：main 的命令行解析由 cobra 迁到 base/argparser
  - main.go：拆成 run() int + os.Exit(run())，主流程末尾改调 command.Run(os.Args[1:], os.Stdout, os.Stderr)
  - quant1x/command/app.go：newRootParser() 用 base/argparser 构建根命令（--debug / --avx2 / --cpu 均为 Global），Run() 负责解析、应用全局开关（取代 PersistentPreRun）与子命令分发
  - quant1x/command/command_update.go：拆成 registerUpdate() 与 runUpdate()，业务逻辑与输出文案不变
  - quant1x/command/command_flag.go：删除 cmdFlag[T] / commandInit / type Command，补 namespaceBool / namespaceString / namespaceInt
  - 新增 quant1x/command/app_test.go：全局选项写在子命令之后、默认值、--help、未知选项退 2
  - 语义一致：选项名、默认值、说明文字不变；退出码改为帮助 0 / 用法错误 2（原实现忽略 Execute 返回值，恒 0）
  - 验证：go build ./...；go vet ./quant1x/command/ ./quant1x/base/argparser/；go test ./quant1x/command/ ./quant1x/base/argparser/；手工跑 q1x-go --help、update --help、--no-such-flag
  - 工单：#7
- refactor[python]：main 的命令行解析由 click 迁到 base/argparser
  - main.py：click.group / @click.option → quant1x.base.argparser.Parser；build_parser() 建根命令与 update 子命令，run_update(namespace) 承接原 update 主体
  - --all / --base / --feature 的选项名、帮助文字与逗号切分口径不变；并发、进度条与 SIGINT 逻辑未动
  - 入口改为 parse_or_exit()：帮助退 0、用法错误退 2
  - 语义一致：更新流程与输出文案逐行保留
  - 验证：python -m py_compile main.py；解析器接线用临时脚本核对（update / --all / --base 落值、--help 退 0、未知选项退 2）；本机缺 python-dotenv，main.py 端到端未执行
  - 工单：#8
- refactor[cpp]：main 的命令行解析由 argparse 迁到 base/argparser
  - main.cpp：p-ranav argparse::ArgumentParser → quant1x::argparser::Parser；--verbose / --debug / --version 与 service / update 子命令重新登记，parse_or_exit 的返回值决定退出码
  - quant1x/command.h / command.cpp：cmdFlags 去掉 value 字段，SubCommand::handler 改收 (Parser&, const Namespace&)；update() 从命名空间读 all / base / features / start / end
  - quant1x/app.h / app.cpp：daemon() 改收 const Namespace&，action 与 --pipe 从命名空间取
  - quant1x/runtime/core.cpp：atexit 钩子由 shutdown()（内部 _exit(0)）换成只刷日志的 flush_logs()，否则帮助文本被缓冲吞掉、退出码恒为 0
  - 语义一致：子命令、选项名、默认值与输出文案不变；update 的 --base / --features 现在能真正取到命令行取值（原先 store_into 未生效）
  - 验证：cmake --build build-msvc --config Release --target q1x；q1x --help 退 0、update --help 退 0、--no-such 退 2、update 无参数提示无适配器
  - 工单：#8
- refactor[rust]：main 的命令行解析由 clap 迁到 base/argparser
  - main.rs：clap 的 Command/Arg 构建换成 quant1x::argparser；根命令 --version / --verbose / --debug，service 与 update 子命令重新登记，parse_or_exit 决定退出码
  - quant1x/app.rs：try_run_subcommand 的 &clap::ArgMatches 改为 &Namespace，取值改走 Value::as_bool / Value::as_list
  - Cargo.toml / Cargo.lock：移除 clap 依赖
  - 语义一致：子命令、选项名、帮助文案与 update 的默认行为（无开关时更新全部）不变
  - 验证：cargo build --bin q1x；cargo test（228 passed / 14 ignored）；q1x --help、update --help、--version、未知选项退 2、无子命令退 1
  - 工单：#8
- docs[multi]：argparser 入向依赖补上四个 main 入口
  - quant1x/base/argparser/argparser.md：路线图里 Python / C++ / Rust 三侧标注已接入；入向依赖表补 main.py / main.cpp / main.rs / quant1x/command
  - 对应 #7（Go）与 #8（Python / C++ / Rust）的落地状态
  - 工单：#8
- refactor[go]：tail 工具的解析由 cobra 迁到 base/argparser
  - tools/tail/main.go：去掉 cobra 与 DisableFlagParsing 的手写扫描，改用 base/argparser；-f / -q / -v 开关，-n / --lines 与 -c / --bytes 取值，file 为 OneOrMore 位置参数
  - 保留 GNU 写法：解析前把裸数字 -10 / -+10 归一成 --lines=N；-n10、-n=10、--lines=+3、-c5、--bytes=+5 由解析器原生支持
  - 同时给 --lines 与 --bytes 时报用法错误；没有文件参数时退 2（原实现打印 usage 后退 0）
  - go.mod / go.sum：移除 spf13/cobra 与随之不再需要的 mousetrap、pflag
  - 语义一致：读取 / 跟随 / 输出逻辑未改，各写法输出与系统 tail 逐项比对一致
  - 验证：go build ./...；go vet ./tools/tail/；go mod tidy -diff 无差异；tail --help 退 0、--no-such 退 2、无文件退 2；对临时文件比对 -10 / -n 2 / -n2 / -n=2 / --lines=+3 / -c 5 / --bytes=+5
  - 工单：#9
- release version 0.8.12

## [0.8.11] - 2026-11-16
### Changed
- fix[rust]: 调整rust工具链的说明
- chore[multi]：去除 VS Code 分层配置中的注释并按功能域重排
  - 删除 .vscode-default / .vscode-windows / .vscode-macos 三份 settings.json 的全部 // 注释，
    改为严格 JSON，消除 jsonc(521)“Comments are not permitted in JSON”告警
  - 按编辑器与字体 / 终端 / Python / Go / CMake / C++ 测试 / 代码运行 分组重排键序，组间保留空行；
    cmake.configureSettings 与 TestMate debug.configTemplate 内部改为公共键在前、平台覆盖在后
  - 语义一致性：与提交前剥注释后的配置逐键比对完全一致，仅键序变化；被删注释中的 Conan 安装命令
    与平台说明已在 BUILDING.md 中
  - 验证：python 逐一 json.loads 三份文件通过；无 BOM、LF 行尾、末尾换行
- feat[python+cpp+rust+go]：在 base 下落地跨语言命令行解析模块 argparser
  - 新增 quant1x/base/argparser：spec / error / parser / help / complete 五层（.py / .hpp+.cpp / .rs / .go
    同名不同后缀），Python 侧另有 derive.py 的 dataclass 派生式声明
  - C++ 源文件登记进根 CMakeLists.txt 的 quant1x-headers / quant1x-sources；Rust 在 quant1x/base/mod.rs 声明
    pub mod argparser；Go 包名随目录为 argparser
  - C++ 用例按仓库约定落到 tests/test-argparser-parser.cpp 与 tests/test-argparser-complete.cpp，并在
    tests/CMakeLists.txt 登记
  - 模块文档 quant1x/base/argparser/argparser.md 与目录 README.md 随代码落地，README 文档索引补一行
  - .gitignore 增加 __cmake_systeminformation/（`cmake --system-information` 的转储工程）
  - 语义一致性：Python 为 spec；C++ / Rust / Go 三侧的帮助版式（短帮助 / 长帮助 / 子命令帮助 / 窄幅折行 /
    错误块）与三种 shell 的补全脚本与 Python 输出逐字节一致
  - 验证：python -m quant1x.base.argparser.parser_test / help_test / complete_test / derive_test；
    go test ./quant1x/base/argparser；cargo test base::argparser（61 条通过）；
    cmake --build build-msvc --config Release --target app-test-argparser-parser app-test-argparser-complete
    后两个可执行文件退出码 0；gofmt -l、go vet、rustfmt --check 无输出
  - 工单：#1
- docs[multi]：文档增加 Zig 语言位并把工单平台切到 Codeberg
  - CONTRIBUTING：§2.1 补 Zig 0.17+ 基线，§2.3 角色表增「Zig（扩展）」并把列序改为
    python → c++ → rust → go → zig → java，§2.4 / §2.5 / §3.2 / §6 / §7 / §8.1 / §9.1 同步补位
  - AGENTS：硬约束第 1 / 12 / 16 条、交付自检与常用命令补 Zig；工单入口改为仓库自带 labs-git-issue
  - BUILDING：构建单元与版本要求表补 Zig，新增 §7「Zig（扩展）」章节，后续章节顺延
  - README：语言表述改为「四门核心 + Zig 扩展」，问题跟踪链接改到 Codeberg
  - Cargo.toml：repository / documentation 改到 codeberg.org
  - 口径：Zig 与 Go 同为扩展语言，当前只承载小功能；未落地模块在对照表留 `—`，不影响该行状态
  - 工单：#1
- feat[zig]：argparser 补齐 Zig 侧实现
  - 新增 quant1x/base/argparser/{spec,error,parser,help,complete}.zig 与三个用例
    （parser_test.zig / help_test.zig / complete_test.zig）；模块无 build.zig，用例以 zig test 直接执行
  - 语义与 Python spec 同口径：帮助版式（短 / 长 / 子命令 / 窄幅折行 / 错误块）与三种 shell 的补全脚本
    与 Python 输出逐字节一致
  - 模块文档 argparser.md 补回 zig 列与路线图行，README 文件表与运行命令同步补 zig
  - 验证：zig test 三份用例 25 / 9 / 5 条全过；zig fmt --check 无差异
  - 工单：#1
- docs[multi]：README 徽章补 Zig 0.17+
  - 与 CONTRIBUTING §2.1、BUILDING §1.1 的 Zig 版本基线保持一致
  - 工单：#1
- docs[multi]：仓库链接统一到 GitHub，Rust 基线 1.100.0、Zig 0.17
  - README / BUILDING / docs/argparse.md / Cargo.toml 的仓库地址与 Go 导入路径统一为 github.com/quant1x/quant1x，
    清掉 Gitee、Codeberg 的旧链接
  - CONTRIBUTING §9.1 与 AGENTS 第 16 条、主题索引：工单只写 labs-git-issue，不传 --host / --repo，
    平台与仓库由工具按 origin 自动识别
  - Rust 最低版本 1.100.0（rust-toolchain.toml 已锁 1.100.0），README 徽章与环境要求表同步
  - Zig 基线 0.17，README 环境要求表补 Zig 行（扩展语言，当前只承载小功能）
  - 工单：#1
- docs[multi]：概念表与命名表补 Zig
  - CONTRIBUTING §1 概念表拆出「扩展实现 Zig / Java」，§4.1 蛇形命名表与沿用句补 Zig
  - 工单：#1
- refactor[go]：移除 base/argparse，命令行解析统一到 argparser
  - 删除 quant1x/base/argparse（argparse.go / argparse_test.go）与 docs/argparse.md：该实现只覆盖
    third_party argparse.hpp 的链式 API，与四语言对位的 base/argparser 重复，仓库内无调用方
  - README 文档索引去掉「命令行参数」行；argparser 的 README / argparser.md 清理两套实现的对照说明
  - 验证：go build ./... 通过，仓库内无 base/argparse 引用
  - 工单：#2
- feat[python]：迁移 dotenv（env / read_dotenv）与 expandvars / reload
  - 新增 quant1x/base/dotenv.py：项目根 .env > 主目录 .env > 系统环境变量 > 默认值，两份逐键合并，只读不写 os.environ
  - 新增 expandvars.py（$NAME / ${NAME} 展开，引用成环时整段原样返回）与 reload.py（按存在性与 mtime 热加载）
  - filesystem 增加 find_upward / find_project_root（pyproject.toml → LICENSE → .git 向上定位），供 dotenv 复用
  - 用例 dotenv_test / expandvars_test / reload_test，模块文档 dotenv.md 随代码落地
  - 验证：python -m quant1x.base.dotenv_test / expandvars_test / reload_test 全绿
  - 工单：#2
- refactor[python]：reload 更名 hotreload，并厘清 dotenv 与运行环境的边界
  - reload.py / reload_test.py 更名 hotreload.py / hotreload_test.py，dotenv 的导入与文档引用同步
  - dotenv 模块与 dotenv.md 补边界：只管 .env 与系统环境变量；进程运行环境（入口应用定位、项目布局推演、
    当前用户）不在本模块 —— 运行环境与“环境变量”是两回事
  - 工单 #2 正文同步该口径
  - 验证：python -m quant1x.base.dotenv_test / expandvars_test / hotreload_test 全绿
  - 工单：#2
- release version 0.8.11

## [0.8.10] - 2026-11-16
### Changed
- docs[multi]：整合多语言协作规范，收敛为单一规范源
  - CONTRIBUTING.md 重写为唯一规范源（11 节）：并入语言版本与编译器基线、语言角色与演进顺序、术语口径（实现异构·语义相同）、目录与多语言布局、命名与数据契约（含领域类型词表）、代码规范（注释三类格式、base 优先、debug_ 开关、幂等、包形态与导入、头文件引用、编码与行尾、卡点抛不支持、抽象方法抛异常）、文档规范（目录 README、模块 md 六段结构、状态三值）、测试规范（六维覆盖、进程级崩溃注入、跨语言对拍）、提交规范、提案与验证工作流、工作准则
  - AGENTS.md 收敛为 AI 入口索引：开工前必读顺序、17 条硬约束（逐条指向 CONTRIBUTING.md 锚点）、主题索引、常用命令、交付前自检；不再复制细则
  - 保留本仓既有约定：实现根为 quant1x/、Go 包边界规则、Java 测试包层级、提交主题全角冒号与 cpp 语言标记、C++ 包含写法 quant1x/<模块>/<头文件>.h、feature_detection.h 平台宏
  - .githooks/text_check.py 三处规范指针改指 CONTRIBUTING.md 5.8（文件编码与行尾一节已从 AGENTS.md 迁出）
  - BUILDING.md 去掉「以 C++ 为基准」的表述，改为与 Python spec 语义一致、与 C++ 实现对位；同步 AGENTS.md / CONTRIBUTING.md 的定位描述
  - 语义一致性：规范口径统一为「Python 是 spec，C++ 是生产实现，Rust/Go 对齐，Java 可选」，四门语言命名与落盘契约规则不变
  - 验证：锚点自检脚本比对 56 个标题锚点，两份文档内部链接 0 断裂；AGENTS.md、CONTRIBUTING.md、BUILDING.md、text_check.py 均 UTF-8 无 BOM、LF 行尾；引用的标量别名与 strings API 已核对存在
- release version 0.8.10

## [0.8.9] - 2026-11-15
### Changed
- feat[go]：将加权评分迁入 learn/scoring
  - 从 foundations 的 mpr/scoring 迁入 Go 存量实现，落在 quant1x/learn/scoring，包名 scoring
  - 收录 ScoreCalculator（自定义权重 + 自动分配余量）与 RangeProcessor / ThresholdProcessor 两个折算处理器
  - 修缺陷：RangeProcessor 零宽区间不再除零、ThresholdProcessor 非正阈值返回错误、WeightDistribution 返回副本
  - 单测由 printf 打印改为断言：权重分配、错误分支、零宽区间、零/负阈值、权重分布不被外部污染，共 11 项
  - 文档：scoring.md 写公式、数据契约与边界；README.md 写职责边界、文件清单与跨语言对应（Python / C++ / Rust 未落地）
  - 验证：go test ./quant1x/learn/scoring/ 全过；go vet 与 gofmt -l 无输出；go build ./quant1x/learn/... 通过
- release version 0.8.9

## [0.8.8] - 2026-11-14
### Changed
- chore[python]：收敛 Python 打包清单，移除不可再分发的中易字体
  - 变更概述：MANIFEST.in 改为白名单式（105 行 → 48 行），sdist 只含 .py 与必需元数据；
    删除 no-op 与重复规则，用 global-exclude 覆盖包内各语言的平行实现与资源；
    pyproject 删除在 include-package-data=false 下不生效的 exclude-package-data
    与空 package-data，packages.find 去掉与 include 重复的 exclude，
    license 改为 PEP 639 的 SPDX 表达式加 license-files 白名单
  - 涉及语言与文件范围：Python 打包配置 MANIFEST.in、pyproject.toml；
    删除 quant1x/util/SimHei.ttf（SimHei v3.02 版权属中易，随 Windows 授权使用、
    无再分发权），删除全仓库无引用的 quant1x/util/font.py，
    quant1x/util/__init__.py 移除 FONT_SimHei 与 CURRENT_PATH
  - 语义一致性说明：仅改变发布产物内容与字体引用方式，不涉及任何量化算法、
    数据契约与跨语言语义；绘图仍按字体族名使用，字体由使用者自备
  - 验证方式：python -m build 全流程；sdist 182 个文件 = 169 个 .py + 13 个必需元数据，
    wheel 174 项且非 .py payload 为 0，从 sdist 重建 wheel 版本一致，导入 quant1x.util 正常
- docs[multi]：补充已移除 SimHei 字体的历史留存说明
  - 变更概述：THIRD_PARTY_NOTICES.md 新增第 5 节，说明 quant1x/util/SimHei.ttf
    （中易黑体 v3.02，无再分发权）已从当前工作树与 Python 发行包中移除，
    代码侧引用一并删除，但受已发布标签与协作成本所限仍留存于历史提交；
    同时声明任何派生版本、镜像与下游发行物不得重新引入该字体；
    第 0 节补充 Python 侧不发布字体等非 Python 资源，第 4 节检查清单增加一条
  - 涉及语言与文件范围：仅仓库级合规文档 THIRD_PARTY_NOTICES.md，
    不涉及 python/cpp/rust/go 任一语言实现与构建配置
  - 语义一致性说明：纯文档补充，不改变代码行为、数据契约与跨语言语义；
    与 MANIFEST.in 排除字体的打包行为保持一致
  - 验证方式：人工复核文档结构与措辞；确认本次仅 THIRD_PARTY_NOTICES.md 变更，
    无源码改动，无需重跑构建
- chore[python]：将第三方声明随 sdist 与 wheel 一并发布
  - 变更概述：THIRD_PARTY_NOTICES.md 加入 PEP 639 的 license-files 白名单，
    wheel 会将其落到 dist-info/licenses/ 并在 METADATA 记录 License-File；
    同时加入 MANIFEST.in 的保留清单，避免被 global-exclude *.md 剔除出 sdist
  - 涉及语言与文件范围：Python 打包配置 MANIFEST.in、pyproject.toml
  - 语义一致性说明：仅调整合规声明文件的发布范围，不影响代码行为与包内容；
    wheel 的 payload 仍只有 .py，声明文件位于 dist-info 元数据区
  - 验证方式：python -m build 全流程；sdist 根目录含 THIRD_PARTY_NOTICES.md，
    wheel 含 dist-info/licenses/THIRD_PARTY_NOTICES.md，METADATA 有
    License-File 两条，三处内容与仓库文件字节一致
- release version 0.8.8

## [0.8.7] - 2026-11-14
### Changed
- refactor[multi]：合并 id 目录并将 id64/id128 迁移到 distributed
  - 变更概述：将 quant1x/id/id64、quant1x/id/id128 迁移到
    quant1x/distributed/ 下，与 distributed/id 同级；删除仅含空
    __init__.py 的 quant1x/id；Java 测试迁移到 tests/distributed/id64、
    tests/distributed/id128
  - 包路径：Python 改为 quant1x.distributed.id64/id128；Go import 改为
    github.com/quant1x/quant1x/quant1x/distributed/id64/id128（包名不变）；
    Java package 改为 quant1x.distributed.id64/id128，测试 package 改为
    tests.distributed.id64/id128
  - Java 可见性：为支持 tests.* 包下的白盒测试，公开 FileStateStore、
    PersistentState 及测试所需方法，公开 HLC.PAYLOAD_BITS 并新增
    HLC.sequence()
  - 涉及语言与文件范围：Python/Go/Java 的 ID 模块与 Java 测试；C++/Rust 的
    quant1x/distributed/id 未修改
  - 语义一致性说明：ID 位布局、编码算法、状态文件格式与公开语义均未改变，
    仅目录、包名与 import 路径变化；旧路径属于破坏性变更
  - 验证方式：python -m unittest quant1x.distributed.id64.id_test
    quant1x.distributed.id128.id_test（17 项通过）；go test
    ./quant1x/distributed/...（id/id64/id128 全通过）；mvn -q -Dtest=IdTest
    test（17 项通过）
- docs[multi]：调整协作规范，补充需求/Issue 与 temp 规则
  - CONTRIBUTING：补充目录 README 与模块关系要求、需求与 Issue 流程、
    temp/ 使用规范，并将版本发布规则顺延为第 9 节
  - AGENTS：同步 temp/ 禁止外部写入、配套产物同目录、工具链产物例外、
    Issue 确认步骤与 README 关系要求
  - README：更新分布式 ID 文档索引到 quant1x/distributed/id64、id128
  - 语义一致性说明：仅文档调整，不修改任何语言实现、构建配置或运行时行为
  - 验证方式：git diff --check 通过
- release version 0.8.7

## [0.8.6] - 2026-11-14
### Changed
- docs[multi]：补齐 fast-cpp-csv-parser 许可证声明与第三方归属
  - 变更概述：恢复 quant1x/io/csv-reader.h 的 BSD-3 版权头与完整条款，标注
    衍生来源及 Quant1X 修改内容；为 csv-reader.md 增加归属说明；新增
    THIRD_PARTY_NOTICES.md 汇总 fast-cpp-csv-parser、third_party 组件与
    Conan 依赖的许可证；README 许可证章节补充 MIT 例外；CMake 安装规则
    增加 LICENSE、第三方声明与 third_party/LICENSE.* 的安装
  - 涉及语言与文件范围：C++ 头文件与文档、CMake 安装规则；涉及
    quant1x/io/csv-reader.h、quant1x/io/csv-reader.md、THIRD_PARTY_NOTICES.md、
    README.md、CMakeLists.txt；未修改 third_party 下任何文件
  - 语义一致性说明：仅版权声明、许可证文本与安装规则变化，解析代码、
    接口、类型命名与 I/O 契约均未改变
  - 验证方式：对 csv-reader.h 去注释后做 token 级对比，解析实现与上游
    fast-cpp-csv-parser 574a9fe 一致；CMake 配置通过，生成的安装脚本包含
    share/licenses/quant1x 与 third_party 许可证目录
- chore[rust]：限制 crates.io 发布包为 Rust 源码与必要文档
  - 变更概述：将 Cargo.toml 的 exclude 黑名单改为 include 白名单，只打包
    Cargo 元数据、README/LICENSE/THIRD_PARTY_NOTICES、main.rs、
    quant1x/**/*.rs、tools 下的两个 bin 与 benches/vyukov_bench.rs
  - 涉及语言与文件范围：Rust 打包配置，仅 Cargo.toml
  - 语义一致性说明：仅改变发布包内容，不修改任何 Rust 源码、特性开关、
    二进制/基准目标或运行时行为
  - 验证方式：cargo package --list 输出 122 个文件，除 Cargo 元数据与
    根目录文档外全部为 .rs；cargo package --no-verify 成功生成
    223 KiB 压缩包；examples/tests 等未包含文件按预期被忽略
- chore[python]：限制 PyPI 发布包为 Python 源码与打包元数据
  - 变更概述：pyproject 关闭 setuptools_scm 的自动包数据收集，显式排除
    md/java/ipynb/proto/C++/Go/Rust/Zig 源码与字体等非 Python 文件；
    MANIFEST.in 增加包内与根目录排除规则，包内仅保留 .py，根目录仅保留
    README、LICENSE、pyproject 等打包必需文件
  - 涉及语言与文件范围：Python 打包配置，涉及 pyproject.toml、MANIFEST.in
  - 语义一致性说明：仅改变 sdist/wheel 内容，不修改任何 Python 源码、
    依赖声明或导入行为
  - 验证方式：清理 quant1x.egg-info 后执行 python -m build：
    wheel 175 个文件（171 个 .py + dist-info），sdist 185 个文件
    （171 个 .py + README/LICENSE/pyproject/MANIFEST/PKG-INFO 等元数据），
    不再包含 C++/Go/Rust/Java/Zig、文档、notebook、字体或图片
- chore[multi]：删除已由 foundations 重构替代的发布脚本
  - 变更概述：删除根目录 publish.sh、publish.ps1、publish.bat；发布流程已
    迁移到 foundations 的 Python 实现，旧脚本不再维护
  - 涉及语言与文件范围：仅删除三个发布辅助脚本，不涉及源码与构建配置
  - 语义一致性说明：不改变任何语言的实现、接口、构建产物或运行时行为
  - 验证方式：全仓库检索确认除 CHANGELOG 历史记录外无 publish.* 引用；
    Python 打包配置中对应的 exclude 项已在上一提交移除
- release version 0.8.6

## [0.8.5] - 2026-11-14
### Changed
- docs[multi]：新增构建指南并补充临时文件目录约束
  - 新增 BUILDING.md，按 C/C++、Rust、Go、Python、Java 分别说明依赖、配置、
    编译、测试、产物位置与常见问题；C++ 部分覆盖 VCPKG_ROOT/MSF_RUNTIME、
    triplet 选择、protobuf 独立安装、CMake 选项、CTest/基准与安装布局
  - 在 README.md 文档索引中登记构建指南入口
  - 在 AGENTS.md 目录规则中新增约束：测试性代码与临时文件必须放入 temp/，
    路径需复刻模块层级，且不纳入版本跟踪
  - 在 .gitignore 中显式忽略 temp/，此前仅依赖本机全局忽略配置

  涉及语言与文件范围：本次仅文档与仓库配置，不涉及任何语言源码；
  文件为 BUILDING.md（新增）、README.md、AGENTS.md、.gitignore。

  语义一致性说明：纯文档与忽略规则变更，不改变 C++/Rust/Go/Python/Java
  任何实现、类型命名与 I/O 契约。

  验证方式：BUILDING.md 内容逐项对照 CMakeLists.txt、cmake/*.cmake、
  Cargo.toml、rust-toolchain.toml、go.mod、pyproject.toml、pom.xml、
  vcpkg.json 与 tests/CMakeLists.txt；git check-ignore -v temp/ 确认忽略规则
  生效。当前环境未安装 cmake/cargo/go，未执行实际编译验证。
- refactor[cpp]：C++ 依赖管理从 vcpkg 迁移到 Conan 2
  - 变更概述：新增 conanfile.py（固定依赖版本、全静态、minizip 关闭 bzip2、
    openssl 关闭 FIPS）与 conan/profiles/windows-msvc-static；删除 vcpkg.json；
    CMakeLists 移除 VCPKG_ROOT/triplet/CMAKE_PREFIX_PATH 及 zlib 库名特判，
    改为校验并消费 conan_toolchain.cmake 与 imported target；iconv/openssl
    改用 CONFIG 模式；quant1x-config.cmake 同步 Conan 包名与条件依赖
  - 编译器策略：Windows 统一 MSVC、macOS Clang、Linux/其他 Unix GCC，
    在 cmake/base.cmake 与 compile_options.cmake 中固定并在配置期校验；
    MSVC 增加 NOMINMAX，Windows 下关闭 VS 自带 vcpkg 集成，避免系统级
    vcpkg 头文件注入与 std::min/std::max 宏冲突
  - 涉及语言与文件范围：C++ 构建配置与文档；涉及 CMakeLists.txt、
    cmake/base.cmake、compile_options.cmake、iconv.cmake、openssl.cmake、
    quant1x-config.cmake.in、scripts/msvc_configure.bat、conanfile.py、
    conan/profiles/windows-msvc-static、BUILDING.md、README.md、
    .vscode-* 模板与 .gitignore；未修改 third_party 下任何文件
  - 语义一致性说明：仅替换第三方依赖的获取与查找方式，不改变 quant1x
    库的接口、类型命名、I/O 字段契约与各语言实现；protobuf 继续使用
    third_party 内置的 3.21.11 头文件与静态库
  - 依赖差异：minizip 使用 Conan Center 可用的 1.2.12（现有代码仅用
    unz* API）；cpr 沿用 Conan 默认 SSL 后端（Windows 为 winssl）
  - 验证方式：
    conan install . -of build-msvc -pr:h conan/profiles/windows-msvc-static
    -pr:b default -s build_type=Release --build=missing
    cmake -S . -B build-msvc -G "Visual Studio 17 2022" -A x64
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_TOOLCHAIN_FILE=build-msvc/conan_toolchain.cmake
    cmake --build build-msvc --config Release --target q1x
    结果：配置通过，q1x.exe 构建并运行成功；CTest 注册 63 个用例，
    catch2-tdd-iconv、catch2-tdd-yaml-deserialize、catch2-tdd-bars 均通过
- release version 0.8.5

## [0.8.4] - 2026-11-14
### Changed
- chore[rust]：移除未使用的 named_pipe 与 windows 依赖，显式声明 winapi feature
  - 变更概述：删除 Cargo.toml 中 cfg(windows) 下未被任何代码引用的 named_pipe 与
    windows 依赖；同时把 winapi 的 feature 列表补齐为代码实际引用的 12 个模块，
    新增 fileapi / namedpipeapi / winnt / minwindef / winerror。
  - 根因：这两个依赖是命名管道实现阶段遗留，最终实现直接调用 winapi（见
    quant1x/app.rs）。named_pipe 自身依赖 winapi 并开启 namedpipeapi 等 feature，
    通过 Cargo 的 feature unification 隐式补全了 Cargo.toml 未声明的 feature，
    因此直接删除会导致 winapi::um::namedpipeapi 无法解析、编译失败。
  - 涉及语言与文件范围：rust；Cargo.toml、Cargo.lock（cargo 自动更新，移除
    named_pipe 0.4.1、windows 0.48.0 及 quant1x 对二者的依赖引用）。
  - 语义一致性：仅调整依赖声明，不改变任何运行期逻辑；显式声明后 winapi 模块的
    可用集合与修改前完全一致，不存在隐式 feature 依赖。
  - 验证：cargo check、cargo check --all-targets 均通过，无 warning 无 error。
- chore[rust]：升级并锁定 Rust 工具链，补齐 rustfmt/clippy/rust-analyzer
  - 变更概述：rust-toolchain.toml 的 channel 由 1.98.1 升至 1.100.0，并显式声明
    components = ["rustfmt", "clippy", "rust-analyzer"]、profile = "minimal"；
    补充注释说明版本锁定、组件用途与 rust-analyzer 必须与编译器同版本的原因。
  - 原因：edition 2024 自 1.100.0 起获得完整支持；rust-analyzer 内置的
    proc-macro-srv 需读取宏 crate 的 metadata，版本偏差会报 invalid metadata
    version，故工具链与组件统一锁定到同一版本，保证开发、CI 与生产构建一致。
  - 涉及语言与文件范围：rust；rust-toolchain.toml。
  - 语义一致性：仅约束构建所用工具链，不改变任何语言实现的语义与产物结构。
  - 验证：cargo --version 报 1.100.0；cargo check 与 cargo check --all-targets
    在该工具链下均通过。
- release version 0.8.4

## [0.8.3] - 2026-11-08
### Changed
- fix[multi]: 修复源代码的版权信息
- release version 0.8.3

## [0.8.2] - 2026-11-03
### Changed
- fix[md]: 调整logo现实的尺寸, 居中
- release version 0.8.2

## [0.8.1] - 2026-11-03
### Changed
- fix[go]: 更新依赖库
- release version 0.8.1

## [0.8.0] - 2026-11-03
### Changed
- refactor[go]：剔除 quant1x/pkg 依赖并对齐 yaml 默认值机制
  - 变更概述：参考 engine 项目剔除 pkg 依赖的做法，将本项目对
    github.com/quant1x/pkg 的 4 处直接引用全部迁移到上游库或 gox，
    同时把配置默认值统一为 default 标签机制。全程不引入
    github.com/quant1x/data。
  - 涉及语言与文件范围（仅 Go）：
    - csv: quant1x/encoding/csv.go、quant1x/data/storage/csv.go，
      pkg/gocsv 迁移至 github.com/gocarina/gocsv
    - tablewriter: quant1x/command/command_flag.go，
      pkg/tablewriter v0 迁移至 github.com/olekukonko/tablewriter v1，
      SetHeader 改为 Header，无边框改为 rendition 配置
    - tail: tools/tail/main.go，pkg/tools/tail 迁移至 github.com/nxadm/tail
    - yaml: quant1x/core/base.go、quant1x/core/decode.go 改用
      github.com/quant1x/gox/encoding/yaml；quant1x/config 下
      trader/strategy/rule 三个参数的默认值由手写函数改为 default 标签，
      删除 defaultTraderParameter/defaultStrategyParameter/defaultRuleParameter
    - NumberRange/TradingSession 补充 UnmarshalText 与 Parse，
      使区间与交易时段可以用 default 标签及 yaml 标量配置
  - 语义一致性说明：默认值取值保持原有语义（对齐 C++ 配置默认值），
    仅改变声明位置与加载方式；yaml 中显式配置的值与显式零值优先，
    未出现的字段才取 default 标签值；依赖版本 gox 升至 v1.27.1，
    go 指令随依赖要求升至 1.27.2。
  - 验证方式：
    - go build ./... 通过
    - go test ./quant1x/core/... ./quant1x/config/... ./quant1x/encoding/... 通过
    - 新增 quant1x/config/config_yaml_test.go，覆盖显式值优先、
      显式零值不被覆盖、缺省字段取默认值、切片元素逐项填充、
      区间与时段默认值解析
- release version 0.8.0

## [0.7.85] - 2026-11-03
### Changed
- refactor[go]: 对齐日历接口
- fix[go]: 新增ETF期权数据，以观测VIX指数
- release version 0.7.85

## [0.7.84] - 2026-10-22
### Changed
- chore[multi]：为 .githooks 钩子脚本补上执行位并纳入版本管理
  - 变更概述：将 .githooks/pre-commit 与 .githooks/commit-msg 两个钩子加入版本管理，并标记为 100755 可执行模式
  - 涉及语言与文件范围：shell 钩子脚本，.githooks/pre-commit、.githooks/commit-msg
  - 语义一致性说明：本机 core.fileMode=false，Windows 文件系统不保留权限位，执行位只能记录在 git 索引中；标记为 100755 后，在 Linux/macOS 侧 clone 出来即为可执行，配合已配置的 core.hooksPath=.githooks 生效
  - 验证方式：git ls-files -s -- .githooks 显示为 100755
- feat[python]：补交 .githooks 钩子的两个校验脚本
  - 变更概述：先前只提交了 shell 钩子本体，其依赖的校验脚本未纳入版本管理，现补交
  - 涉及语言与文件范围：python，.githooks/commit_msg_check.py、.githooks/text_check.py
  - 语义一致性说明：两个脚本不带 shebang，钩子以 python "$hook_dir/xxx.py" 方式调用，故保持 100644 不设执行位
  - 验证方式：git ls-files -s -- .githooks 显示两个 .py 为 100644，两个钩子为 100755
- release version 0.7.84

## [0.7.83] - 2026-10-16
### Changed
- fix[python]: 修订authors信息，影响面是pip show显示author信息不空
- release version 0.7.83

## [0.7.82] - 2026-10-16
### Changed
- fix[python]: 调整setuptools版本未77.0.3
- fix[python]: 支持到python 3.15
- release version 0.7.82

## [0.7.81] - 2026-10-16
### Changed
- feat[python]：为网站图标套件补 64x64 尺寸
  - 变更概述：web favicon 套件此前只有 16/32/96 三档 PNG，缺少 64x64，现补齐
  - 涉及文件范围：scripts/gen_icons.py 增加 favicon-64x64.png 一档；新增产物 assets/icons/web/favicon-64x64.png
  - 说明：Windows app.ico 内嵌尺寸本已含 64，macOS 的 icon_32x32@2x.png 即 64px 文件，本次补的是网站侧的显式 64x64 PNG
  - 验证方式：python scripts/gen_icons.py 重生成成功，产物校验为 64x64 RGBA 透明底
- refactor[png]: 调整logo
- release version 0.7.81

## [0.7.80] - 2026-10-16
### Changed
- feat[python]：新增图标生成脚本并由 logo.png 派生标准 app 与网站图标
  - 变更概述：新增 scripts/gen_icons.py，由仓库根目录 logo.png 生成一套标准图标，并将产物一并入库
  - 涉及文件范围：新增 scripts/gen_icons.py 与 assets/icons/ 下 ios、android、macos、windows、web 五组共 42 个文件
  - 生成流程：先把非方形的 logo 等比缩放居中到 1024x1024 透明方形母版（内容占比 92%），再由母版按 LANCZOS 派生各尺寸，保证同一次生成内各尺寸比例一致
  - 平台口径：iOS 采用 Xcode AppIcon 命名并垫白底不透明（系统会把透明渲染为黑色）；Android 采用 mipmap 密度命名并含 Play Store 512；macOS 采用 AppIcon.appiconset 命名并含 @2x；Windows 输出 16 至 256 七尺寸合一的 ICO；Web 输出 favicon 套件与 PWA 清单 site.webmanifest
  - 依赖说明：脚本仅依赖 Pillow，无其它第三方依赖，不进入项目构建链
  - 验证方式：python scripts/gen_icons.py 执行成功，校验两个 ICO 的内嵌尺寸齐全、apple-touch-icon 为不透明 RGB、母版为 1024x1024 透明底
- chore[python]：补入图标 PNG 产物并为 assets/icons 开 .gitignore 例外
  - 变更概述：上一个提交只入库了脚本与两个 ICO，PNG 图标被 .gitignore 的全局 *.png 规则排除，现补入
  - 涉及文件范围：.gitignore；assets/icons 下 ios、android、macos、windows、web 五组共 38 个 PNG
  - .gitignore 调整：在 *.png 之后追加 !assets/icons/**/*.png，图标属交付物需入库；其余 PNG 仍按原规则忽略
  - 验证方式：git add 后 status 显示 38 个 PNG 均已暂存，工作树其余部分无改动
- release version 0.7.80

## [0.7.79] - 2026-10-13
### Changed
- fix[python]: 创业板拼写错误
- chore[git]: git仓库忽略.vscode目录
- chore[vscode]: 将windows平台的.vscode配置目录改成.vscode-windows
- chore[cpp]：按共享层与平台层拆分 VS Code 工作区配置并显式给出 vcpkg 路径
  - 变更概述：将原先全部堆在 .vscode-windows/ 的工作区配置拆为共享层与平台层，由 mopr-vscode-sync 按当前平台深合并生成 .vscode/
  - 涉及文件范围：新增 .vscode-default/settings.json 与 .vscode-default/c_cpp_properties.json；.vscode-windows/settings.json 收敛为 Windows 差异；移除 .vscode-windows/c_cpp_properties.json（跨平台一致，已在共享层）
  - 共享层承载：字体与终端/编辑器外观、python 与 go 分析项、code-runner、testMate 测试与调试模板（含 linux/darwin/win32 三分支）、cmake 构建目录与公共编译定义、vcpkg 通用开关
  - 平台层承载：vcpkg 路径显式配置，新增 cmake.configureEnvironment.VCPKG_ROOT=d:\runtime\vcpkg，并由同一根目录推导 CMAKE_TOOLCHAIN_FILE；x64-windows triplet 与 VCPKG_APPLOCAL_DEPS=ON；cmake.configureArgs 为数组整体替换故重述公共项
  - launch.json 保留在 Windows 层：pipeName 使用 \\.\pipe\ 命名管道，非跨平台
  - 语义一致性：合并产物与原 .vscode/settings.json 逐键一致，仅多出 VCPKG_ROOT，满足 CMakeLists.txt 对该环境变量的取值要求
  - 验证方式：mopr-vscode-sync 生成成功，mopr-vscode-sync --check 返回 0（无差异）
- feat[vscode]: 拆分macos下的配置
- fix[cpp]：修正 Windows 层 vcpkg triplet 为 x64-mingw-static 并显式给出 vcpkg 根目录
  - 变更概述：修正 .vscode-windows/settings.json 的 vcpkg 目标 triplet 与根目录配置
  - 涉及文件范围：仅 .vscode-windows/settings.json（Windows 平台层）
  - 原因：CMakeLists.txt 对 GNU/MinGW 取 x64-mingw-static，仅 MSVC/Clang 取 x64-windows-static；原值 x64-windows 与项目实际构建口径不符
  - 关键点：CMakeLists.txt 用 set(VCPKG_TARGET_TRIPLET ... CACHE STRING) 不带 FORCE，命令行一旦带进 -DVCPKG_TARGET_TRIPLET（含空值）即顶掉自探测，导致 VCPKG_LIBS_PATH 退化为 installed/ 根，OpenSSL 等依赖包查找失败
  - 处理：triplet 改由 vcpkg.target.defaultTriplet 与 hostTriplet 给出，值为 x64-mingw-static，不再在 cmake.configureArgs 中重复书写该 -D 项
  - vcpkg 路径：新增 vcpkg.general.vcpkgPath 指向 d:\runtime\vcpkg，配合既有的 VCPKG_ROOT 与 CMAKE_TOOLCHAIN_FILE，三处同源
  - 验证方式：mopr-vscode-sync 生成成功，mopr-vscode-sync --check 返回 0（无差异）
- fix[cpp]：vcpkg triplet 自探测改为 FORCE，避免被命令行空值覆盖
  - 变更概述：CMakeLists.txt 中按平台选择 VCPKG_TARGET_TRIPLET 的缓存写入统一加上 FORCE
  - 涉及文件范围：CMakeLists.txt 第 79、81、84 行（Windows 的 MSVC/Clang 与 MinGW/GNU 分支、Linux 分支）；macOS 分支原本已带 FORCE
  - 原因：set(... CACHE STRING ...) 不带 FORCE 时，命令行传入的 -DVCPKG_TARGET_TRIPLET（含扩展注入的空值）会顶掉自探测结果，导致 VCPKG_LIBS_PATH 退化为 installed/ 根，OpenSSL 等依赖包查找失败
  - 语义一致性说明：四个平台分支的写入方式现已一致，triplet 恒由编译器与系统判定得出，外部 -D 项不再能覆盖
  - 验证方式：重新执行 CMake 配置，观察 VCPKG_TARGET_TRIPLET 与 VCPKG_LIBS_PATH 输出指向 installed/x64-mingw-static
- refactor[cpp]：按功能域与插件重排 VS Code 分层配置并修复 Windows 层非法 JSON
  - 变更概述：对 .vscode-default / .vscode-windows / .vscode-macos 三层的 settings.json 按功能域与所属插件重排键序，并补充分区注释
  - 涉及文件范围：.vscode-default/settings.json、.vscode-windows/settings.json、.vscode-macos/settings.json
  - 分区顺序：编辑器与字体、终端、Python、Go、CMake、vcpkg、C++ 测试 (C++ TestMate)、代码运行 (Code Runner)，各层保持一致
  - 修复：.vscode-windows/settings.json 在注释掉 vcpkg triplet 后，cmake.configureSettings 的右花括号后缺少逗号导致 JSON 非法，现随重排一并修正
  - 说明：仅重排键序与注释，未增删配置项；triplet 已由 CMakeLists.txt 按编译器 FORCE 写入，故该两行保持注释状态
  - 验证方式：mopr-vscode-sync 生成成功，mopr-vscode-sync --check 返回 0（无差异）；产物键集合与重排前逐一相同
- release version 0.7.79

## [0.7.78] - 2026-09-14
### Changed
- fix[rust]: 更新rust标准版本到1.98.1
- fix[multi]: 删除废弃的文件
- fix[docs]: 优化主题logo
- docs[multi]：透明化 logo 背景并裁剪四周空白
  - 将 docs/quant1x-2.png 的灰蓝色背景（189,199,209）设为透明
  - 一并处理字母 Q、u 等被图形包围的封闭填充区域，避免残留底色
  - 按 alpha 包围盒裁掉四周空白，尺寸由 680×586 调整为 676×564
  - 边缘增加 3px 羽化，消除硬锯齿
  - 验证：以透明棋盘格底查看 docs/quant1x-2.png
- fix[image]: 调整logo图片
- docs[multi]：重组 README 文档结构
  - 新增目录导航与项目结构章节，说明顶层目录与多语言平行实现规则
  - 快速开始调整为六步：获取源码、配置文件、Go、Rust、C/C++、Python
  - 环境搭建细节下沉到附录，正文只保留结论性内容
  - 合并重复的环境要求表格，清理 AI 口吻段落与过时的版本号示例
  - 新增文档索引，汇总架构规范、数据字典与核心模块说明
  - 贡献章节补充中文提交信息规范 type[language]：summary
  - 验证：人工通读 README.md，确认章节层级与文档链接有效
- release version 0.7.78

## [0.7.77] - 2026-09-07
### Changed
- fix[rust]: 修复criterion 0.4版本的安全问题
- release version 0.7.77

## [0.7.76] - 2026-09-05
### Changed
- fix[c++]: 修订注释
- release version 0.7.76

## [0.7.75] - 2026-09-02
### Changed
- 添加 Python 原生 RingBuffer 实现与回归测试
- docs[python+multi]：补充 git 提交语言标注与 checklist 约束
- test[java]：新增 LMAX Disruptor 基准脚本与 Java 性能验证
- chore[java]：收敛 Maven Wrapper 忽略规则
- fix[java]：修正 Maven 测试包声明，确保 tests 目录下的 Java 测试可正常执行
  - 修正 tests/LMAXDisruptorBenchmark.java 的 package 声明，从 quant1x.tests 调整为 tests\n- 该改动与 pom.xml 中 testSourceDirectory=tests 的配置保持一致，避免 Java(536871240) 包路径不匹配警告\n- 语义保持不变，仅修正测试源码与 Maven 测试目录的对应关系\n- 验证：./mvnw -q -Dtest=tests.LMAXDisruptorBenchmark test
- fix[java]：修正 distributed id Java 测试包层级与规范规则
- feat[java]：实现分布式 id Java 版本并补齐状态持久化
- release version 0.7.75

## [0.7.74] - 2026-09-01
### Changed
- distributed/id: 修正 base64url 跨语言不兼容与严格模式水位回退, 修复 Go Serve 队满卡死
  三处跨语言一致性问题与缺陷修复:

  1. Rust 版 base64url 编码与 Go/Python 不兼容
     Rust 的 encode_base64url 把末字符的数据放在 6 位值的低 4 位, 而 Go 的
     base64.RawURLEncoding 与 Python 的 urlsafe_b64encode 放在高 4 位 (低 2 位补 0),
     导致同一 ID 在 Rust 与其他语言间产生的字符串不同, 无法互通.
     按 Python (Spec 锚点) 与 Go 的标准布局修正 Rust 的编码与解码, 并在 Go/Rust/C++
     三侧补充标准向量断言锁定布局, 防止回归.

  2. 严格模式下 checkpoint 未同步内存水位导致水位回退
     Go/Rust 的 checkpoint() 写入成功後未更新 latest, close() 触发的 flush() 会用
     旧水位覆盖刚写入的新 checkpoint, 造成水位回退, 严格模式重启可能重复发号.
     按 Python (Spec 锚点) 的修正方式, 在 Go/Rust 的 checkpoint() 内于写入成功后
     同步 latest, 并把 Python 侧的修正统一收敛到 checkpoint() 内, 使四种语言行为一致.

  3. Go 版 Serve 在队列满时无法响应取消 (TestGeneratorServeDrainAfterCancel 挂起)
     Serve 使用阻塞式 Push, 队列满且消费者停止时会卡在 Push 内部, 无法回到循环顶部
     检查 ctx.Done(), 导致取消信号被永久延迟, 整个测试套件挂死; 该缺陷在 Rust/C++/
     Python 版实现时已刻意规避. 本次改用非阻塞 TryPush 循环, 队列满时重试同一个 ID
     并让出时间片, 每轮检查取消与关闭, 与其余三语言语义一致.
     同时加强该用例: 改为等待队列填满后再取消以复现"生产端阻塞在满队列"场景,
     并增加 5 秒超时保护, 避免回归时挂死测试套件.

  验证: Go/Rust/Python/C++ 四语言测试全部通过, base64url 标准向量四语言输出一致.
- benches: 修复 vyukov 基准测试从未运行、吞吐高估与场景缺失
  修复 benches/vyukov_bench.rs 与 Cargo.toml 的以下缺陷:

  1. 基准从未实际运行 (致命)
     Cargo.toml 未声明 [[bench]] harness = false, Cargo 默认启用 libtest harness,
     rustc 在 --test 模式注入的 main 覆盖了 criterion_main! 生成的主函数,
     表现为 `running 0 tests` 后 0.00s 通过, 编译无错无警告且退出码为 0.
     CI 中长期绿灯但从未产生任何数据. 现已补上 harness = false 声明.

  2. 吞吐被高估 60 倍
     Throughput::Elements 硬编码 8*300000, 而实际单轮工作量为 8*5000,
     声明值与真实值脱节, 报告吞吐虚高 60 倍 (613 Melem/s vs 真实约 10 Melem/s).
     现改为由 TOTAL 常量推导, 与断言共用同一来源.

  3. 背压路径从未被测量
     队列容量 65536 大于单轮总量 40000, 队列永不满, push 从不阻塞.
     现拆分为背压 (容量 1024) 与无背压 (容量 262144) 两个场景.

  4. 线程创建开销被计入队列吞吐
     每轮 spawn/join 16 个线程, Windows 上约 1ms; 早期单轮总耗时仅约 4ms,
     该开销占比可达 25%. 现以 iter_custom 差分扣除等量空转线程的开销,
     并保留 thread_overhead 对照基准使开销可见; 单轮数据量取 16 万条,
     使扣除后剩余耗时远大于线程开销, 降低差分测量的相对误差.

  关于线程复用: 曾尝试复用生产/消费线程以彻底消除创建开销, 实测劣化且
  方差极大. 原因是常驻消费者在两轮之间持续阻塞在 pop 上, 而 pop 在队列
  空时会退避到 sleep(50us), 下一轮开始时必须先等这些睡眠中的消费者被唤醒,
  该延迟远大于线程创建开销. 该结论已写入文件注释, 避免后续重复尝试.

  当前实测 (Windows, 8 生产者 + 8 消费者, 受机器负载影响约 10~15% 波动):
  背压场景约 21 Melem/s, 无背压场景约 23 Melem/s.
- examples: 将 vyukov_demo 注册为 CTest 用例并补齐正确性校验
  examples/vyukov_demo.cpp 从未被注册到 examples/CMakeLists.txt, 因此不参与
  构建 (build 目录无任何产物), 长期处于"写了但不编译"的状态. 本次将其纳入
  构建并注册为 CTest 用例, 同时按下述要点补齐其作为测试用例的必备条件:

  - 头文件包含改用 <quant1x/runtime/ringbuffer.h>: 原先使用相对路径
    "../quant1x/runtime/ringbuffer.h", 与 tests 下同类文件不一致;
  - 增加并发语义校验: 生产总数与消费总数均须等于预期值, 否则以非 0 退出码
    结束. 原实现只打印吞吐并恒返回 0, 直接作为测试会永远通过;
  - 增加超时保护 (60 秒): 消费者以"消费总数达到预期"为退出条件, 若生产者异常
    终止, 消费者会无限自旋并挂死整个测试套件;
  - 变量统一为 constexpr 常量式命名, 计数使用 relaxed 原子操作.

  examples/CMakeLists.txt 同步清理: 移除 parse_yaml_example 上指向不存在目录
  的相对 include 路径 (该 target 的头文件路径由链接 quant1x 传递而来).

  验证:
  - ctest -R vyukov_demo 通过 (40 万条, 吞吐约 8.3 Melem/s);
  - 注入缺陷 (生产者不计数) 后用例 ***Failed 并输出期望值与实际值;
  - 注入缺陷 (生产者少生产) 后用例在超时后 3.27 秒失败退出, 未挂死;
  - 上述临时改动已还原, 文件内无残留.
- runtime: 修正 Vyukov 队列 try_ 语义并根除 C++ 异常回滚的索引倒退隐患
  代码审查发现三处问题, 本次一并修正 (C++ 与 Rust 同步, 与 Go 版既有
  架构对齐, 即 try_ 快速失败 + 阻塞退避归外层方法):

  1. try_push / try_pop 违背非阻塞契约: 原实现竞争失败时调用 backoff_spin,
     最坏路径会走到 yield 甚至 sleep (C++ 最坏 256us), 行情线程会被意外
     阻塞引入延迟尖峰. 修正后竞争失败最多以一次 CPU pause (Rust 为
     spin_loop) 吸收纳秒级瞬态并重试一次, 仍不可行立即返回 false/Err;
     热路径不再包含任何 OS 调度操作.

  2. (仅 C++) emplace 异常回滚会倒退全局入队索引, 破坏无锁算法正确性:
     CAS 成功后其他生产者可继续推进 enqueue_pos_, catch 块中的
     store(pos) 回滚会覆盖其进度, 导致后续生产者死循环或覆盖未读数据.
     修正: emplace 入口增加 static_assert 强制元素类型满足 nothrow 构造
     与 nothrow 析构, 从编译期杜绝异常路径, 并删除整个 try-catch 回滚
     逻辑及仅为它存在的 atomic_store_release_gcc. 现有调用方
     (distributed/id 的 Id, uint64_t) 均为平凡类型, 不受影响.

  3. backoff_spin 及 <thread>/<chrono> 依赖删除: try_ 改快速失败后该
     函数不再被调用; Rust 阻塞版 push/pop 的内部退避 (含 sleep) 属阻塞
     方法语义, 予以保留.

  验证:
  - g++ -O3: vyukov_demo (4P4C, 40 万条) 计数校验通过, 吞吐 7.0-7.8
    Melem/s 与修改前持平;
  - test-ringbuffer (10 采样 x 400 万) 全部通过;
  - distributed/id/queue.cpp 编译通过 (static_assert 成立);
  - cargo test: runtime::ringbuffer (basic, mpmc_small) 与
    distributed::id (16 tests) 全部通过.
- runtime: Vyukov 队列 C++ 槽位对齐显式化并消除 32 位平台移位 UB
  微调两处 (纯 C++, Rust/Go 无对应问题):

  1. alignas(64) 从成员 seq 提升到 Slot 结构体声明, 显式表达"整个槽位
     独占一个缓存行"的设计意图: 无论未来如何调整成员, 槽的起始地址与
     大小都保持 64 字节整数倍, 杜绝跨槽伪共享. 生成的布局不变 (seq 仍
     在偏移 0, sizeof 仍为 64), 且与 Rust 版 #[repr(align(64))] 的
     struct 级对齐标注跨语言一致.

  2. round_up_to_power_of_two 中 v >> 32 在 32 位 size_t 下移位计数
     大于等于类型位宽, 属未定义行为. 以 if constexpr (sizeof(size_t)
     > 4) 包裹, 64 位下编译期剪除该分支保持零开销, 32 位下合法.

  验证:
  - g++ -O3: vyukov_demo 2 轮通过 (400000/400000), 吞吐 7.36 Melem/s
    与修改前持平;
  - test-ringbuffer (10 采样 x 400 万) 全部通过;
  - clangd lint 无警告.
- 修复vcpkg.json格式的错误
- 修复 macOS 构建时找不到 zlib 库的问题
  vcpkg 在 MinGW(x64-mingw-static) 下生成的 zlib 静态库名为 libzs.a，
  而 macOS(arm64-osx) 等其他平台为 libz.a。原实现硬编码 libzs.a，
  导致 macOS 上链接 q1x 时缺失该库。现按平台区分 zlib 静态库名。
- 对齐 Rust: ringbuffer 增加阻塞式 push/pop 并修复 aarch64 退避指令
  - ringbuffer.h 新增 push/pop 阻塞式语义(与 Rust Queue::push/pop 一致),
    仅队满或已关闭且为空时返回 false, 竞争失败按 backoff_spin 四级退避重试
  - 修复 Apple Silicon 性能 bug: aarch64 CPU_PAUSE 由 yield 改为 isb
    (yield 的节流语义在 8 路 CAS 争抢下形成 convoy 正反馈, drain 阶段
    138~7700us 剧烈抖动; 改 isb 后与 Rust spin_loop 完全对齐)
  - 新增 benches/vyukov_bench.cpp (Google Benchmark) 与 benches/CMakeLists.txt,
    场景/参数与 Rust benches/vyukov_bench.rs 对齐 (8P8C, PER=20000,
    容量 1024/262144, 差分扣除线程开销, 每轮断言数据完整性)
- 文档: 更新 ringbuffer.md 记录 C++/Rust 性能对齐状态
  - 记录 aarch64 退避指令 yield->isb 的根因修复与实测效果
  - 记录已排除的候选方案 (ctor 清零 / strong+SeqCst CAS) 及原因
  - 记录剩余差距 (uncontended drain 阶段 ~25%, 微架构效应) 与基准对齐方法
- 修复 ringbuffer ctor 槽区分配与基准计数器伪影，对齐 C++/Rust 性能
  1. ringbuffer.h: ctor 改用 aligned_alloc(64) + placement new 只写 seq，消除 16MB 槽区清零，对齐 Rust Vec::with_capacity 不初始化语义（~246µs → ~185µs）

  2. vyukov_bench.cpp/rs: 基准消费者改为每线程本地计数、join 后合并，消除共享原子计数器 consumed.fetch_add 引发的复合缓存行争用伪影（8P8C 消费速率 160 vs 159M/s 完全对齐）

  3. ringbuffer.md/README.md: 同步更新性能对齐状态、强制 Clang/LLVM 复现命令与 aarch64 isb 技术注记
- 收敛跨平台对齐内存分配至 base/safe
  Windows CRT 不提供 aligned_alloc, 导致 MinGW-w64 下 std::aligned_alloc
  编译失败. 在 safe 中新增 aligned_alloc/aligned_free 统一三平台分支
  (MSVC _aligned_malloc / MinGW-w64 __mingw_aligned_malloc / 其余
  std::aligned_alloc), ringbuffer 槽区分配改用 safe 接口.
- c++: 更新依赖库robin-map版本到1.4.1
- c++: 更新croncpp版本到2026.08.12
- c++: 更新部分依赖库的版本号
- c++: 调整环境变量函数的调用
- 修复 Windows x86_64 上 Vyukov 队列比 Rust 慢数倍的问题
  根因是三级退避的第三级休眠粒度: 名义 50us, 但 MSVC 的
  std::this_thread::sleep_for 会向上取整为 Sleep(1), 受 Windows 默认
  15.6ms 定时器粒度支配, 实测单次 15.57ms; 而 Rust 的 thread::sleep(50us)
  实测 0.55ms, 相差 28 倍. 连续 8 次 CAS 竞争失败后进入该级退避, C++ 侧
  每次停摆 15.6ms, 这是多线程场景慢数倍的主因.

  该问题长期被单线程消融基准掩盖: 无争抢时不触发退避, 单线程实测
  167M ops/s 属健康值, 只有测 1P1C 到 16P16C 的扩展性曲线才能看出吞吐
  随线程数单调崩塌的特征.

  修复:
  - 新增 safe::sleep_for_microseconds(), Windows 上改用带
    CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 标志的可等待定时器(实测 0.53ms,
    与 Rust 一致), 其余平台退化为 std::this_thread::sleep_for.
    windows.h 仅在 safe.cpp 内包含, 避免其宏泄漏到所有 safe.h 使用方;
    该标志缺失时编译期自动退化, 语义不变.
  - ringbuffer.h 的 backoff_spin 第三级改用它.

  顺带修复两处真实缺陷:
  - MSVC 下曾以 _InterlockedCompareExchange64(p, 0, 0) 实现 relaxed 读游标,
    该内建是带 lock 前缀的读改写, 而 Rust 的 load(Relaxed) 只是一条 mov.
    cl /O2 汇编实测 push 热循环由 2 条 lock cmpxchg(读游标与 CAS) 变为
    1 条 mov 加 1 条 lock cmpxchg, 与 rustc/clang-cl 一致.
  - closed_ 未做缓存行对齐, 落在 dequeue_pos_ 同一行内, close() 的一次
    release 写会使 8 路消费者争抢的 dequeue_pos_ 整行失效; 补充
    alignas(64) 以对齐 Rust 侧 AlignedAtomicUsize 的布局.
  - 移除随之不再需要的 <chrono> 包含.

  效果(i7-12700T / Windows 11 / cl /O2, 环形容量 65536, 单轮 320 万条),
  8P8C 由 15.6M/s 提升到 75.0M/s(4.8 倍), 16P16C 由 8.5M/s 提升到
  68.4M/s; 修复后曲线与 Rust 同样平坦, 三轮交替复测两者落在同一噪声带内.
- 新增 Windows 侧 Vyukov 队列性能诊断工具(探针与构建脚本)
  本次性能问题的定位过程暴露出原有工具的空缺: 单线程消融基准测不出缓存行
  争用与退避停摆, 而那正是根因所在. 补上三个分工明确的探针, 使同类问题
  今后可被直接观测, 而不是靠猜测编译器后端.

  新增探针(benches/):
  - ringbuffer_sched_cost_probe.cpp: 测量 sleep_for / yield / Sleep(1) /
    可等待定时器的真实耗时. 这是定位休眠粒度问题的关键工具, 也用于回归
    验证 safe::sleep_for_microseconds 是否仍生效(若回落到 15ms 量级即说明
    高精度定时器在当前系统不可用, 队列退避会回归慢路径).
  - ringbuffer_mpmc_probe.cpp: 8P8C 争抢探针, 分 backpressure / uncontended
    两档并输出构造/生产/排空三阶段耗时. 固化了"扩展性曲线"这一测量方法 ——
    吞吐随线程数单调下降是争抢类问题的特征信号.
  - ringbuffer_single_thread_ablation.cpp: 单线程消融, 并在文件头明确标注其
    定位局限(测不出争用与退避), 避免后续再次误用它判断多线程性能.
    两处场景均保证队列不会队满: 一旦队满 push 会快速返回 false, 剩余迭代
    退化为空转, 吞吐会虚高一个量级(初版曾因此测出无意义的 387M/s).

  配套脚本(scripts/):
  - msvc_sched_cost_probe.bat / msvc_mpmc_probe.bat / msvc_mini_bench.bat
    均用 cl 与 clang-cl 双编译器构建, 便于区分"编译器后端差异"与"实现缺陷".
  - msvc_configure.bat: 本机 CMake 配置辅助(含机器专属路径, 已加注释说明).

  三个探针均不依赖 Google Benchmark, 单文件可直接编译, 因此不接入
  benches/CMakeLists.txt, 以免拖慢常规构建.
- 修正 MSVC 下 Release 构建被 -Os 覆盖为体积优化的问题
  编译选项检测里 cl.exe 同样"接受" -Os 语法, 但其语义是 MSVC 的 /Os(优化
  体积), 且 /O 组选项互斥、最后一个生效 —— 于是追加的 -Os 会覆盖前面的 /O2,
  使 Release 构建的热路径性能崩盘.

  之前该分支对所有编译器一视同仁地追加 -Os, 仅在两端都报告支持时生效;
  现为 MSVC 排除该分支, 保持 /O2 的速度优化语义.
- release version 0.7.74

## [0.7.73] - 2026-08-31
### Changed
- feat(distributed): 新增 distributed/id 发号器与 Serve 队列流水线
  - 新增 64 位可排序分布式 ID：HLC 单调推进、时钟回拨保护、动态节点位宽、base64url 编码
  - 状态文件为定长 128B mmap 双槽 checkpoint（generation + CRC 选最新有效槽），旧追加式日志自动迁移
  - 跨进程互斥采用共享映射内锁字（PID + 时间戳 CAS），持锁进程死亡立即抢占，无需锁文件
  - 新增 Generator.Serve 生产循环接入 Vyukov MPMC 无锁队列，消费端 TryPop 取号无锁化
  - 修复 Flush 与严格模式 checkpoint 的并发竞态、Close 在刷盘失败时的句柄泄漏；统一错误前缀为 distributed/id
  - README 补充实测基准数据（含每秒速率换算与读法说明）、Serve 用法与选型建议
- 修复log包的日志级别丢失、压缩错误处理与并发安全问题
  - fatal core 的 LevelEnabler 改为接收 DPanic/Panic/Fatal, 避免三个级别日志被静默丢弃
  - compressOldLogs 显式检查 os.Create/io.Copy/gzWriter.Close 错误, 失败时清理不完整 .gz 文件
  - getLogger/NewTextLoggerWithCompression/InitLogger 由 panic 改为返回 error, init 失败仅输出 stderr
  - InitLogger 创建日志目录(base.MkDirs), 与 Python/C++ 实现行为对齐
  - 重复调用 InitLogger 时先停止并清理旧缓冲写入器, 避免文件句柄泄漏
  - waitForStop 先输出退出日志再停止写入器, 避免退出日志丢失
  - 全局 cfg/logger/bws 读写统一加互斥锁保护
  - 包级日志入口判空防 nil panic, 修正 FlushInterval 类型语义
  - getApplicationName 改用 TrimSuffix 去除扩展名, 兼容多点文件名
  - 删除拼写错误的 logger_wapper.go, 新建 logger_wrapper.go
  - 重写 logger_test.go 为正确调用方式
- runtime: ringbuffer 增加非阻塞 try_push/try_pop 与 len/cap/is_empty 查询方法
- distributed: 新增分布式 ID 生成器, 含 HLC 混合逻辑时钟与 mmap 状态文件持久化
- distributed: 新增 Python 版分布式 ID 生成器, 与 Go/Rust 语义对齐
  - id.py: ID 类型与位布局 (41 位 physical + 22 位 payload, 大端 8 字节)
  - hlc.py: HLC 混合逻辑时钟, 回拨单调, seq 溢出物理 +1
  - state_store.py: mmap 双槽 checkpoint + 锁字跨进程互斥 + 旧版 18B 日志迁移
  - queue.py: 有界 MPMC 队列, 非阻塞 try_push/try_pop
  - generator.py: 单例 Generator 与响应取消的 serve 循环
  - tests.py: 14 个用例, 对齐 Go id_test.go / Rust tests.rs 语义
  - 修正 strict 模式 latest 回退缺陷 (Go/Rust 参考实现潜在问题, Python 作为 Spec 锚点修正)
- distributed/id: 增加 C++ 版分布式 ID 实现
  新增 quant1x/distributed/id 的 C++ 实现 (命名空间 quant1x::distributed::id),
  与 Go/Python/Rust 三版二进制与语义对齐, 共 7 个模块:

  - crc32: CRC32-IEEE 查表法, 与 Go hash/crc32 ChecksumIEEE 输出一致
  - error: 12 变体错误分类与 Result 包装, 不使用异常做控制流
  - id: 64 位 ID 类型, 大端 8 字节与 11 字符 base64url 无填充编码
  - state_store: 128 字节定长 mmap 状态文件, 双槽 checkpoint 加跨进程锁字,
    支持旧版 18 字节追加式记录迁移
  - hlc: 混合逻辑时钟与构建器, 对应 Go 的 Option 函数式配置
  - queue: 复用 runtime::ringbuffer 的 Vyukov MPMC 队列
  - generator: 发号器与 serve 生产循环, 响应取消与队列关闭

  编码对齐说明: Rust 版 base64url 的末字符把数据放在低 4 位, 与 Go/Python
  的标准 RawURLEncoding (高 4 位) 不兼容; 按 Python (Spec) 与 Go 的标准实现,
  C++ 对齐后者, 已在代码注释标注, 建议后续修正 Rust 版.

  缺陷修正: 严格模式下 checkpoint 未同步 latest, 导致 close() 的 flush()
  用旧水位覆盖新 checkpoint (重启可能重复 ID), 该问题在 Go/Rust 参考实现中
  同样存在, 本次按 Python (Spec 锚点) 的修正方式同步处理.

  基建调整:
  - runtime/ringbuffer.h: 补充 len/is_empty 查询方法, 对齐 Rust runtime::Queue
  - base/mmap.h: 头内函数补充 inline 以消除多翻译单元重复符号;
    malloc 后显式清零并初始化句柄字段, 修复未初始化句柄告警;
    补充缺失的 string.h 与 stdexcept 依赖

  测试: 新增 tests/tdd-distributed-id.cpp, 覆盖位字段解析、编码往返、
  时钟回拨单调性、节点位宽推导、并发唯一性、状态文件跨重启恢复与迁移,
  以及 serve 的生产消费链路; 并已与 Python 版做过状态文件双向读写验证.
- release version 0.7.73

## [0.7.72] - 2026-08-27
### Changed
- 优化 Go Vyukov 环形缓冲区
  对齐 C++ sequence-based MPMC 算法，隔离生产者和消费者缓存行，优化槽位布局与竞争退避，并补充非阻塞操作和边界测试。
- release version 0.7.72

## [0.7.71] - 2026-08-27
### Changed
- 补充 Rust Region 字符串解析
  为 Region 实现 FromStr 和解析错误类型，支持合法区域代码解析并增加非法输入测试。
- release version 0.7.71

## [0.7.70] - 2026-08-27
### Changed
- 修复 mmap 跨平台实现并更新 id64 checkpoint
  新增固定大小 mmap 文件封装与双槽位 checkpoint，修复 POSIX 构建标签、Bytes 接口和映射长度校验，更新 id64 文档与测试。
- 调整 id64 标识文件命名
  将 id64.go 重命名为 id.go，并保留标识类型实现。
- go: 调整测试输出的文件扩展名为out, 不保存在代码仓库
- release version 0.7.70

## [0.7.69] - 2026-08-26
### Changed
- feat(id): 状态文件批量缓冲优化并新增 id64 多语言实现
  - id128 快速路径改为批量缓冲：热路径纯内存推进，攒满 syncEvery 条才落盘，吞吐提升约 200 倍；新增 Close()/close() 优雅刷盘接口（Go/Python/Java）
  - 新增 id64 多语言实现（Go/Python/Java）：快速路径批量缓冲 + 严格模式句柄缓存，严格模式每轮系统调用由 8 次降至 5 次、零分配编码，吞吐提升约 17 倍
  - 补齐 Go/Python/Java 测试与 README 文档
- release version 0.7.69

## [0.7.68] - 2026-08-26
### Changed
- id 模块代码下沉一级至 id128 子目录，Go/Java/Python 包名同步调整
  - quant1x/id/ 下全部代码移动到 quant1x/id/id128/（git mv 保留历史）
  - Go：package id → id128（含测试文件）
  - Java：package quant1x.id → quant1x.id.id128，测试移动至 tests/id/id128/IdTest.java
  - Python：新增子包 quant1x.id.id128，父包 quant1x/id/__init__.py 保留并转发到子包
  - README：import 路径、测试命令、目录结构同步更新
- python: 修复文件头部版权信息
- release version 0.7.68

## [0.7.67] - 2026-08-26
### Changed
- 改名：id 模块包名由 hlcid 统一为 id，测试文件命名与落位规范化
  将 hlcid 更名为 id（混合型分布式 ID）：统一各语言错误前缀、状态文件名、注释与 README 示例；测试源文件统一命名；pom.xml 测试源根改为仓库级 tests 目录并通用排除 tests 下所有 Java 文件。
- 构建：pom.xml 改用 include 白名单编译 Java 源，测试根统一为 tests/，Java 输出目录移出 bin/
  - pom.xml 主源码根保持仓库根（sourceDirectory=.），仅编译 quant1x/**/*.java，
    不依赖 excludes，未来新增功能模块自动纳入，无需再改 pom
  - 测试源码根固定为 tests/（testSourceDirectory=tests），不再绑定单一模块目录
  - 移除 build-helper-maven-plugin 与 excludes 方案
  - .vscode/settings.json 将 Java 输出目录改为 target/classes，
    避免 VS Code Java 扩展默认生成仓库根 bin/ 目录
  - 清理已产生的 bin/ 编译产物
- java: 调整编译临时目录
- 构建：Maven 构建目录独立为 build/java，避免 mvn clean 误删 Rust 的 target/ 产物
  - pom.xml 设置 <directory>build/java</directory>，mvn clean 只删该目录，
    target/ 完全留给 Cargo
  - surefire 的 LD_LIBRARY_PATH / DYLD_LIBRARY_PATH 同步改为 build/java/classes
- release version 0.7.67

## [0.7.66] - 2026-08-26
### Changed
- 更新rust版本到1.98.0
- release version 0.7.66

## [0.7.65] - 2026-08-26
### Changed
- 新增 Maven Wrapper 并修正测试源码目录
  - 新增 mvnw / mvnw.cmd 脚本，VSCode 终端无需依赖全局 PATH 即可使用 Maven，首次运行自动下载 Maven 3.9.16 到 .mvn/wrapper/
  - mvnw 脚本标记可执行权限（100755），保证 Mac/Linux 下可直接运行
  - .gitignore 忽略 .mvn/wrapper 下的下载缓存，避免入库
  - pom.xml 修正 testSourceDirectory 为 tests/core/hlcid
- release version 0.7.65

## [0.7.64] - 2026-08-26
### Changed
- 新增分布式id 128bit版本
- release version 0.7.64

## [0.7.63] - 2026-08-26
### Changed
- 修复 std 包迁移与 KLine 到 Bar 类型迁移后遗留的编译错误
  - std.* 引用统一迁移到 base 包(CloseQuietly/NewException/ExpandUser 等)
  - KLine/KLineRaw/XdxrInfo/CumulativeAdjustment 类型迁移到 schema 与 tdx 包
  - data 包补齐 UpdateWithAdapters/DetectSymbol/DetectInstrumentTypeByRule 及 re-export
  - config 恢复 GetXdxrFilename/GetBarFilename/GetBarFilenameEx, 对齐 C++ subpath 截断
  - tdx 包新增 LoadXdxr 导出, tdxproto 新增 ResetSeqId
  - core.GetBasePath 优先读取 QUANT1X_HOME 环境变量
  - 补齐 data/tdx/factors/command 等包的编译错误, go build 与测试编译全部通过
- release version 0.7.63

## [0.7.62] - 2026-08-13
### Changed
- python: 调整包路径
- python: 调整功能模块为regime
- release v0.7.62

## [0.7.61] - 2026-08-12
### Changed
- rust: 修改最低版本为1.97.1
- python: 新增定时任务依赖库
- c++: 适配arm架构
- c++: vcpkg适配arm架构
- 修复 macOS arm64 (Apple Silicon) 编译兼容性
  - cmake/simd.cmake: arm64 分支去掉 -mavx/-mavx2，保留 -march=native
  - CMakeLists.txt: VCPKG_TARGET_TRIPLET 设置加 FORCE 覆盖缓存；unofficial-minizip 版本变量判空兜底
  - quant1x/runtime/ringbuffer.h: arm64 用 yield 指令替代 x86 _mm_pause()
  - quant1x/proto/snapshot.capnp.h: 用 Cap'n Proto 1.5.0 重新生成
  - tests/tdd-rolling.cpp: AVX2 代码用 #ifdef __AVX2__ 守卫
  - tests/tdd-datasets.cpp: SSE/AVX 内联函数用 #ifdef 守卫
- c++: 升级ASIO版本到1.38.2
- c++: 升级BS::thread_pool版本到5.1.0
- c++: 升级xsimd到14.3.0
- c++: 更新date库
- c++: 调整编译配置
- c++: 调整依赖库版本
- python: Regime Switching 的多因子量化框架
- release v0.7.61

## [0.7.60] - 2026-07-28
### Changed
- docs: 修复 README MD025 多顶级标题告警
  将 5 个 # N. 章节标题及全部子标题逐级降一级, 全文仅保留一个 H1
- release v0.7.60

## [0.7.59] - 2026-07-28
### Changed
- docs: 将 README 2.1.6 pip 源配置改为 pip config 命令
- docs: 修复 README MD033/MD041/markdownlint 告警
  - 将 <img> 内联 HTML 改为 Markdown 图片语法, 修复 MD033
  - 将顶级标题移至文件首行, 修复 MD041/MD022/MD031
  - 全文标题及代码块前后补空行
- release v0.7.59

## [0.7.58] - 2026-07-27
### Changed
- 新增主要交易所的公开授权费用表
- 调整go版本
- rust: 调整默认元数据路径
- fix(calendar): 修复 last_trading_day upper_bound 语义及测试环境变量污染
  - last_trading_day 对齐 C++ std::upper_bound: Ok(idx) 不再直接返回,
    统一走 idx+1 路径确保盘前判断始终生效
  - 新增 Test 3: exact match + 盘前 current 边界用例
  - test_lazy_load_calendar_from_file 不再修改 HOME/QUANT1X_HOME
    进程环境变量, 避免并行测试竞态污染 test_expand_user / test_last_trading_day
- release v0.7.58

## [0.7.57] - 2026-07-14
### Changed
- 提交：base(bits)：添加 bits.h 及最高位函数测试；更新 tests/CMakeLists
- bits - 统一为原生实现，跨语言语义对齐，更新测试
- python: 调整round_up_to_power_of_two代码
- bits(Python)：使用 bit_length 优化，添加类型检查与单元测试
- bits(Python)：拒绝 bool 输入，完善类型检查测试
- bits(python): 优化部分注释
- release v0.7.57

## [0.7.56] - 2026-07-11
### Changed
- 更新schema文档
- refactor: rename std to base across all four languages
  - C++: rename base.h -> common.h, update all include directives
  - Go: rename package std -> package base (12 files)
  - Rust: rename crate::std:: -> crate::base:: (18 files)
  - Python: rename from quant1x.std -> from quant1x.base (16 files)
  - CMakeLists.txt: update precompiled header path to common.h
- release v0.7.56

## [0.7.55] - 2026-06-30
### Changed
- fix(rust): fix remaining kline->bar renames in datasource.rs and f10.rs
- release v0.7.55

## [0.7.54] - 2026-06-30
### Changed
- refactor: rename kline to bar across all languages (C++, Rust, Go, Python)
  - Rename tdd-klines.cpp to tdd-bars.cpp (test file)
  - Update fetch_kline_raw -> fetch_bar_raw in bar.cpp
  - Fix type names, function names, comments, and documentation
  - All four languages share the same Bar/BarRaw nomenclature
- release v0.7.54

## [0.7.53] - 2026-06-30
### Changed
- 调整K线采样频率的枚举名
- release v0.7.53

## [0.7.52] - 2026-06-30
### Changed
- docs: 在 CONTRIBUTING.md 和 AGENTS.md 中增加版本与发布规则
  - CONTRIBUTING.md 新增第 7 节：版本号由 Git Tag 唯一决定，四种语言共享
  - AGENTS.md 新增第九节：AI Agent 严禁触碰版本号、Git Tag、CHANGELOG、发布脚本
  - autochangelog 是唯一合法发布入口
- release v0.7.52

## [0.7.51] - 2026-06-30
### Changed
- 统一消息序列号函数签名
- refactor: remove data/kline.h, merge into schema/bar.h
  - Delete quant1x/data/kline.h
  - Move operator<<(ostream&, const Bar&) and KLine alias into schema/bar.h
  - Keep backward-compat alias quant1x::data::KLine → schema::Bar
  - Update all #include references from kline.h to schema/bar.h
  - Affected files: backtest.h, position.h, tdd-chips, tdd-dataframe-data, tdd-xdxr, tdd-patterns-wave-release
- fix: 修复 init lambda 重复注册 TDX 适配器导致崩溃
  - 移除 main.cpp init lambda 中重复的 data::Register() 调用
  - init_datasource() 已统一注册 DataXdxr、DataKLineRaw、DataKLine、DataTrans、DataMinute、DataMinuteKLine
  - 重复注册触发 adapter::Register() 的 ErrAlreadyExists 异常, 导致未捕获崩溃
- Merge branch 'fix-package' into 0.7.x
- 调整git仓库忽略项设置
- fix: 修复C++编译错误
  - filesystem.cpp: 在manual_clock_cast前添加<chrono> include，公共区域也添加<chrono>
  - bar_raw.cpp: 解决namespace io重定义冲突(重命名为csvio)，删除未使用的element_count/reply_size变量
  - bar.cpp: 删除未使用的element_count/reply_size变量
  - sector.cpp: 为未使用的operator<<添加[[maybe_unused]]属性
- fix: 为未使用的const变量添加[[maybe_unused]]属性
  - tdd-f10-safety-score.cpp: defaultSafetyScoreOfIgnore
  - tdd-snapshot.cpp: tdd_capnp_cache_size
- fix: 修复test-iconv.cpp在macOS上的编译错误
  - <malloc.h> -> <stdlib.h> (macOS无此头文件)
  - <windows.h> 用 #ifdef _WIN32 包裹
- fix: test-iconv.cpp中Windows API调用用#ifdef _WIN32包裹
- release v0.7.51
- docs: 细化 AGENTS.md 命名规范，新增国际量化金融标准术语参考体系
  - 新增 §3.0 命名参考体系（Type Vocabulary）：13 项核心类型标准名称表
  - 明确命名以 Bloomberg/QuantLib/Backtrader 等国际平台术语为基准
  - 拒绝拼音、音译、本地化翻译（如 KLine → Bar）
  - 详解 Bar vs KLine：Bar 是国际标准，KLine 是中文音译遗留
  - 新增命名验收标准三步校验
  - 补充文件命名规则：data/schema/ 下以领域概念命名
  - 修正 §3.2 示例从 Tick 改为 Bar
- release v0.7.51

## [0.7.50] - 2026-06-22
### Changed
- feat(ticker_rules): add Go implementation of all market ticker rules
  Implement rule.go (RulePrefix, CodeRule, MatchRule, GlobalRules) and market rules for BSE, SSE, SZSE, HKEX, USA including HK price tick and US code mapping.
- release v0.7.50

## [0.7.49] - 2026-06-22
### Changed
- feat(storage): add C++, Rust, Go storage implementations based on Python design
  - Add C++ FileStorage<T>, BasedataFileStorage<T>, MetaFileStorage<T> in storage.h
  - Add Rust FileStorage trait, BasedataFileStorage, MetaFileStorage in storage.rs
  - Add Go FileStorage interface, BasedataFileStorage, MetaFileStorage in storage.go
  - Add storage.md design document
  - Register storage module in data/mod.rs
- release v0.7.49

## [0.7.48] - 2026-06-22
### Changed
- 添加许可证头部
- c++: 调整入口源文件名为app
- c++: datasets收敛到app
- fix: API迁移 - 恢复CMakeLists.txt中禁用的源文件并修复旧版API调用
  - CMakeLists.txt: 取消注释约12个TODO标记的源文件
  - detect_symbol: 3元组返回值迁移为Instrument结构体
  - correct_security_code: 双参数迁移为单参数
  - date_range -> get_date_range
  - ExchangeId -> Exchange枚举直接使用
  - instruments命名空间函数迁移
  - cpr::Parameters使用双花括号语法
  - 修复缺失的#include和namespace别名
  - 修复Owner()缺少const限定符
  - 清理build临时日志文件
- release v0.7.48

## [0.7.47] - 2026-06-21
### Changed
- c++: 删除废弃的头文件
- rust: 清理冗余的依赖库
- release v0.7.47

## [0.7.46] - 2026-06-21
### Changed
- python: 更新测试笔记
- release v0.7.46

## [0.7.45] - 2026-06-21
### Changed
- c++: 删除废弃的示例代码
- release v0.7.45

## [0.7.44] - 2026-06-21
### Changed
- go: 初步与新的数据框架对齐
- 修复 buffer.h 的未定义行为及潜在缺陷
  - 添加 <limits>/<array>/<string>/<cassert> 头文件，消除隐式依赖
  - push_arithmetic/get_arithmetic 增加 static_assert 阻止 long double（防止栈溢出）
  - get_le 用 memcpy 替代 static_cast 避免有符号转换实现定义行为
  - get_string 用 std::memchr 替代 strnlen（POSIX）解决跨平台问题
  - varint_decode 中 shift 类型改为 uint32_t 避免位宽假设
  - seek/skip 添加 assert 边界检查
  - ensure_capacity 替换为 prepare_write 增加 overflow 检查，防止写入越界
  - 修复 get_array 返回 std::array 时的重载匹配编译错误
- release v0.7.44

## [0.7.43] - 2026-06-21
### Changed
- 删除过期的演示代码
- release v0.7.43

## [0.7.42] - 2026-06-21
### Changed
- 补充开发规范缺失的目录结构示意图
- release v0.7.42

## [0.7.41] - 2026-06-20
### Changed
- fix: 修复 security_list_test 编译和链接错误
  - data.ExchangeBSE -> data.BSE (修正不存在常量)
  - 移除 _ = data.DataHandler() (go:linkname 依赖 tdx 包导致链接失败)
  - bar.go: 移除 gitee.com 外部依赖, exchange.Timestamp -> data.Timestamp
  - 测试改为自包含连接提供者, 直接拨号+握手, 不依赖 tdx 包 init()
- release v0.7.41

## [0.7.40] - 2026-06-20
### Changed
- 统一调整KLine命名为Bar
- 新增开发规范
- release v0.7.40

## [0.7.39] - 2026-06-20
### Changed
- c++: 剔除对旧版本kline的依赖
- release v0.7.39

## [0.7.38] - 2026-06-20
### Changed
- debug: add trace logging for sh880915 昨日突涨 block to find where it drops
- fix: parse_block_raw_data scan all 400 code slots instead of trusting num field; remove debug logs
- release v0.7.38

## [0.7.37] - 2026-06-20
### Changed
- fix_last_trading_day_with_debug_timestamp_parameter
- feat: implement Print methods for all data adapters; add blocks comparison tool
- release v0.7.37

## [0.7.36] - 2026-06-19
### Changed
- c++修订注释
- 统一消息名称签名格式
- release v0.7.36

## [0.7.35] - 2026-06-18
### Changed
- 修复漏掉的rust版本说明
- release v0.7.35

## [0.7.34] - 2026-06-18
### Changed
- 更新rust最低版本的说明, 确定1.96+
- release v0.7.34

## [0.7.33] - 2026-06-18
### Changed
- 修复autochangelog不能通过不更新cargo.lock的问题
- release v0.7.33

## [0.7.32] - 2026-06-18
### Changed
- fix: align publish.sh with publish.ps1 and fix bash regex version extraction
  - 统一构建工具: python setup.py → python -m build
  - 版本号从构建产物文件名解析，修正贪婪 (.+) → ([^-]+) 避免 PEP 427 wheel 文件名误匹配
  - 参数对齐: 移除 --dry-run/--repository，新增 --no-rich/--allow-existing
  - 版本重复检查改为阻断型 (exit 2)，--allow-existing 可强制继续
  - 新增 PYTHONIOENCODING/TWINE_NON_INTERACTIVE 环境变量
  - 新增 build 模块预检
- release v0.7.32

## [0.7.31] - 2026-06-18
### Changed
- refactor: rename namespace config to quant1x::config
  - 将所有 quant1x/config/ 目录下的 namespace config 改为 namespace quant1x::config
  - 更新所有外部文件的 namespace 别名: namespace config = ::config; -> namespace config = quant1x::config;
  - 更新 YAML convert 模板特化中的 config:: 引用
  - 涉及 33 个文件，编译通过
- release v0.7.31

## [0.7.30] - 2026-06-18
### Changed
- refactor(calendar): align lazy_load_calendar with Python impl - check marker file expiry before update
- 调整消息头字段名
- fix: namespace resolution and compilation errors across tdx modules
- release v0.7.30

## [0.7.29] - 2026-06-17
### Changed
- 调整全角符号为半角, 调整io部分文件类功能归于std模块
- release v0.7.29

## [0.7.28] - 2026-06-17
### Changed
- refactor(cpp): align cache paths with Rust/Python, add network fetch for stub adapters
  - Use inst.cache_dir() (e.g. sse/szse) in cache paths for kline_raw, xdxr, kline, trans, minute, kline_minute
  - kline_raw: day_raw/{cache_dir}/{symbol}.raw
  - xdxr: xdxr/{cache_dir}/{symbol}.csv
  - kline: day/{cache_dir}/{symbol}.csv
  - trans: trans/{cache_dir}/{year}/{yyyymmdd}/{symbol}.csv
  - minute: minute/{cache_dir}/{symbol}.csv
  - kline_minute: kline_minute/{cache_dir}/{symbol}.csv
  - Implement actual network fetch + save logic for kline, trans, minute, kline_minute adapters
- refactor(cpp): move protocol.h/cpp and helpers.h from level1/ to tdx/ to align with Rust/Python
  - Moved quant1x/contrib/data/tdx/level1/{protocol.h, protocol.cpp, helpers.h} -> quant1x/contrib/data/tdx/
  - Updated all 19 include references across client.h, instruments.cpp, tests, and 16 level1 headers
  - Fixed double-slash paths in xdxr_info.h, transaction_data.h
  - Updated CMakeLists.txt source paths and added helpers.h to header list
  - Updated helpers.rs comment to reflect new path
- refactor(cpp): move forward-adjustment logic from factors/ into tdx/kline, align DataKLine::Update with Python
  kline.h/cpp:
  - Full DataKLine::Update() flow matching Python: cache load -> date range -> batch fetch -> merge -> adjust -> save
  - apply_forward_adjustment_for_event() for incremental updates
  - apply_forward_adjustments_once() / calculate_pre_adjust() for general use
  - combine_adjustments_in_period() using meta::schema::CumulativeAdjustment
  - CSV I/O: read_kline_from_csv, save_kline, load_kline, get_kline_filename
  - get_xdxr_list / ipo_date_from_xdxrs reading from xdxr cache

  factors/base.h/cpp:
  - Remove all adjustment types/functions (moved to tdx::kline)
  - checkout_klines / klines_forward_adjusted_to_date now delegate to tdx:: functions
  - Keep feature constants and backward-compatible wrapper API
- refactor(cpp): align kline_raw with Python semantics, add ext protocol support, remove old base factors
  - kline_raw: fetch_kline_raw returns domain Bar (not protocol SecurityBar), internal SecurityBar->Bar conversion
  - level1: add ext_sync.h (ExtSynchronize, cmd 0x2454) and instrument_bars.h (InstrumentBars, cmd 0x23FF) for ext K-line protocol
  - client: add ExtensionProtocolHandler with get_ext_conn() for ext connection pool
  - fetch_kline_raw_from_ext: replace TODO stub with full ext protocol implementation
  - kline: remove redundant SecurityBar->Bar conversion, directly consume domain Bar
  - factors: delete old base.cpp/h, update base_compat.h/f10.cpp/history.cpp
  - tests: update tdd-* tests, user: update no0/strategy-no0
  - gitignore: add build_output.txt
- 删除cmake构建临时文件
- release v0.7.28

## [0.7.27] - 2026-06-16
### Changed
- python: 删除旧版本的代码检测
- release v0.7.27

## [0.7.26] - 2026-06-16
### Changed
- fix: compile error in connection_pool and varint_decode bug, ignore env-dependent tests
- git仓库忽略codebuddy的临时目录
- refactor: migrate exchange namespace to meta, adapter Update/Print to Instrument ref
  - Move exchange-related types under data::meta namespace
  - Change DataAdapter/FeatureAdapter Update/Print from string code to Instrument& inst
  - Update all 12 adapter subclasses, extract code via inst.symbol() internally
  - Fix callers (cache.cpp, tests) to pass Instrument via data::detect_symbol()
  - Update CMakeLists.txt source paths accordingly
- release v0.7.26

## [0.7.25] - 2026-06-15
### Changed
- refactor: align RequestHeader/ResponseHeader field names with Python base message
  - zip_flag -> frame_type (align with Python RequestHeader.frame_type)

  - packet_type -> packet_flag (align with Python ResponseHeader.packet_flag)

  - update all submodule references in heartbeat/minute_time/security_bars/finance_info
- release v0.7.25

## [0.7.24] - 2026-06-15
### Changed
- fix: extension server probe now uses ExtensionProtocolHandler handshake
- release v0.7.24

## [0.7.23] - 2026-06-15
### Changed
- refactor: 合并 handshake/handshake_std 为统一的 handshake 方法
- release v0.7.23

## [0.7.22] - 2026-06-15
### Changed
- feat(tdx): 实现板块文件下载与解析功能，添加测试
  - 新增 block.rs: BlockInfo 消息 (STD_BLOCK_DATA 0x06b9)，支持分块下载板块原始数据
  - 扩展 sector.rs: 实现 sync_block_files() 完整下载流程
    - 从 level1 分块下载原始板块文件
    - 解析二进制板块文件 (384字节头 + 2813字节记录)
    - 解压 zhb.zip 提取配置文件
    - 加载行业配置并生成CSV缓存
    - get_sector_list() 首次调用自动触发同步
  - 添加 9 个测试用例 (6个单元测试 + 3个集成测试标记ignore)
- release v0.7.22

## [0.7.21] - 2026-06-15
### Changed
- python: 修订项目主页url
- python: 测试代码恢复k线测试
- 调整资源配置文件
- python: 拆分常量
- 对齐rust和python的部分消息结构
- python: 删除cli包
- rust: 修复协议握手没实现的bug
- release v0.7.21

## [0.7.20] - 2026-06-12
### Changed
- go: 统一调整仓库地址为github
- go: 切换go基础库代码仓库到github
- update changelog

## [0.7.19] - 2026-06-12
### Changed
- 缓存代码
- update changelog
- python: git仓库忽略打包时自动生成的_version.py

## [0.7.18] - 2026-06-11
### Changed
- python: 恢复港股F10的接口
- 调整包路径
- update changelog

## [0.7.17] - 2026-06-09
### Changed
- python: 删除废弃的market代码
- refactor: restructure data/meta module with exchange, instrument, region, ticker_rules and timestamp
- update changelog

## [0.7.16] - 2026-06-09
### Changed
- feat: migrate F10 data modules to contrib layer, add financial_report/share_holder/notice
  - Add financial_report.py: quarterly report fetching from EastMoney API
    with full-market cache + single-stock fallback
  - Add share_holder.py: top10 shareholders data from TDX F10
  - Add notice.py: annual/quarterly report disclosure dates
  - Add f10.py: unified F10 data aggregation
  - Add fund.py: fund flow data placeholder in level1
  - Add reports_filename/top10_holders_filename to config module
  - Fix quarterly cache path: avoid double-offset bug (2026Q1 -> 2025Q4)
  - Fix SecurityCode comparison with correct_security_code normalization
  - Upgrade exception logging from debug to warning with traceback
  - Move f10.py from level1/ to tdx/ parent directory
  - Update level1 imports and command routing
- python: 保存python测试数据
- python: 调整命令字包路径
- python: 调整辅助函数的包路径
- python: 调整包路径
- python: 调整协议字段名
- python: 修复当日分笔成交记录的日期
- update changelog

## [0.7.15] - 2026-06-08
### Changed
- refactor: 优化run_module.py，package_name改为从LICENSE父目录自动获取，找不到则抛异常
- update changelog

## [0.7.14] - 2026-06-07
### Changed
- python: 新增集合竞价命令字
- update changelog

## [0.7.13] - 2026-06-07
### Changed
- python: improve version detection flow and ignore _version.py
- python: configure setuptools_scm write_to and fallback_version
- update changelog

## [0.7.12] - 2026-06-05
### Changed
- python: rename XdxrInfoList to XdxrEntry
- python: update xdxr caller and instrument default
- update changelog

## [0.7.11] - 2026-06-05
### Changed
- python: 标准协议统一处理流程
- update changelog

## [0.7.10] - 2026-06-05
### Changed
- python: 统一网络协议处理为process_level1_new，删除旧版process/process_level1及Serializable/Request/Response等废弃类
- update changelog

## [0.7.9] - 2026-06-03
### Changed
- Tighten Python package discovery and update publish scripts for pyproject build
- Refine MANIFEST.in: separate C/C++ exclusion block and include CMake rules
- update changelog

## [0.7.8] - 2026-06-03
### Changed
- chore: 统一换行符为LF
- fix: fix_header.py统一输出LF换行符，避免Windows下产生CRLF
- chore: .gitattributes强制所有源码文件使用LF换行符
- chore: .gitattributes移除json和capnp的LF约束
- Update publish scripts for pyproject build and exclude third_party
- update changelog

## [0.7.7] - 2026-06-03
### Changed

## [0.7.6.1] - 2026-06-17
### Changed
- refactor: 移除ThrottledMultiProgress，使用Rich原生refresh_per_second节流；优化main.py多级进度条
- fix: CTRL+C无法终止运行，添加signal中断处理
- update changelog
- chore: 统一换行符为LF
- fix: fix_header.py统一输出LF换行符，避免Windows下产生CRLF
- chore: .gitattributes强制所有源码文件使用LF换行符
- chore: .gitattributes移除json和capnp的LF约束
- Update publish scripts for pyproject build and exclude third_party
- update changelog
- Tighten Python package discovery and update publish scripts for pyproject build
- Refine MANIFEST.in: separate C/C++ exclusion block and include CMake rules
- update changelog
- python: 统一网络协议处理为process_level1_new，删除旧版process/process_level1及Serializable/Request/Response等废弃类
- update changelog
- python: 标准协议统一处理流程
- update changelog
- python: rename XdxrInfoList to XdxrEntry
- python: update xdxr caller and instrument default
- update changelog
- python: improve version detection flow and ignore _version.py
- python: configure setuptools_scm write_to and fallback_version
- update changelog
- python: 新增集合竞价命令字
- update changelog
- refactor: 优化run_module.py，package_name改为从LICENSE父目录自动获取，找不到则抛异常
- update changelog
- feat: migrate F10 data modules to contrib layer, add financial_report/share_holder/notice
  - Add financial_report.py: quarterly report fetching from EastMoney API
    with full-market cache + single-stock fallback
  - Add share_holder.py: top10 shareholders data from TDX F10
  - Add notice.py: annual/quarterly report disclosure dates
  - Add f10.py: unified F10 data aggregation
  - Add fund.py: fund flow data placeholder in level1
  - Add reports_filename/top10_holders_filename to config module
  - Fix quarterly cache path: avoid double-offset bug (2026Q1 -> 2025Q4)
  - Fix SecurityCode comparison with correct_security_code normalization
  - Upgrade exception logging from debug to warning with traceback
  - Move f10.py from level1/ to tdx/ parent directory
  - Update level1 imports and command routing
- python: 保存python测试数据
- python: 调整命令字包路径
- python: 调整辅助函数的包路径
- python: 调整包路径
- python: 调整协议字段名
- python: 修复当日分笔成交记录的日期
- update changelog
- python: 删除废弃的market代码
- refactor: restructure data/meta module with exchange, instrument, region, ticker_rules and timestamp
- update changelog
- python: 恢复港股F10的接口
- 调整包路径
- update changelog
- 缓存代码
- update changelog
- go: 统一调整仓库地址为github
- go: 切换go基础库代码仓库到github
- update changelog
- python: 修订项目主页url
- python: 测试代码恢复k线测试
- 调整资源配置文件
- python: 拆分常量
- 对齐rust和python的部分消息结构
- python: 删除cli包
- rust: 修复协议握手没实现的bug
- release v0.7.21
- feat(tdx): 实现板块文件下载与解析功能，添加测试
  - 新增 block.rs: BlockInfo 消息 (STD_BLOCK_DATA 0x06b9)，支持分块下载板块原始数据
  - 扩展 sector.rs: 实现 sync_block_files() 完整下载流程
    - 从 level1 分块下载原始板块文件
    - 解析二进制板块文件 (384字节头 + 2813字节记录)
    - 解压 zhb.zip 提取配置文件
    - 加载行业配置并生成CSV缓存
    - get_sector_list() 首次调用自动触发同步
  - 添加 9 个测试用例 (6个单元测试 + 3个集成测试标记ignore)
- release v0.7.22
- refactor: 合并 handshake/handshake_std 为统一的 handshake 方法
- release v0.7.23
- fix: extension server probe now uses ExtensionProtocolHandler handshake
- release v0.7.24
- refactor: align RequestHeader/ResponseHeader field names with Python base message
  - zip_flag -> frame_type (align with Python RequestHeader.frame_type)

  - packet_type -> packet_flag (align with Python ResponseHeader.packet_flag)

  - update all submodule references in heartbeat/minute_time/security_bars/finance_info
- release v0.7.25
- fix: compile error in connection_pool and varint_decode bug, ignore env-dependent tests
- git仓库忽略codebuddy的临时目录
- refactor: migrate exchange namespace to meta, adapter Update/Print to Instrument ref
  - Move exchange-related types under data::meta namespace
  - Change DataAdapter/FeatureAdapter Update/Print from string code to Instrument& inst
  - Update all 12 adapter subclasses, extract code via inst.symbol() internally
  - Fix callers (cache.cpp, tests) to pass Instrument via data::detect_symbol()
  - Update CMakeLists.txt source paths accordingly
- release v0.7.26
- python: 删除旧版本的代码检测
- release v0.7.27
- refactor(cpp): align cache paths with Rust/Python, add network fetch for stub adapters
  - Use inst.cache_dir() (e.g. sse/szse) in cache paths for kline_raw, xdxr, kline, trans, minute, kline_minute
  - kline_raw: day_raw/{cache_dir}/{symbol}.raw
  - xdxr: xdxr/{cache_dir}/{symbol}.csv
  - kline: day/{cache_dir}/{symbol}.csv
  - trans: trans/{cache_dir}/{year}/{yyyymmdd}/{symbol}.csv
  - minute: minute/{cache_dir}/{symbol}.csv
  - kline_minute: kline_minute/{cache_dir}/{symbol}.csv
  - Implement actual network fetch + save logic for kline, trans, minute, kline_minute adapters
- refactor(cpp): move protocol.h/cpp and helpers.h from level1/ to tdx/ to align with Rust/Python
  - Moved quant1x/contrib/data/tdx/level1/{protocol.h, protocol.cpp, helpers.h} -> quant1x/contrib/data/tdx/
  - Updated all 19 include references across client.h, instruments.cpp, tests, and 16 level1 headers
  - Fixed double-slash paths in xdxr_info.h, transaction_data.h
  - Updated CMakeLists.txt source paths and added helpers.h to header list
  - Updated helpers.rs comment to reflect new path
- refactor(cpp): move forward-adjustment logic from factors/ into tdx/kline, align DataKLine::Update with Python
  kline.h/cpp:
  - Full DataKLine::Update() flow matching Python: cache load -> date range -> batch fetch -> merge -> adjust -> save
  - apply_forward_adjustment_for_event() for incremental updates
  - apply_forward_adjustments_once() / calculate_pre_adjust() for general use
  - combine_adjustments_in_period() using meta::schema::CumulativeAdjustment
  - CSV I/O: read_kline_from_csv, save_kline, load_kline, get_kline_filename
  - get_xdxr_list / ipo_date_from_xdxrs reading from xdxr cache

  factors/base.h/cpp:
  - Remove all adjustment types/functions (moved to tdx::kline)
  - checkout_klines / klines_forward_adjusted_to_date now delegate to tdx:: functions
  - Keep feature constants and backward-compatible wrapper API
- refactor(cpp): align kline_raw with Python semantics, add ext protocol support, remove old base factors
  - kline_raw: fetch_kline_raw returns domain Bar (not protocol SecurityBar), internal SecurityBar->Bar conversion
  - level1: add ext_sync.h (ExtSynchronize, cmd 0x2454) and instrument_bars.h (InstrumentBars, cmd 0x23FF) for ext K-line protocol
  - client: add ExtensionProtocolHandler with get_ext_conn() for ext connection pool
  - fetch_kline_raw_from_ext: replace TODO stub with full ext protocol implementation
  - kline: remove redundant SecurityBar->Bar conversion, directly consume domain Bar
  - factors: delete old base.cpp/h, update base_compat.h/f10.cpp/history.cpp
  - tests: update tdd-* tests, user: update no0/strategy-no0
  - gitignore: add build_output.txt
- 删除cmake构建临时文件
- release v0.7.28
- 调整全角符号为半角, 调整io部分文件类功能归于std模块
- release v0.7.29
- python: git仓库忽略打包时自动生成的_version.py

## [0.7.6] - 2026-06-02
### Changed
- c++: 修复global_terminate_handler可能存在的遗漏异常信息的问题
- c++: 加固更新流程
- go: 调整fs的目录检测函数名
- 新增主要交易所的交易时段文档
- go: 将缓存可更新状态归于exchange
- 统一成份股字段名为蛇形命名规则
- rust: 按交易日期保存板块数据
- go: 添加部分基础数据
- python: 打包增加changelog
- rust: 下一个版本0.7.6
- csv字段名改成蛇形命名规则
- python: 调整前复权函数签名
- 增加示例代码
- c++: 收敛配置文件名常量到cpp
- go: 修订基础配置结构体
- go: 设计数据接口
- go: 调整累计复权因子
- vscode修复go插件的异常配置告警
- go: 代码暂存
- python: 实现status缓存可更新状态
- go: 调整获取文件状态的函数
- rust: 实现缓存更新状态检测功能
- python: 完善缓存更新状态
- c++: 实现缓存文件更新检测功能
- 暂存代码
- 合并dataset和data
- 暂存代码
- go: 修订mic包描述
- go: 调整RollingOnce函数签名
- 修订session
- go: 调整测试文件路径
- 调整session的rust和python实现
- 调整calendar更新流程，增加marker状态时间戳
- c++: 优化调度器销毁流程
- go: 收敛k线时间的处理函数
- go: 新增即时行情
- 修订基础数据适配器的注释，明确是基础数据
- c++: 优化线程池退出机制
- go: 成交数据的请求参数证券代码给用exchange.SecurityCode
- go: 优化成交数据接口
- 调整的部分代码
- go: 添加应用退出前的现场清理
- go: ExchangeId新增mic函数
- 暂存代码
- go: 更新go版本到1.26如此
- go: 分笔成交数据修订集合竞价收盘常量
- go: 调整证券信息结构
- go: 优化命令行参数
- go: 修正服务器列表缓存机制
- 调整日志模块路径为log
- rust: 调整edition版本为2024, 调整工具链版本1.93.0
- python: 新增修复源文件头部信息
- python: 优化模块运行脚本支持包路径映射
- vscode新增以模块方式运行python源代码的配置
- 修改开源协议为MIT
- 调整模块路径
- python: 新增AKShare实验代码
- python: 新增Qlib实验代码
- python: 禁止APScheduler的logging日志并组织传播
- python: 优化csv和切片互转函数
- python: 文件存储抽象类
- python: 导出dataclass的字段名工具函数
- python: 线程安全的单例模式
- python: 剔除演示代码和优化装饰器
- python: 完成KLine复权中下一个交易日获取方法的todo
- python: 优化交易日历
- python: 新增基础数据存储抽象类
- python: k线周期增加全局常量
- python: 调整获取证券信息函数签名
- python: 数据接口新增证券信息、K线和成交数据
- python: 优化证券列表的组成部分的描述信息
- python: 调整包路径，添加许可证头部
- python: 取消开发模式安装包时对quant1x/version.py的依赖，`pip install -e .` 开发模式会安装失败
- python: 新增quant1x实验笔记
- python: 格式化常量
- python: 与pyproject.toml对齐依赖项
- python: 统一项目的配置
- python: 调整引入包顺序
- python: 调整实现K线的源文件名
- python: 调整全局配置导出
- python: 新增RFC1123的转换函数
- python: 扩展行情7727和7721的区别是7721没有期货，属于券商版本，7727是通达信官方扩展双线服务器
- python: 新增NumberRange
- python: 优化香港市场的证券代码规则
- python: 优化NumberRange, 支持字符串范围
- python: 调整时段权限掩码
- python: 修订分钟数的计算方法
- 调整文档, 增加logo
- 新增港交所代码分配规则
- 新增上交所2025第5次代码分配规则
- go: 升级go版本最低1.26.0
- 明确go版本为1.26+
- python: 调整美股盘前、收盘以及盘后交易时段
- python: 增加区域region
- 新增港交所的价位表
- 调整交易所规则文件名
- python: 调整实验性代码
- python: 删除测试代码
- python: 删除测试代码
- python: 调整测试代码
- python: 调整测试代码
- python: 新增计算目标时区与本地时区的时间差
- python: 调整日志记录器
- python: 新增python以-m 模块方式运行时定位入口模块名的功能
- python: 新增多进度条封装
- python: 新增cli模块
- python: 新增交易代码规则模块
- python: 调整level1的业务指令类名
- python: 剔除配置
- python: 新增bar、trade的数据结构
- python: 新增资产类型
- python: 交易日历归属于meta
- python: 交易所归于meta
- python: meta数据增加基金fund
- python: meta新增合约结构
- python: 新增存储抽象类
- python: 调整基础数据的关键词, 从base改为basedata
- python: 确定对外研究接口为DataHandler, 证券代码的参数统一为字符串symbol, 而内部统一使用Instrument合约, Instrument可以转成symbol，而symbol需要通过查询，获得Instrument对象
- python: 调整关键词base为basedata
- python: 调整引入代码规则包路径
- python: 新增初级演示的主应用程序入口
- python: 删除冗余的引用
- python: 屏蔽7721的服务器资源
- python: 调整缓存路径为2级，数据类型和交易所缩写，原需要年份和日期拆分的子目录不变
- python: 调整测试代码
- 修订文档中关于项目风险的强调
- python: 删除顶层的系统功能
- c++: 调整文件系统命名空间
- python: 通过配置调整日志级别
- go: 调整数据源的包路径
- python: 将默认的开始日期改成1900年1月1日以支持港股和美股
- go: 新增时区差
- go: 调整时间戳
- go: 调整session支持国际化
- python: k线支持港股和美股
- python: 补充k线周期转换
- python: 调整对象设置属性的方法
- python: 调整服务器配置检测方式
- python: 优化ext握手协议
- python: Command允许自定义枚举
- python: 调整关键日志为warning
- python: 新增测试代码
- python: 增加部分协议debug日志
- python: 支持k线支持港股、美股
- python: 调整扩展行情代码
- python: 元数据增加汇率
- python: 暂时保存解码分红派息
- python: 新增独立的解析派息方案
- python: 调整汇率部分代码
- python: 优化分红多币种的处理方式
- python: 优化分红派送方案, 00008.hk 2020-09-03的分红方案原数据的分红金额缺少实物分配的货币替代派息方案
- python: 扩展行情增加分笔成交数据
- python: 缓存东方财富的港股分红派息接口
- python: 优化扩展行情的K线分红派息等权益数据
- python: 新增yyyymmdd格式的整型日期
- update changelog
- c++: 修复logger路径
- python: 梳理依赖库
- c++: 调整头文件宏
- c++: 优化因子工厂
- 调整vscode配置文件
- c++: 调整配置功能函数的路径
- go: 简化net/http代码
- c++: 删除废弃的交易配置函数
- go: 调整日志记录器
- c++: 补充croncpp库LICENSE
- go: 基础功能, 新增cron表达式
- go: 优化调度器
- go: 去掉cron的独立包路径, 并入base
- c++: 调整结构体
- go: 将cron归于std
- go: 修复homedir测试的错误
- 将net并入io
- rust: cargo fmt
- rust: 修复测试中路径的兼容问题
- c++: 删除废弃的代码
- c++: 调整注释格式
- go: 拆分config中对exchange依赖的函数
- go: 去掉标准库net的别名stdNet
- c++: 取消safe的q1x上层命名空间
- go: 统一季度函数
- go: 对齐c++的日期格式的常量
- c++: 补充config初始化函数注释
- go: 去掉config重复的子目录函数
- go: 修复logger包路径
- rust: 当下的连接池作为调整为标准协议的连接池
- rust: 补充关键函数注释
- rust: 修订config测试代码中路径的错误设定
- python: 修订函数注释
- go: 调整测试代码
- go: 补充context注释
- 分离缓存路径，meta作为core的一部分
- 对齐expanduser函数的路径拼接细节
- go: c++版本的argparse.hpp迁移
- go: 补充argparse文档
- rust: 调整cache模块, 新设置adapter单元
- rust: 修复单次拉取证券列表的最大条数的常量
- 配置参数对齐
- 优化证券代码的检测规则
- rust: 调整缓存适配器
- 调整证券代码的规则
- go: 新增同步板块流程
- go: 删除从gox复制的字符集工具
- c++: 补充Fast C++ CSV Parser许可证及文档
- 完善证券代码规则
- go: 压缩工具源文件名改为compress
- go: 抽象本地缓存的是否可更新功能
- go: 新增上市公司财务信息接口
- go: 优化缓存证券代码的处理流程
- 屏蔽深圳市场首位大于3的代码规则
- c++: 修复时区大小写的bug,
- c++: 修复crashwindows下不记录日志的问题
- c++: 修复交易标的的头文件路径
- go: 删除保留字段
- go: 交易标的的基本面
- c++: 将adapter调整到cache
- go: 构建更新数据流程
- git仓库删除launch.json
- c++: crash增加SA_SIGINFO

## [0.7.5] - 2025-12-05
### Changed
- rust: 下一个版本号0.7.5
- python: 优化证券代码部分函数
- c++: 优化策略数据加载机制，引入RollingOnce
- rust: 调整config模块路径
- python: 补充level1的协议实现
- python: 调整transaction测试代码
- rust: 补全f10信息, 附带完全了相关的level1的协议以及标准函数库的支持
- update changelog

## [0.7.4] - 2025-12-03
### Changed
- learn库归档到quant1x下
- update changelog

## [0.7.3] - 2025-12-03
### Changed
- rust: 下一个版本0.7.3
- go: 新增创建目录、检查文件路径的函数
- go: 新增homedir功能函数
- go: 创建目录增加对宿主目录写法的支持
- 调整strings文档文件名
- go: 调整宿主目录的引用函数
- c++: 优化调度器stop函数
- go: 优化客户端
- rust: 调整once组件源文件名
- c++: 删除numa文档
- c++: 调整头文件宏
- go: 删除homedir文档
- rust: 对齐homedir源文件名
- vscode: 调整debug控制台字体
- 调整vscode字体
- go: 缓存的服务器地址区分标准和扩展行情
- c++: 缓存的服务器地址区分标准和扩展行情
- rust: 缓存的服务器地址区分标准和扩展行情
- python: 修订level1包注释
- 修订level1协议的压缩表示常量
- c++: 修复is_optional_v 分支的判断逻辑
- python: 优化yaml解析
- 调整vscode chat字符集
- 对齐服务器检测结果server.bin的缓存结构
- python: 补全标准协议命令字
- rust: 对齐c++的process函数签名
- rust: 修复examples
- rust: 对齐c++的process函数
- c++: 调整logger目录
- python: 调整logger目录
- python: 修行函数注释
- python: 调整chart图标简易工具的包路径
- python: 调整部分代码
- python: 拆分cache模块到cache目录, 方便扩展
- python: 梳理废弃的代码, 合并功能相同的代码
- go: 删除废弃的演示代码
- 梳理exchange功能代码结构
- 修订引用exchange.h
- python: 梳理exchange工具包
- python: 修订cache工具包
- c++: 调整市场markets模块源文件名
- 对齐go、python的市场代码
- rust: 调整timestamp路径
- 对齐session、calendar和timestamp代码
- rust: 下一个版本0.7.4
- go: 新增港股代码规则
- python: 补充部分函数的注释
- 调整tcp连接池的技术文档
- go和rust的错误模块与c++对齐
- go: 新增http工具
- go: 公开exchange.DateRange函数
- rust: 拆分函数内嵌套函数的做法
- go: 删除废弃的连接池测试代码
- update changelog

## [0.7.2] - 2025-11-22
### Changed
- rust下一个版本0.7.2
- 消除重复的证券代码矫正函数
- 调整go代码包名为exchange
- 新增go版本的证券代码规则
- 抽象网络接口
- 暴露网络接口
- python实现c++网络模式
- rust市场代码改源文件名为markets
- 备份exchange.py
- 实现python版本的证券代码列表
- 整理python的证券代码, 消除package和python源文件名冲突的问题
- 消除部分警告
- 统一level1标准行情接口名
- 调整vscode mac下的配置
- 修复osx下numa_code未使用的警告
- 删除对duktape javascript库的依赖
- 去掉对duktape的依赖
- 修复统一标准协议后的对齐问题
- 修复函数未使用参数的告警，用有“故意未使用"的前缀下划”_“线来解决
- rustfmt
- 修复调度器stop可能被调度线程自身调用, 如果join会死锁
- 修复linux缺少numaif.h的问题
- 修复quant1x cmake config脚本对date 非windows平台和clang的判断
- 兼容quant1x.yaml可以不配置分钟级K线
- 屏蔽mimalloc的版本提示
- 小调整部分crash handler代码
- 删除numa
- 修复mac下cpu亲和性绑定失败的问题
- 屏蔽httplib
- 删除废弃的测试代码
- 修订注释
- 删除废弃的python代码
- 删除废弃的代码
- 调整vscode终端的字体
- 调整go代码依赖库版本
- 优化go版本的协议实现方式
- 优化客户端
- 调整字符集功能的源文件名，避免和iconv冲突
- 删除废弃的fft代码
- 修订gcc和clang的最低版本
- 调整ringbuffer的包名
- ringbuffer文档文件改名，去掉README
- c++：验证调度器延迟执行
- rust: 调整sina解码器的包路径
- go: 新增缓存机制
- 新增证券代码SecurityCode结构体，减少运行时对代码的检测
- 修订上海证券交所证券代码规则链接
- c++: 修订关于证券交易所的注释
- c++: 修复字符集头文件
- go: logger包路径和c++统一为log
- c++: 调整头文件宏
- c++: 优化FP Growth算法
- go: 重构FP Growth算法
- rust: 删除废弃sina解码的测试代码
- rust: 优化FP Growth算法
- update changelog

## [0.7.1] - 2025-11-03
### Changed
- rust代码下一个版本号0.7.0
- 修正证券列表, 已支持北交所
- rust代码下一个版本0.7.1
- 屏蔽测试utf-8异常的观测代码, 暂时保留后面可能还会用到
- 优化c++字符集转码
- 调整证券名称的字节数为16, 后续的4个字节不属于证券名称, 视为未知/保留字段
- c++新增minizip库, 从服务器下载行业及zhb.zip, 解压获得tdxzs*.cfg
- 修复go和c++版本的板块数据差异, python补全板块代码的sh前缀
- 调整mimlloc编译辅助实现文件的路径到std
- 删除废弃的代码
- 删除废弃的代码
- go代码网络与c++对齐
- cmake安装脚本增加minizip
- 统一ETF的level1基准价格单位的处理
- rust实现板块每日自动更新
- update changelog

## [0.7.0] - 2025-10-29
### Changed
- 更新go依賴庫engine的版本號到1.12.2
- 取消对javascript库的支持, 改为c++实现的解码
- 优化level1证券列表接口, 支持北交所
- update changelog

## [0.6.160] - 2025-10-27
### Changed
- 修改cmake编译gtest测试用例存在失败的问题
- 修复5minK线前复权金额错误的bug
- 修复增加分钟K线数据存在日期和记录不对齐的bug
- 对齐rust与c++的量前复权
- update changelog

## [0.6.159] - 2025-10-27
### Changed
- 优化部分F10数据的代码
- update changelog

## [0.6.158] - 2025-10-26
### Changed
- 删除废弃的临时文件
- 修订应用程序名
- 优化f10缓存过期机制
- 调整测试代码
- 修复git tag脚本
- 修复c++ Testing模板
- 修复vscode 单元测试执行其它exe的问题
- 修改除权除息文档文件名
- 删除废弃的除权除息代码
- 调整vscode配置
- 剔除废弃的代码
- 修复std::error_code未使用告警
- 调整部分代码
- 调整测试代码
- 调整c++ catch2调试配置
- 删除扩展终端选项
- windows下调整为gcc的cppdbg
- 成交量复权与通达信算法对齐
- update changelog

## [0.6.157] - 2025-10-20
### Changed
- 优化.env的检索方法
- update changelog

## [0.6.156] - 2025-10-20
### Changed
- 新增vscode python配置
- 修改加载,env配置
- 优化加载.env
- update changelog

## [0.6.155] - 2025-10-20
### Changed
- 修复quant1x引入的bug
- update changelog

## [0.6.154] - 2025-10-20
### Changed
- 修订config描述
- update changelog

## [0.6.153] - 2025-10-20
### Changed
- 删除废弃的代码，补全函数注释
- 补充注释
- 文件改名前需要关闭临时tmp文件
- 调整readme文件名，采用功能模块名为前缀，方便维护
- 优化.env主导的工作区逻辑
- update changelog

## [0.6.152] - 2025-10-20
### Changed
- rust下一个版本0.6.15
- 优化csv写入, 修复可能null的bug
- 格式化代码，风格保持一致
- 删除废弃的代码
- 优化c++版本的分笔成交数据的更新流程, 修改csv读写方法，去掉反射的部分，作为adapter更新，不适用带有返回列表的函数
- 修订日志
- update changelog

## [0.6.151] - 2025-10-19
### Changed
- 交易日历增加盘前边界测试
- update changelog

## [0.6.150] - 2025-10-19
### Changed
- rust下一个版本0.6.13
- rust代码注释转中文
- update changelog

## [0.6.149] - 2025-10-19
### Changed
- 转英文注释为中文
- update changelog

## [0.6.148] - 2025-10-19
### Changed
- 将英文注释改成中文
- update changelog

## [0.6.147] - 2025-10-19
### Changed
- 修订网络超时
- 调整测试的证券代码
- 细化kline数据为空时的日志
- update changelog

## [0.6.146] - 2025-10-19
### Changed
- rust版本新增反序列化错判枚举
- rust协议接口统一反序列化失败或异常返回错误枚举
- update changelog

## [0.6.145] - 2025-10-19
### Changed
- 进程结束刷新日志改为info级别
- update changelog

## [0.6.144] - 2025-10-19
### Changed
- 优化加固rust连接池
- update changelog

## [0.6.143] - 2025-10-19
### Changed
- rust版本去掉日志文件前缀quant1x
- c++ level1协议处理模板函数增加quant1x::error
- 删除废弃的日志sink代码
- update changelog

## [0.6.142] - 2025-10-19
### Changed
- rust版本按级别拆分成不同的日志文件
- 删除git子模块
- 格式化代码
- 合并rust版本的协议命令字
- 去掉mod.rs中的函数，将mod作为纯粹的模块聚合功能
- tcp连接池增加重试功能
- 优化协议编解码
- update changelog

## [0.6.141] - 2025-10-18
### Changed
- 转换部分代码中全角符号为半角
- 转换部分全角字符为半角
- 新增按日志级别拆分日志
- 调整按日轮转
- 日志文件惰性生成
- update changelog

## [0.6.140] - 2025-10-12
### Changed
- 删除废弃的代码
- update changelog

## [0.6.139] - 2025-10-11
### Changed
- 删除废弃的测试文本文件
- update changelog

## [0.6.138] - 2025-10-10
### Changed
- 新增tail工具, 可安装
- update changelog

## [0.6.137] - 2025-10-10
### Changed
- 新增隐形价差的计算及单元测试
- update changelog

## [0.6.136] - 2025-10-10
### Changed
- 删除废弃的代码
- 优化tcp连接池，修复endpoint活跃数的bug，release时不应该归还endpoint，减少活跃数
- update changelog

## [0.6.135] - 2025-10-06
### Changed
- 删除废弃的注释
- update changelog

## [0.6.134] - 2025-10-06
### Changed
- 格式化c++代码
- 格式化并修正部分注释
- update changelog

## [0.6.133] - 2025-10-05
### Changed
- rust下一个版本0.6.12
- 前十大流通股东结构字段调整, 关于数量的字段改为int64_t
- 修订注释
- update changelog

## [0.6.132] - 2025-10-05
### Changed
- !1 backtest: implement FIFO round-trip stats, add stats helper and compat…
  * backtest: implement FIFO round-trip stats, add stats helper and compat…
  * 新增0号演示性策略(C++)
  * 调整错误码的命名空间
- 调整backtest代码结构
- update changelog

## [0.6.131] - 2025-10-04
### Changed
- 调整测试yaml配置文件解析功能, msvc编译不通过
- 修复msvc编译yaml解析工具失败的问题, examples引用yaml-cpp方式不对
- update changelog

## [0.6.130] - 2025-10-04
### Changed
- 屏蔽进度条的锁, 进度条本身就是线程安全
- 消除未使用变量的告警
- update changelog

## [0.6.129] - 2025-10-03
### Changed
- c++默认关闭mimalloc
- update changelog

## [0.6.128] - 2025-10-03
### Changed
- 优化yaml配置文件解析方式
- 调整编译选项，gcc暂时还是使用-O2编译
- 消除msvc ULONG类型转换告警的问题
- 修复quant1x依赖项不能获取属性的问题
- 屏蔽重复的C++编译选项
- 新增默认开启mimalloc
- update changelog

## [0.6.127] - 2025-10-03
### Changed
- 调整ringbuffer路径
- update changelog

## [0.6.126] - 2025-10-03
### Changed
- 调整ringbuffer基准测试的引用路径
- 删除github action设置
- 提供基础版本的配置文件，说明各版本的配置文件路径
- 新增json到struct映射的c++模板工具
- 调整encoding测试用例源文件名，json新增自动反射编解码功能
- update changelog

## [0.6.125] - 2025-10-03
### Changed
- 删除废弃的代码
- update changelog

## [0.6.124] - 2025-10-03
### Changed
- 修订依赖库列表布局
- update changelog

## [0.6.123] - 2025-10-03
### Changed
- 修正ringbuffer路径
- 删除废弃的代码日志
- 调整numpy和numba的版本号，匹配numpy2.3.0
- update changelog

## [0.6.122] - 2025-10-02
### Changed
- 修订python类self拼写的问题
- update changelog

## [0.6.121] - 2025-10-02
### Changed
- 修订用于python加载本地缓存数据的QUANT1X_WORK关键字的机制
- update changelog

## [0.6.120] - 2025-10-01
### Changed
- 修复头文件路径
- 补充关键注释
- rust下一个版本0.6.11
- update changelog

## [0.6.119] - 2025-10-01
### Changed
- 将RingBuffer归于runtime
- update changelog

## [0.6.118] - 2025-10-01
### Changed
- 新增mio网络服务端和客户端的demo
- 新增rust cron测试代码
- 新增rust 筹码分析代码
- 新增rust hook测试代码
- 迁移早期的rust的engine代码
- 新增早期rust stock代码
- 新增rust早期的q1x代码
- 补充q1x rust代码
- 新增rust MuZero代码
- 新增rust cnn代码
- 新增chips rust代码
- update changelog

## [0.6.117] - 2025-10-01
### Changed
- 修复头文件扩展名的问题
- 调整测试工具的路径
- 明确k线协议中参数I其实是frequency频率，或者step步长，运行在指定k线类型的前提下按
  frequency聚合k线
- runtime：添加基于 tokio 的 AsyncScheduler 与 core 全局绑定；清理 scheduler.rs 中警告
- 新增异步调度器模块
- update changelog

## [0.6.116] - 2025-10-01
### Changed
- 删除废弃的代码
- 修改函数签名, 入参data不符合日期的定义
- 调整capnp的测试缓存文件路径, 这个路径需要按照实际调整，这里仅仅是为了去掉q2x的关键字
- c++原文档合并到项目的readme中, 不在独立的展示c++的文档
- 头文件扩展名修改为h
- 将主应用程序名修改为q1x，q1x可以作为quant1x的缩写, 但是q2x就不合适了，没有2x这样语义
- update changelog

## [0.6.115] - 2025-10-01
### Changed
- 删除废弃的js测试工具
- update changelog

## [0.6.114] - 2025-10-01
### Changed
- 测试工具转移到项目根路径下tools
- 演示类代码转移到exemples
- rust下一个版本号0.6.10
- update changelog

## [0.6.113] - 2025-10-01
### Changed
- 缓存一个版本
- 删除mod备份
- 优化RingBuffer：
  1. 改成使用 std::byte storage[]（从 aligned_storage 改为 byte-array），并使用 std::launder，这影响了如何对内存读写与类型别名进行优化与内联（不同编译器处理此类写法的 codegen 有差异）。
  2. 添加了异常安全 rollback（在构造失败时回滚 enqueue_pos_/slot.seq），并在函数上标注了 noexcept(...) 条件，这可能让编译器为异常路径插入额外栈/原子操作或限制某些优化（尤其是 GCC 在当前版本上对条件 noexcept + 异常捕获/回滚的交互优化可能更保守）。
  3. backoff_spin 更激进/可变，这有利于减少 busy-wait，但会改变时序与测量（不大可能直接造成 GCC 性能降这么多，但会影响热点在运行时的竞争特征）。
- update changelog

## [0.6.112] - 2025-09-30
### Changed
- 修复测试告警的问题
- 调整代码格式
- 删除临时文件
- 删除废弃的测试工具
- 新增ring buffer go版本已经rust实现的“Vyukov bounded MPMC queue”
- update changelog

## [0.6.111] - 2025-09-30
### Changed
- 修复网络数据可能存在包不完整的情况，修复分钟级别K线的开始日期没矫正导致的除权除息以及level1协议参数造成逻辑上的越界问题
- update changelog

## [0.6.110] - 2025-09-30
### Changed
- 修正rust版本要求，上下文一致1.90+
- update changelog

## [0.6.109] - 2025-09-30
### Changed
- rust下一个版本0.6.8
- update changelog

## [0.6.108] - 2025-09-30
### Changed
- 更新项目文档, 增加rust库下载数,
- rust版本确定为1.90+
- update changelog

## [0.6.107] - 2025-09-30
### Changed
- 删除不完善的timestamp文档
- 新增log4rs的yaml格式配置文件
- 新增分钟级别k线, 默认关闭, 打开的方式是quant1x.yaml中配置data.cache.kline, 按照pandas的freq的规则设置key，value为布尔值true
- update changelog

## [0.6.106] - 2025-09-30
### Changed
- rust下一个版本0.6.6
- 内存管理使用mimalloc
- rust代码格式化
- update changelog

## [0.6.105] - 2025-09-29
### Changed
- 更新rust配置项
- 新增rust版本的分笔成交数据
- update changelog

## [0.6.104] - 2025-09-29
### Changed
- 修复只有windows平台才依赖的组件
- update changelog

## [0.6.103] - 2025-09-29
### Changed
- 更新rust版本号到0.6.2
- update changelog

## [0.6.102] - 2025-09-29
### Changed
- 增加quant1x证书
- update changelog

## [0.6.101] - 2025-09-29
### Changed
- 调整文档, 明确目前只支持level1行情数据
- 更新rust版本号为0.6.1
- update changelog

## [0.6.100] - 2025-09-29
### Changed
- rust/c++的首页设置在github,仓库在gitee
- update changelog

## [0.6.99] - 2025-09-29
### Changed
- 调整main.rs路径与c++保持一致
- 调整rust package的包含路径
- update changelog

## [0.6.98] - 2025-09-29
### Changed
- 修复kline数据处理没有遵循”每页最新优先“的bug
- update changelog

## [0.6.97] - 2025-09-28
### Changed
- rust补全证券代码
- update changelog

## [0.6.96] - 2025-09-28
### Changed
- rust工具包quant1x发布0.2.0版本
- update changelog

## [0.6.95] - 2025-09-28
### Changed
- 补充部分rust level1协议
- 去掉行尾多余的分号
- 文件不存在时，时间戳返回0
- http协议时间时UTC，需要转成本地时间
- 删除rust废弃的代码
- 新增rust版本的level1协议的实现
- update changelog

## [0.6.94] - 2025-09-27
### Changed
- 新增rust版本二进制小端的编解码
- update changelog

## [0.6.93] - 2025-09-27
### Changed
- 修复doctest
- chore: prepare crate for publish (narrow include)
- 优化二进制小端编解码器
- 新增rust tcp连接池功能
- update changelog

## [0.6.92] - 2025-09-27
### Changed
- 修复包路径错误
- 新增rust版本的js解码器
- update changelog

## [0.6.91] - 2025-09-25
### Changed
- 调整level1头文件包含写法
- update changelog

## [0.6.90] - 2025-09-24
### Changed
- 新增FP Growth rust版本
- update changelog

## [0.6.89] - 2025-09-24
### Changed
- 修复tag标签展示样式
- update changelog

## [0.6.88] - 2025-09-24
### Changed
- 调整项目文件增加tag的展示
- update changelog

## [0.6.87] - 2025-09-24
### Changed
- 优化二进制文件转c++头文件的工具
- update changelog

## [0.6.86] - 2025-09-24
### Changed
- 调整FP Growth到learn
- update changelog

## [0.6.85] - 2025-09-24
### Changed
- 补全未实现的成员函数及文档
- update changelog

## [0.6.84] - 2025-09-24
### Changed
- 新增vscode cmake编译配置
- 新增FP Growth c++版本
- update changelog

## [0.6.83] - 2025-09-24
### Changed
- 删除废弃的代码，quant1x不收录实验性代码
- update changelog

## [0.6.82] - 2025-09-24
### Changed
- 新增FP Growth的go语言实现版本
- update changelog

## [0.6.81] - 2025-09-24
### Changed
- 新增贝叶斯分类算法
- 新增神经网络
- 新增simd实验性代码
- 新增板块实验性代码
- 新增sar实验性代码
- 新增go加载动态库的实验性代码
- 新增Garch模型实验性代码
- 新增筹码实验性代码
- 新增筹码分布实验性代码
- update changelog

## [0.6.80] - 2025-09-23
### Changed
- 修订tcp连接池的注释及文档
- update changelog

## [0.6.79] - 2025-09-23
### Changed
- 补充AI对tcp连接池实现方案的评估文档
- update changelog

## [0.6.78] - 2025-09-23
### Changed
- utf8 to gbk
- merge 0.6.x
- update changelog

## [0.6.77] - 2025-09-23
### Changed
- 补充部分技术文档
- 修复plantuml语法上的告警
- update changelog

## [0.6.76] - 2025-09-23
### Changed
- 新增factors go package概述
- 调整config c++实现代码的路径
- update changelog

## [0.6.75] - 2025-09-23
### Changed
- 新增检查当前版本是否在pypi中已经存在
- update changelog

## [0.6.74] - 2025-09-23
### Changed
- 优化shell脚本
- update changelog

## [0.6.73] - 2025-09-23
### Changed
- 调整bat发布脚本
- update changelog

## [0.6.72] - 2025-09-23
### Changed
- 修复else不能识别的问题
- update changelog

## [0.6.71] - 2025-09-23
### Changed
- 新增windows ps1的pypi发布脚本
- update changelog

## [0.6.70] - 2025-09-23
### Changed
- quant1x go实现只是提供新扩展的功能，主体功能都在engine中
- 补全go版本应用入口
- update changelog

## [0.6.69] - 2025-09-22
### Changed
- 调整go应用入口main
- update changelog

## [0.6.68] - 2025-09-22
### Changed
- 计算优化归于标准库组件
- update changelog

## [0.6.67] - 2025-09-22
### Changed
- 更新go依赖库engine版本到1.11.16
- update changelog

## [0.6.66] - 2025-09-22
### Changed
- 整理插件模块源文件
- update changelog

## [0.6.65] - 2025-09-22
### Changed
- 调整测试代码
- 修正头文件引用
- update changelog

## [0.6.64] - 2025-09-22
### Changed
- 调整yaml解码功能归于encoding包
- update changelog

## [0.6.63] - 2025-09-21
### Changed
- 调整tdd协议测试源文件名
- 删除python的行情数据实现
- 更新magic_enum版本
- 调度器归于runtime
- update changelog

## [0.6.62] - 2025-09-21
### Changed
- 更新xtensor版本到0.27.1
- update changelog

## [0.6.61] - 2025-09-20
### Changed
- 删除对Javascript支持的内容
- update changelog

## [0.6.60] - 2025-09-20
### Changed
- 修订关于rust开发环境的说明
- update changelog

## [0.6.59] - 2025-09-20
### Changed
- 新增git仓库过滤rust临时文件
- 新增rust项目初始设定
- 优化cpu亲和性，新增numa节点感知功能
- 调整README文档
- 字符串操作新增字符串蛇形与驼峰的转换功能
- 新增numa测试代码
- 新增go版本的timestamp
- update changelog

## [0.6.58] - 2025-09-20
### Changed
- 代码对齐
- update changelog

## [0.6.57] - 2025-09-20
### Changed
- 优化yaml序列化
- update changelog

## [0.6.56] - 2025-09-20
### Changed
- 改q1x命名空间为quant1x
- update changelog

## [0.6.55] - 2025-09-19
### Changed
- 修复日历js原算法的测试数据(√)
- update changelog

## [0.6.54] - 2025-09-19
### Changed
- 修复日历的c++/go的算法(√)
- update changelog

## [0.6.53] - 2025-09-18
### Changed
- 修复msvc编译boost.pfr+yaml-cpp失败的问题
- 恢复xsimd版本13.2.0，最新的代码gcc/clang编译通过, msvc会出现异常
- 修复boost.pfr模板编译的错误告警
- 绘图功能库
- 打开mimalloc库的检测
- 调整catch2用法, 增加catch_discover_tests的调用，但是vs2022并不能自动识别
- update changelog

## [0.6.52] - 2025-09-18
### Changed
- 新增第三方http库cpp-httplib，版本0.26.0
- 新增cpp-httplib测试代码，实验失败
- update changelog

## [0.6.51] - 2025-09-18
### Changed
- 新增go版本实验性质的交易日历解码工具
- 调整代码
- 实验结果, 可能是解码的问题, 只过滤双休日, 对于法定的节假日处理的比较薄弱，应该不是x的函数的问题。
- 统一交易日历编解码的源文件名
- update changelog

## [0.6.50] - 2025-09-17
### Changed
- 修订波峰检测的测试代码
- update changelog

## [0.6.49] - 2025-09-17
### Changed
- 修订过滤规则的测试代码
- update changelog

## [0.6.48] - 2025-09-17
### Changed
- 调整部分测试代码
- update changelog

## [0.6.47] - 2025-09-17
### Changed
- 修行性能优化指南的
- 补充后续可能的优化点的实验性代码
- update changelog

## [0.6.46] - 2025-09-17
### Changed
- 修复clang编译失败的问题
- update changelog

## [0.6.45] - 2025-09-16
### Changed
- 更新依赖库版本
- update changelog

## [0.6.44] - 2025-09-16
### Changed
- 整理level1测试代码
- update changelog

## [0.6.43] - 2025-09-16
### Changed
- 调整level1下的encoding为helpers, 防止与顶层编解码encoding冲突
- update changelog

## [0.6.42] - 2025-09-16
### Changed
- 新增为msvc准备的vcpkg的配置文件
- update changelog

## [0.6.41] - 2025-09-16
### Changed
- 将go版本的learn包迁移的quant1x顶层
- update changelog

## [0.6.40] - 2025-09-16
### Changed
- 更新依赖库版本
- 调整网络部分组件头文件宏前缀
- update changelog

## [0.6.39] - 2025-09-15
### Changed
- 修复全天交易时段分钟数综合大于240的bug
- update changelog

## [0.6.38] - 2025-09-15
### Changed
- 修复尾盘集合竞价判断不准确的bug
- update changelog

## [0.6.37] - 2025-09-15
### Changed
- 新增CPU性能优化文档
- 调整目录结构
- update changelog

## [0.6.36] - 2025-09-15
### Changed
- 新增获取cpu信息的函数
- 各数据下载的并发数可以从配置文件加载
- update changelog

## [0.6.35] - 2025-09-15
### Changed
- 新增获取物理CPU数量的函数
- update changelog

## [0.6.34] - 2025-09-15
### Changed
- 调整分笔成交数据的增量更新
- 更新数据线程数设定为5个
- update changelog

## [0.6.33] - 2025-09-14
### Changed
- 更新依赖库版本
- update changelog

## [0.6.32] - 2025-09-14
### Changed
- 调整交易日历实验性代码, 检测结果只有开始的日期是对的，还是存在问题.
- 新增go版本机器学习-聚类工具
- update changelog

## [0.6.31] - 2025-09-13
### Changed
- 删除go pool功能, 需要重新规划package
- 删除废弃的代码
- update changelog

## [0.6.30] - 2025-09-13
### Changed
- 修正头文件宏与目录名保持一致
- update changelog

## [0.6.29] - 2025-09-13
### Changed
- 删除中文注释
- update changelog

## [0.6.28] - 2025-09-13
### Changed
- 修订k线和分笔成交数据的字段为小写的蛇形格式，兼容go版本的数据结构
- update changelog

## [0.6.27] - 2025-09-13
### Changed
- 新增可用环境变量QUAN1X_WORK来指定配置文件的目录名是quant1x还是新版本的q2x
- update changelog

## [0.6.26] - 2025-09-13
### Changed
- 修订函数注释
- 删除废弃的cista编解码库的测试代码, 弃用cista
- 优化yaml配置自动反序列化
- 进度条indicators使用别名mpb，防止与技术指标冲突
- 原dataframe目录改名pandas
- 修改日线源文件名, 去掉复数s, 与其它类型k线命名保持一致
- 原dataframe目录改名pandas
- 新增分钟级别的K线
- 优化分钟级别K线开关, 默认关闭
- update changelog

## [0.6.25] - 2025-09-12
### Changed
- 更新依赖库boost.pfr版本
- update changelog

## [0.6.24] - 2025-09-12
### Changed
- 修复data和fmt新版本问题
- update changelog

## [0.6.23] - 2025-09-12
### Changed
- 更新date库
- 更新fmt库
- update changelog

## [0.6.22] - 2025-09-09
### Changed
- 调整base内部引用包的路径, 取消相对路径的用法
- update changelog

## [0.6.21] - 2025-09-08
### Changed
- 去掉不必要的设置注释
- update changelog

## [0.6.20] - 2025-09-08
### Changed
- bat文件设置GBK编码和回车换行
- update changelog

## [0.6.19] - 2025-09-08
### Changed
- 修正头文件归类不清晰的问题
- update changelog

## [0.6.18] - 2025-09-08
### Changed
- 更新依赖库boost.pfr版本(d9fde1f2a0ab92e7db204c49c1612536cb296dad)
- update changelog

## [0.6.17] - 2025-09-07
### Changed
- 修复头文件兼容问题
- update changelog

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


[Unreleased]: https://codeberg.org/quant1x/quant1x/compare/v0.9.0...HEAD
[0.9.0]: https://codeberg.org/quant1x/quant1x/compare/v0.8.19...v0.9.0
[0.8.19]: https://codeberg.org/quant1x/quant1x/compare/v0.8.18...v0.8.19
[0.8.18]: https://codeberg.org/quant1x/quant1x/compare/v0.8.17...v0.8.18
[0.8.17]: https://codeberg.org/quant1x/quant1x/compare/v0.8.16...v0.8.17
[0.8.16]: https://codeberg.org/quant1x/quant1x/compare/v0.8.15...v0.8.16
[0.8.15]: https://codeberg.org/quant1x/quant1x/compare/v0.8.14...v0.8.15
[0.8.14]: https://codeberg.org/quant1x/quant1x/compare/v0.8.13...v0.8.14
[0.8.13]: https://codeberg.org/quant1x/quant1x/compare/v0.8.12...v0.8.13
[0.8.12]: https://codeberg.org/quant1x/quant1x/compare/v0.8.11...v0.8.12
[0.8.11]: https://codeberg.org/quant1x/quant1x/compare/v0.8.10...v0.8.11
[0.8.10]: https://codeberg.org/quant1x/quant1x/compare/v0.8.9...v0.8.10
[0.8.9]: https://codeberg.org/quant1x/quant1x/compare/v0.8.8...v0.8.9
[0.8.8]: https://codeberg.org/quant1x/quant1x/compare/v0.8.7...v0.8.8
[0.8.7]: https://codeberg.org/quant1x/quant1x/compare/v0.8.6...v0.8.7
[0.8.6]: https://codeberg.org/quant1x/quant1x/compare/v0.8.5...v0.8.6
[0.8.5]: https://codeberg.org/quant1x/quant1x/compare/v0.8.4...v0.8.5
[0.8.4]: https://codeberg.org/quant1x/quant1x/compare/v0.8.3...v0.8.4
[0.8.3]: https://codeberg.org/quant1x/quant1x/compare/v0.8.2...v0.8.3
[0.8.2]: https://codeberg.org/quant1x/quant1x/compare/v0.8.1...v0.8.2
[0.8.1]: https://codeberg.org/quant1x/quant1x/compare/v0.8.0...v0.8.1
[0.8.0]: https://codeberg.org/quant1x/quant1x/compare/v0.7.85...v0.8.0
[0.7.85]: https://codeberg.org/quant1x/quant1x/compare/v0.7.84...v0.7.85
[0.7.84]: https://codeberg.org/quant1x/quant1x/compare/v0.7.83...v0.7.84
[0.7.83]: https://codeberg.org/quant1x/quant1x/compare/v0.7.82...v0.7.83
[0.7.82]: https://codeberg.org/quant1x/quant1x/compare/v0.7.81...v0.7.82
[0.7.81]: https://codeberg.org/quant1x/quant1x/compare/v0.7.80...v0.7.81
[0.7.80]: https://codeberg.org/quant1x/quant1x/compare/v0.7.79...v0.7.80
[0.7.79]: https://codeberg.org/quant1x/quant1x/compare/v0.7.78...v0.7.79
[0.7.78]: https://codeberg.org/quant1x/quant1x/compare/v0.7.77...v0.7.78
[0.7.77]: https://codeberg.org/quant1x/quant1x/compare/v0.7.76...v0.7.77
[0.7.76]: https://codeberg.org/quant1x/quant1x/compare/v0.7.75...v0.7.76
[0.7.75]: https://codeberg.org/quant1x/quant1x/compare/v0.7.74...v0.7.75
[0.7.74]: https://codeberg.org/quant1x/quant1x/compare/v0.7.73...v0.7.74
[0.7.73]: https://codeberg.org/quant1x/quant1x/compare/v0.7.72...v0.7.73
[0.7.72]: https://codeberg.org/quant1x/quant1x/compare/v0.7.71...v0.7.72
[0.7.71]: https://codeberg.org/quant1x/quant1x/compare/v0.7.70...v0.7.71
[0.7.70]: https://codeberg.org/quant1x/quant1x/compare/v0.7.69...v0.7.70
[0.7.69]: https://codeberg.org/quant1x/quant1x/compare/v0.7.68...v0.7.69
[0.7.68]: https://codeberg.org/quant1x/quant1x/compare/v0.7.67...v0.7.68
[0.7.67]: https://codeberg.org/quant1x/quant1x/compare/v0.7.66...v0.7.67
[0.7.66]: https://codeberg.org/quant1x/quant1x/compare/v0.7.65...v0.7.66
[0.7.65]: https://codeberg.org/quant1x/quant1x/compare/v0.7.64...v0.7.65
[0.7.64]: https://codeberg.org/quant1x/quant1x/compare/v0.7.63...v0.7.64
[0.7.63]: https://codeberg.org/quant1x/quant1x/compare/v0.7.62...v0.7.63
[0.7.62]: https://codeberg.org/quant1x/quant1x/compare/v0.7.61...v0.7.62
[0.7.61]: https://codeberg.org/quant1x/quant1x/compare/v0.7.60...v0.7.61
[0.7.60]: https://codeberg.org/quant1x/quant1x/compare/v0.7.59...v0.7.60
[0.7.59]: https://codeberg.org/quant1x/quant1x/compare/v0.7.58...v0.7.59
[0.7.58]: https://codeberg.org/quant1x/quant1x/compare/v0.7.57...v0.7.58
[0.7.57]: https://codeberg.org/quant1x/quant1x/compare/v0.7.56...v0.7.57
[0.7.56]: https://codeberg.org/quant1x/quant1x/compare/v0.7.55...v0.7.56
[0.7.55]: https://codeberg.org/quant1x/quant1x/compare/v0.7.54...v0.7.55
[0.7.54]: https://codeberg.org/quant1x/quant1x/compare/v0.7.53...v0.7.54
[0.7.53]: https://codeberg.org/quant1x/quant1x/compare/v0.7.52...v0.7.53
[0.7.52]: https://codeberg.org/quant1x/quant1x/compare/v0.7.51...v0.7.52
[0.7.51]: https://codeberg.org/quant1x/quant1x/compare/v0.7.50...v0.7.51
[0.7.50]: https://codeberg.org/quant1x/quant1x/compare/v0.7.49...v0.7.50
[0.7.49]: https://codeberg.org/quant1x/quant1x/compare/v0.7.48...v0.7.49
[0.7.48]: https://codeberg.org/quant1x/quant1x/compare/v0.7.47...v0.7.48
[0.7.47]: https://codeberg.org/quant1x/quant1x/compare/v0.7.46...v0.7.47
[0.7.46]: https://codeberg.org/quant1x/quant1x/compare/v0.7.45...v0.7.46
[0.7.45]: https://codeberg.org/quant1x/quant1x/compare/v0.7.44...v0.7.45
[0.7.44]: https://codeberg.org/quant1x/quant1x/compare/v0.7.43...v0.7.44
[0.7.43]: https://codeberg.org/quant1x/quant1x/compare/v0.7.42...v0.7.43
[0.7.42]: https://codeberg.org/quant1x/quant1x/compare/v0.7.41...v0.7.42
[0.7.41]: https://codeberg.org/quant1x/quant1x/compare/v0.7.40...v0.7.41
[0.7.40]: https://codeberg.org/quant1x/quant1x/compare/v0.7.39...v0.7.40
[0.7.39]: https://codeberg.org/quant1x/quant1x/compare/v0.7.38...v0.7.39
[0.7.38]: https://codeberg.org/quant1x/quant1x/compare/v0.7.37...v0.7.38
[0.7.37]: https://codeberg.org/quant1x/quant1x/compare/v0.7.36...v0.7.37
[0.7.36]: https://codeberg.org/quant1x/quant1x/compare/v0.7.35...v0.7.36
[0.7.35]: https://codeberg.org/quant1x/quant1x/compare/v0.7.34...v0.7.35
[0.7.34]: https://codeberg.org/quant1x/quant1x/compare/v0.7.33...v0.7.34
[0.7.33]: https://codeberg.org/quant1x/quant1x/compare/v0.7.32...v0.7.33
[0.7.32]: https://codeberg.org/quant1x/quant1x/compare/v0.7.31...v0.7.32
[0.7.31]: https://codeberg.org/quant1x/quant1x/compare/v0.7.30...v0.7.31
[0.7.30]: https://codeberg.org/quant1x/quant1x/compare/v0.7.29...v0.7.30
[0.7.29]: https://codeberg.org/quant1x/quant1x/compare/v0.7.28...v0.7.29
[0.7.28]: https://codeberg.org/quant1x/quant1x/compare/v0.7.27...v0.7.28
[0.7.27]: https://codeberg.org/quant1x/quant1x/compare/v0.7.26...v0.7.27
[0.7.26]: https://codeberg.org/quant1x/quant1x/compare/v0.7.25...v0.7.26
[0.7.25]: https://codeberg.org/quant1x/quant1x/compare/v0.7.24...v0.7.25
[0.7.24]: https://codeberg.org/quant1x/quant1x/compare/v0.7.23...v0.7.24
[0.7.23]: https://codeberg.org/quant1x/quant1x/compare/v0.7.22...v0.7.23
[0.7.22]: https://codeberg.org/quant1x/quant1x/compare/v0.7.21...v0.7.22
[0.7.21]: https://codeberg.org/quant1x/quant1x/compare/v0.7.20...v0.7.21
[0.7.20]: https://codeberg.org/quant1x/quant1x/compare/v0.7.19...v0.7.20
[0.7.19]: https://codeberg.org/quant1x/quant1x/compare/v0.7.18...v0.7.19
[0.7.18]: https://codeberg.org/quant1x/quant1x/compare/v0.7.17...v0.7.18
[0.7.17]: https://codeberg.org/quant1x/quant1x/compare/v0.7.16...v0.7.17
[0.7.16]: https://codeberg.org/quant1x/quant1x/compare/v0.7.15...v0.7.16
[0.7.15]: https://codeberg.org/quant1x/quant1x/compare/v0.7.14...v0.7.15
[0.7.14]: https://codeberg.org/quant1x/quant1x/compare/v0.7.13...v0.7.14
[0.7.13]: https://codeberg.org/quant1x/quant1x/compare/v0.7.12...v0.7.13
[0.7.12]: https://codeberg.org/quant1x/quant1x/compare/v0.7.11...v0.7.12
[0.7.11]: https://codeberg.org/quant1x/quant1x/compare/v0.7.10...v0.7.11
[0.7.10]: https://codeberg.org/quant1x/quant1x/compare/v0.7.9...v0.7.10
[0.7.9]: https://codeberg.org/quant1x/quant1x/compare/v0.7.8...v0.7.9
[0.7.8]: https://codeberg.org/quant1x/quant1x/compare/v0.7.7...v0.7.8
[0.7.7]: https://codeberg.org/quant1x/quant1x/compare/v0.7.6.1...v0.7.7
[0.7.6.1]: https://codeberg.org/quant1x/quant1x/compare/v0.7.6...v0.7.6.1
[0.7.6]: https://codeberg.org/quant1x/quant1x/compare/v0.7.5...v0.7.6
[0.7.5]: https://codeberg.org/quant1x/quant1x/compare/v0.7.4...v0.7.5
[0.7.4]: https://codeberg.org/quant1x/quant1x/compare/v0.7.3...v0.7.4
[0.7.3]: https://codeberg.org/quant1x/quant1x/compare/v0.7.2...v0.7.3
[0.7.2]: https://codeberg.org/quant1x/quant1x/compare/v0.7.1...v0.7.2
[0.7.1]: https://codeberg.org/quant1x/quant1x/compare/v0.7.0...v0.7.1
[0.7.0]: https://codeberg.org/quant1x/quant1x/compare/v0.6.160...v0.7.0
[0.6.160]: https://codeberg.org/quant1x/quant1x/compare/v0.6.159...v0.6.160
[0.6.159]: https://codeberg.org/quant1x/quant1x/compare/v0.6.158...v0.6.159
[0.6.158]: https://codeberg.org/quant1x/quant1x/compare/v0.6.157...v0.6.158
[0.6.157]: https://codeberg.org/quant1x/quant1x/compare/v0.6.156...v0.6.157
[0.6.156]: https://codeberg.org/quant1x/quant1x/compare/v0.6.155...v0.6.156
[0.6.155]: https://codeberg.org/quant1x/quant1x/compare/v0.6.154...v0.6.155
[0.6.154]: https://codeberg.org/quant1x/quant1x/compare/v0.6.153...v0.6.154
[0.6.153]: https://codeberg.org/quant1x/quant1x/compare/v0.6.152...v0.6.153
[0.6.152]: https://codeberg.org/quant1x/quant1x/compare/v0.6.151...v0.6.152
[0.6.151]: https://codeberg.org/quant1x/quant1x/compare/v0.6.150...v0.6.151
[0.6.150]: https://codeberg.org/quant1x/quant1x/compare/v0.6.149...v0.6.150
[0.6.149]: https://codeberg.org/quant1x/quant1x/compare/v0.6.148...v0.6.149
[0.6.148]: https://codeberg.org/quant1x/quant1x/compare/v0.6.147...v0.6.148
[0.6.147]: https://codeberg.org/quant1x/quant1x/compare/v0.6.146...v0.6.147
[0.6.146]: https://codeberg.org/quant1x/quant1x/compare/v0.6.145...v0.6.146
[0.6.145]: https://codeberg.org/quant1x/quant1x/compare/v0.6.144...v0.6.145
[0.6.144]: https://codeberg.org/quant1x/quant1x/compare/v0.6.143...v0.6.144
[0.6.143]: https://codeberg.org/quant1x/quant1x/compare/v0.6.142...v0.6.143
[0.6.142]: https://codeberg.org/quant1x/quant1x/compare/v0.6.141...v0.6.142
[0.6.141]: https://codeberg.org/quant1x/quant1x/compare/v0.6.140...v0.6.141
[0.6.140]: https://codeberg.org/quant1x/quant1x/compare/v0.6.139...v0.6.140
[0.6.139]: https://codeberg.org/quant1x/quant1x/compare/v0.6.138...v0.6.139
[0.6.138]: https://codeberg.org/quant1x/quant1x/compare/v0.6.137...v0.6.138
[0.6.137]: https://codeberg.org/quant1x/quant1x/compare/v0.6.136...v0.6.137
[0.6.136]: https://codeberg.org/quant1x/quant1x/compare/v0.6.135...v0.6.136
[0.6.135]: https://codeberg.org/quant1x/quant1x/compare/v0.6.134...v0.6.135
[0.6.134]: https://codeberg.org/quant1x/quant1x/compare/v0.6.133...v0.6.134
[0.6.133]: https://codeberg.org/quant1x/quant1x/compare/v0.6.132...v0.6.133
[0.6.132]: https://codeberg.org/quant1x/quant1x/compare/v0.6.131...v0.6.132
[0.6.131]: https://codeberg.org/quant1x/quant1x/compare/v0.6.130...v0.6.131
[0.6.130]: https://codeberg.org/quant1x/quant1x/compare/v0.6.129...v0.6.130
[0.6.129]: https://codeberg.org/quant1x/quant1x/compare/v0.6.128...v0.6.129
[0.6.128]: https://codeberg.org/quant1x/quant1x/compare/v0.6.127...v0.6.128
[0.6.127]: https://codeberg.org/quant1x/quant1x/compare/v0.6.126...v0.6.127
[0.6.126]: https://codeberg.org/quant1x/quant1x/compare/v0.6.125...v0.6.126
[0.6.125]: https://codeberg.org/quant1x/quant1x/compare/v0.6.124...v0.6.125
[0.6.124]: https://codeberg.org/quant1x/quant1x/compare/v0.6.123...v0.6.124
[0.6.123]: https://codeberg.org/quant1x/quant1x/compare/v0.6.122...v0.6.123
[0.6.122]: https://codeberg.org/quant1x/quant1x/compare/v0.6.121...v0.6.122
[0.6.121]: https://codeberg.org/quant1x/quant1x/compare/v0.6.120...v0.6.121
[0.6.120]: https://codeberg.org/quant1x/quant1x/compare/v0.6.119...v0.6.120
[0.6.119]: https://codeberg.org/quant1x/quant1x/compare/v0.6.118...v0.6.119
[0.6.118]: https://codeberg.org/quant1x/quant1x/compare/v0.6.117...v0.6.118
[0.6.117]: https://codeberg.org/quant1x/quant1x/compare/v0.6.116...v0.6.117
[0.6.116]: https://codeberg.org/quant1x/quant1x/compare/v0.6.115...v0.6.116
[0.6.115]: https://codeberg.org/quant1x/quant1x/compare/v0.6.114...v0.6.115
[0.6.114]: https://codeberg.org/quant1x/quant1x/compare/v0.6.113...v0.6.114
[0.6.113]: https://codeberg.org/quant1x/quant1x/compare/v0.6.112...v0.6.113
[0.6.112]: https://codeberg.org/quant1x/quant1x/compare/v0.6.111...v0.6.112
[0.6.111]: https://codeberg.org/quant1x/quant1x/compare/v0.6.110...v0.6.111
[0.6.110]: https://codeberg.org/quant1x/quant1x/compare/v0.6.109...v0.6.110
[0.6.109]: https://codeberg.org/quant1x/quant1x/compare/v0.6.108...v0.6.109
[0.6.108]: https://codeberg.org/quant1x/quant1x/compare/v0.6.107...v0.6.108
[0.6.107]: https://codeberg.org/quant1x/quant1x/compare/v0.6.106...v0.6.107
[0.6.106]: https://codeberg.org/quant1x/quant1x/compare/v0.6.105...v0.6.106
[0.6.105]: https://codeberg.org/quant1x/quant1x/compare/v0.6.104...v0.6.105
[0.6.104]: https://codeberg.org/quant1x/quant1x/compare/v0.6.103...v0.6.104
[0.6.103]: https://codeberg.org/quant1x/quant1x/compare/v0.6.102...v0.6.103
[0.6.102]: https://codeberg.org/quant1x/quant1x/compare/v0.6.101...v0.6.102
[0.6.101]: https://codeberg.org/quant1x/quant1x/compare/v0.6.100...v0.6.101
[0.6.100]: https://codeberg.org/quant1x/quant1x/compare/v0.6.99...v0.6.100
[0.6.99]: https://codeberg.org/quant1x/quant1x/compare/v0.6.98...v0.6.99
[0.6.98]: https://codeberg.org/quant1x/quant1x/compare/v0.6.97...v0.6.98
[0.6.97]: https://codeberg.org/quant1x/quant1x/compare/v0.6.96...v0.6.97
[0.6.96]: https://codeberg.org/quant1x/quant1x/compare/v0.6.95...v0.6.96
[0.6.95]: https://codeberg.org/quant1x/quant1x/compare/v0.6.94...v0.6.95
[0.6.94]: https://codeberg.org/quant1x/quant1x/compare/v0.6.93...v0.6.94
[0.6.93]: https://codeberg.org/quant1x/quant1x/compare/v0.6.92...v0.6.93
[0.6.92]: https://codeberg.org/quant1x/quant1x/compare/v0.6.91...v0.6.92
[0.6.91]: https://codeberg.org/quant1x/quant1x/compare/v0.6.90...v0.6.91
[0.6.90]: https://codeberg.org/quant1x/quant1x/compare/v0.6.89...v0.6.90
[0.6.89]: https://codeberg.org/quant1x/quant1x/compare/v0.6.88...v0.6.89
[0.6.88]: https://codeberg.org/quant1x/quant1x/compare/v0.6.87...v0.6.88
[0.6.87]: https://codeberg.org/quant1x/quant1x/compare/v0.6.86...v0.6.87
[0.6.86]: https://codeberg.org/quant1x/quant1x/compare/v0.6.85...v0.6.86
[0.6.85]: https://codeberg.org/quant1x/quant1x/compare/v0.6.84...v0.6.85
[0.6.84]: https://codeberg.org/quant1x/quant1x/compare/v0.6.83...v0.6.84
[0.6.83]: https://codeberg.org/quant1x/quant1x/compare/v0.6.82...v0.6.83
[0.6.82]: https://codeberg.org/quant1x/quant1x/compare/v0.6.81...v0.6.82
[0.6.81]: https://codeberg.org/quant1x/quant1x/compare/v0.6.80...v0.6.81
[0.6.80]: https://codeberg.org/quant1x/quant1x/compare/v0.6.79...v0.6.80
[0.6.79]: https://codeberg.org/quant1x/quant1x/compare/v0.6.78...v0.6.79
[0.6.78]: https://codeberg.org/quant1x/quant1x/compare/v0.6.77...v0.6.78
[0.6.77]: https://codeberg.org/quant1x/quant1x/compare/v0.6.76...v0.6.77
[0.6.76]: https://codeberg.org/quant1x/quant1x/compare/v0.6.75...v0.6.76
[0.6.75]: https://codeberg.org/quant1x/quant1x/compare/v0.6.74...v0.6.75
[0.6.74]: https://codeberg.org/quant1x/quant1x/compare/v0.6.73...v0.6.74
[0.6.73]: https://codeberg.org/quant1x/quant1x/compare/v0.6.72...v0.6.73
[0.6.72]: https://codeberg.org/quant1x/quant1x/compare/v0.6.71...v0.6.72
[0.6.71]: https://codeberg.org/quant1x/quant1x/compare/v0.6.70...v0.6.71
[0.6.70]: https://codeberg.org/quant1x/quant1x/compare/v0.6.69...v0.6.70
[0.6.69]: https://codeberg.org/quant1x/quant1x/compare/v0.6.68...v0.6.69
[0.6.68]: https://codeberg.org/quant1x/quant1x/compare/v0.6.67...v0.6.68
[0.6.67]: https://codeberg.org/quant1x/quant1x/compare/v0.6.66...v0.6.67
[0.6.66]: https://codeberg.org/quant1x/quant1x/compare/v0.6.65...v0.6.66
[0.6.65]: https://codeberg.org/quant1x/quant1x/compare/v0.6.64...v0.6.65
[0.6.64]: https://codeberg.org/quant1x/quant1x/compare/v0.6.63...v0.6.64
[0.6.63]: https://codeberg.org/quant1x/quant1x/compare/v0.6.62...v0.6.63
[0.6.62]: https://codeberg.org/quant1x/quant1x/compare/v0.6.61...v0.6.62
[0.6.61]: https://codeberg.org/quant1x/quant1x/compare/v0.6.60...v0.6.61
[0.6.60]: https://codeberg.org/quant1x/quant1x/compare/v0.6.59...v0.6.60
[0.6.59]: https://codeberg.org/quant1x/quant1x/compare/v0.6.58...v0.6.59
[0.6.58]: https://codeberg.org/quant1x/quant1x/compare/v0.6.57...v0.6.58
[0.6.57]: https://codeberg.org/quant1x/quant1x/compare/v0.6.56...v0.6.57
[0.6.56]: https://codeberg.org/quant1x/quant1x/compare/v0.6.55...v0.6.56
[0.6.55]: https://codeberg.org/quant1x/quant1x/compare/v0.6.54...v0.6.55
[0.6.54]: https://codeberg.org/quant1x/quant1x/compare/v0.6.53...v0.6.54
[0.6.53]: https://codeberg.org/quant1x/quant1x/compare/v0.6.52...v0.6.53
[0.6.52]: https://codeberg.org/quant1x/quant1x/compare/v0.6.51...v0.6.52
[0.6.51]: https://codeberg.org/quant1x/quant1x/compare/v0.6.50...v0.6.51
[0.6.50]: https://codeberg.org/quant1x/quant1x/compare/v0.6.49...v0.6.50
[0.6.49]: https://codeberg.org/quant1x/quant1x/compare/v0.6.48...v0.6.49
[0.6.48]: https://codeberg.org/quant1x/quant1x/compare/v0.6.47...v0.6.48
[0.6.47]: https://codeberg.org/quant1x/quant1x/compare/v0.6.46...v0.6.47
[0.6.46]: https://codeberg.org/quant1x/quant1x/compare/v0.6.45...v0.6.46
[0.6.45]: https://codeberg.org/quant1x/quant1x/compare/v0.6.44...v0.6.45
[0.6.44]: https://codeberg.org/quant1x/quant1x/compare/v0.6.43...v0.6.44
[0.6.43]: https://codeberg.org/quant1x/quant1x/compare/v0.6.42...v0.6.43
[0.6.42]: https://codeberg.org/quant1x/quant1x/compare/v0.6.41...v0.6.42
[0.6.41]: https://codeberg.org/quant1x/quant1x/compare/v0.6.40...v0.6.41
[0.6.40]: https://codeberg.org/quant1x/quant1x/compare/v0.6.39...v0.6.40
[0.6.39]: https://codeberg.org/quant1x/quant1x/compare/v0.6.38...v0.6.39
[0.6.38]: https://codeberg.org/quant1x/quant1x/compare/v0.6.37...v0.6.38
[0.6.37]: https://codeberg.org/quant1x/quant1x/compare/v0.6.36...v0.6.37
[0.6.36]: https://codeberg.org/quant1x/quant1x/compare/v0.6.35...v0.6.36
[0.6.35]: https://codeberg.org/quant1x/quant1x/compare/v0.6.34...v0.6.35
[0.6.34]: https://codeberg.org/quant1x/quant1x/compare/v0.6.33...v0.6.34
[0.6.33]: https://codeberg.org/quant1x/quant1x/compare/v0.6.32...v0.6.33
[0.6.32]: https://codeberg.org/quant1x/quant1x/compare/v0.6.31...v0.6.32
[0.6.31]: https://codeberg.org/quant1x/quant1x/compare/v0.6.30...v0.6.31
[0.6.30]: https://codeberg.org/quant1x/quant1x/compare/v0.6.29...v0.6.30
[0.6.29]: https://codeberg.org/quant1x/quant1x/compare/v0.6.28...v0.6.29
[0.6.28]: https://codeberg.org/quant1x/quant1x/compare/v0.6.27...v0.6.28
[0.6.27]: https://codeberg.org/quant1x/quant1x/compare/v0.6.26...v0.6.27
[0.6.26]: https://codeberg.org/quant1x/quant1x/compare/v0.6.25...v0.6.26
[0.6.25]: https://codeberg.org/quant1x/quant1x/compare/v0.6.24...v0.6.25
[0.6.24]: https://codeberg.org/quant1x/quant1x/compare/v0.6.23...v0.6.24
[0.6.23]: https://codeberg.org/quant1x/quant1x/compare/v0.6.22...v0.6.23
[0.6.22]: https://codeberg.org/quant1x/quant1x/compare/v0.6.21...v0.6.22
[0.6.21]: https://codeberg.org/quant1x/quant1x/compare/v0.6.20...v0.6.21
[0.6.20]: https://codeberg.org/quant1x/quant1x/compare/v0.6.19...v0.6.20
[0.6.19]: https://codeberg.org/quant1x/quant1x/compare/v0.6.18...v0.6.19
[0.6.18]: https://codeberg.org/quant1x/quant1x/compare/v0.6.17...v0.6.18
[0.6.17]: https://codeberg.org/quant1x/quant1x/compare/v0.6.16...v0.6.17
[0.6.16]: https://codeberg.org/quant1x/quant1x/compare/v0.6.15...v0.6.16
[0.6.15]: https://codeberg.org/quant1x/quant1x/compare/v0.6.14...v0.6.15
[0.6.14]: https://codeberg.org/quant1x/quant1x/compare/v0.6.13...v0.6.14
[0.6.13]: https://codeberg.org/quant1x/quant1x/compare/v0.6.12...v0.6.13
[0.6.12]: https://codeberg.org/quant1x/quant1x/compare/v0.6.11...v0.6.12
[0.6.11]: https://codeberg.org/quant1x/quant1x/compare/v0.6.10...v0.6.11
[0.6.10]: https://codeberg.org/quant1x/quant1x/compare/v0.6.9...v0.6.10
[0.6.9]: https://codeberg.org/quant1x/quant1x/compare/v0.6.8...v0.6.9
[0.6.8]: https://codeberg.org/quant1x/quant1x/compare/v0.6.7...v0.6.8
[0.6.7]: https://codeberg.org/quant1x/quant1x/compare/v0.6.6...v0.6.7
[0.6.6]: https://codeberg.org/quant1x/quant1x/compare/v0.6.5...v0.6.6
[0.6.5]: https://codeberg.org/quant1x/quant1x/compare/v0.6.4...v0.6.5
[0.6.4]: https://codeberg.org/quant1x/quant1x/compare/v0.6.3...v0.6.4
[0.6.3]: https://codeberg.org/quant1x/quant1x/compare/v0.6.2...v0.6.3
[0.6.2]: https://codeberg.org/quant1x/quant1x/compare/v0.6.1...v0.6.2
[0.6.1]: https://codeberg.org/quant1x/quant1x/compare/v0.6.0...v0.6.1
[0.6.0]: https://codeberg.org/quant1x/quant1x/compare/v0.5.9...v0.6.0
[0.5.9]: https://codeberg.org/quant1x/quant1x/compare/v0.5.8...v0.5.9
[0.5.8]: https://codeberg.org/quant1x/quant1x/compare/v0.5.7...v0.5.8
[0.5.7]: https://codeberg.org/quant1x/quant1x/compare/v0.5.6...v0.5.7
[0.5.6]: https://codeberg.org/quant1x/quant1x/compare/v0.5.5...v0.5.6
[0.5.5]: https://codeberg.org/quant1x/quant1x/compare/v0.5.4...v0.5.5
[0.5.4]: https://codeberg.org/quant1x/quant1x/compare/v0.5.3...v0.5.4
[0.5.3]: https://codeberg.org/quant1x/quant1x/compare/v0.5.2...v0.5.3
[0.5.2]: https://codeberg.org/quant1x/quant1x/compare/v0.5.1...v0.5.2
[0.5.1]: https://codeberg.org/quant1x/quant1x/compare/v0.5.0...v0.5.1
[0.5.0]: https://codeberg.org/quant1x/quant1x/compare/v0.1.13...v0.5.0
[0.1.13]: https://codeberg.org/quant1x/quant1x/compare/v0.1.12...v0.1.13
[0.1.12]: https://codeberg.org/quant1x/quant1x/compare/v0.1.11...v0.1.12
[0.1.11]: https://codeberg.org/quant1x/quant1x/compare/v0.1.10...v0.1.11
[0.1.10]: https://codeberg.org/quant1x/quant1x/compare/v0.1.9...v0.1.10
[0.1.9]: https://codeberg.org/quant1x/quant1x/compare/v0.1.8...v0.1.9
[0.1.8]: https://codeberg.org/quant1x/quant1x/compare/v0.1.7...v0.1.8
[0.1.7]: https://codeberg.org/quant1x/quant1x/compare/v0.1.6...v0.1.7
[0.1.6]: https://codeberg.org/quant1x/quant1x/compare/v0.1.5...v0.1.6
[0.1.5]: https://codeberg.org/quant1x/quant1x/compare/v0.1.4...v0.1.5
[0.1.4]: https://codeberg.org/quant1x/quant1x/compare/v0.1.3...v0.1.4
[0.1.3]: https://codeberg.org/quant1x/quant1x/compare/v0.1.2...v0.1.3
[0.1.2]: https://codeberg.org/quant1x/quant1x/compare/v0.1.1...v0.1.2
[0.1.1]: https://codeberg.org/quant1x/quant1x/compare/v0.1.0...v0.1.1

[0.1.0]: https://codeberg.org/quant1x/quant1x/releases/tag/v0.1.0
