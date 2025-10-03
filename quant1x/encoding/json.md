# encoding::json 使用说明（中文）

本文档概述项目中 `encoding::json` 模块的特性、行为契约、常见用法与注意事项。示例基于仓库现有实现（位于 `quant1x/encoding/json.h`），并配套单元测试在 `tests/tdd-encoding-json.cpp`。

## 简介

`encoding::json` 是一个轻量的 JSON 序列化 / 反序列化工具封装，主要特点是：

- 对 C++ 聚合类型（struct）使用 Boost.PFR 做字段级反射（自动按字段顺序序列化/反序列化）。
- 对常见 STL 容器（序列与关联容器）和基础类型提供专门处理路径，避免对容器误用 PFR。
- 使用 `nlohmann::json` 作为中间 JSON 表示（下文简称 `json_t`）。
- 提供 `strict` 模式用于对缺失字段或类型不匹配进行更严格的错误检查。
- 支持 `std::optional` 的两种序列化策略：默认（不输出缺失的 optional 字段），和显式输出 `null`（可通过 RAII 辅助打开）。

## 支持的类型（当前实现）

- 聚合（aggregate）类型：通过 Boost.PFR 反射字段（struct）
- 基本类型：整数、浮点、布尔、`std::string`
- 枚举类型：按底层整数值序列化为数值（反序列化时按整数读回并静态转换）
- 容器：
  - 顺序容器：`std::vector<T>`, `std::deque<T>`, `std::array<T, N>`
  - 关联容器：`std::map<Key, Value>`, `std::unordered_map<Key, Value>`
- 可选：`std::optional<T>`
- 嵌套类型：上述类型可以嵌套组合（例如 `vector<struct>`，`map<string, vector<struct>>`）

> 注：如需支持更多容器（例如 `forward_list`, `set`, `multimap` 等），可按现有模式添加类型特征和专门的 (de)serialize 辅助函数并增加测试。

## API 契约（简化）

- `json_t serialize<T>(const T& obj, bool strict = false);`
  - 输入：任意受支持的类型 `T`。
  - 输出：对应的 `nlohmann::json`（即 `json_t`）。
  - `strict`：在序列化过程中通常无大影响（保留为统一签名）。

- `T deserialize<T>(const json_t& j, bool strict = false);`
  - 从 `json_t` 反序列化为 `T`。
  - 如果 `strict == true`：缺失字段或类型不匹配会抛出异常（`std::runtime_error` 或 `nlohmann::json::type_error`）；否则尽量使用类型的默认值或 `std::optional` 的 empty 状态。

- `serialize_field(json_t& out, const T& value, bool strict)`
  - 内部用来序列化单个字段，`std::optional` 的空值在默认策略下会被省略（不写入对象），而当显式打开“输出 null”策略时会写入 `null`。

- 可配置符：
  - `inline thread_local bool g_serialize_optional_as_null = false;`：控制是否将 `std::optional` 的 empty 序列化为 JSON `null`（默认 `false`，即省略字段）。
  - RAII 辅助：`OptionalNullGuard`（构造时设置 flag，析构时恢复），并提供便利包装 `serialize_with_optional_null(obj)` 来在一次序列化调用中临时把 empty optional 输出为 `null`。

## 常见用法示例

下面示例中的 `json_t` 就是 `nlohmann::json`：

1. 序列化 / 反序列化聚合（struct）

```cpp
struct Person {
    std::string name;
    int age;
    std::optional<double> score;
};

Person p{"Alice", 30, 88.5};
json_t j = json::serialize(p);
// j -> { "name": "Alice", "age": 30, "score": 88.5 }

Person p2 = json::deserialize<Person>(j);
```

2. 序列化 vector / map

```cpp
std::vector<Person> v = { p, Person{"Bob", 20, std::nullopt} };
json_t jv = json::serialize(v);
// jv 是数组，每个元素为对象

std::map<std::string,int> m{{"a",1},{"b",2}};
json_t jm = json::serialize(m);
// jm 是对象 {"a":1, "b":2}
```

3. optional 的 `omit`（默认）与 `null`（显式）行为

默认行为：空的 `std::optional` 字段不会写入到对象中：

```cpp
Person p{"Bob", 20, std::nullopt};
json_t j = json::serialize(p);
// j 可能不包含 "score" 字段
```

如需在序列化时显式写入 `null`（例如与某些后端契约对齐），使用 RAII 辅助：

```cpp
json_t j2 = json::serialize_with_optional_null(p); // 临时把 optional 的空值序列化为 null
// 或者手动：
// OptionalNullGuard g(true);
// auto j2 = json::serialize(p);
```

4. 严格模式

- 反序列化时：
  - 若 `strict == true`，缺失必需字段会抛出 `std::runtime_error`。
  - 类型不匹配（例如 JSON 字段是字符串但目标类型为数字）会抛出 `nlohmann::json::type_error`。

```cpp
Person p3 = json::deserialize<Person>(j_missing_field, true); // 可能抛异常
```

## 错误处理与边界情况

- 尝试对容器调用 PFR 会导致编译错误／静态断言。当前实现通过编译期类型特征分流：容器走专门路径，只有聚合结构体才会进入 Boost.PFR 路径。
- 反序列化时要注意：关联容器的键类型必须可从 JSON 字符串/数字正确读取并可作为键（例如 `std::string`、整型）。
- `std::array<T, N>` 的反序列化会检查数组长度是否匹配（不匹配时会抛出或按实现处理，视严格实现细节）。
- `enum` 采用整数表示：序列化为数值，反序列化时按整数填充枚举（请确保枚举值语义正确）。

## 扩展点

- 添加对新容器（比如 `std::set`, `std::multimap`, `std::forward_list`）的支持：需要
  1. 添加对应的 type trait（像 `is_set_v`）
  2. 在 `serialize<T>` / `deserialize<T>` 的分流中添加条件分支
  3. 提供相应的具体序列化/反序列化实现
  4. 增加单元测试覆盖

- 可将当前的线程局部配置 `g_serialize_optional_as_null` 改为显式 `SerializeOptions` 上下文对象，从而避免线程局部状态并允许 per-call 配置。

## 测试与运行

仓库已包含一组 Catch2 单元测试（位于 `tests/tdd-encoding-json.cpp`），构建并运行方法（PowerShell）：

```powershell
# 在仓库根目录执行一次（如尚未建立构建目录）
mkdir build; cd build
cmake ..
cmake --build . --config Debug --target catch2-tdd-encoding-json -- -j 4
# 运行测试可直接调用生成的测试二进制（示例路径）
.\tests\catch2-tdd-encoding-json.exe -s
# 或者（如果在仓库根目录）
# D:\projects\quant1x\quant1x\build\tests\catch2-tdd-encoding-json.exe -s
```

测试覆盖了：聚合 roundtrip、vector/map/deque/array/unordered_map 支持、optional 的 omit vs null 行为、严格模式异常等。

## 依赖与编译要求

- C++20 编译器（GCC/Clang/MSVC 支持 C++20 的版本）
- Boost.PFR
- nlohmann::json
- Catch2（用于单元测试）
- CMake 构建系统（仓库使用 CMakeLists 和测试注册宏）

## 小结与建议

- `encoding::json` 在默认情况下对聚合类型提供零配置的 JSON 支持，这对快速序列化/反序列化非常方便。
- 对容器使用专门分流避免了 Boost.PFR 对非聚合类型的误用，这是实现的关键修复点。
- 如果你偏好显式、可配置的序列化策略（例如对 optional 的处理、字段命名策略、忽略空容器等），建议引入一个 `SerializeOptions` / `JsonContext` 对象代替线程局部标志，并把其作为参数传递到 `serialize` / `deserialize` 中。

---

文件位置：`quant1x/encoding/json.h`
测试：`tests/tdd-encoding-json.cpp`

如需我把文档移动到其他目录、调整格式为 README 风格或生成中文/英文双语版本，或把 `SerializeOptions` 作为 API 改造并更新测试，我可以继续实现。
