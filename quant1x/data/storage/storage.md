# Storage 模块设计文档

## 概述

`storage` 模块提供了一套基于 **泛型抽象类** 的文件存储框架，专用于单一数据类型的 CSV 文件持久化。模块遵循 **初始化 → 更新 → 加载/保存** 的生命周期模式，子类只需实现数据判断逻辑，无需关心底层 I/O 细节。

---

## 类层次结构

```
FileStorage[T]                    (抽象基类, Generic[T])
    ├── BasedataFileStorage[T]    (基础数据存储, 关联 Instrument)
    └── MetaFileStorage[T]        (元数据存储, 文件名自动生成)
```

---

## FileStorage[T] — 文件存储接口

### 类型参数

| 参数 | 说明 |
|------|------|
| `T` | 数据类型（dataclass/struct），CSV 行与该类型字段一一映射 |

### 生命周期

```
                 should_initialize()?
                  /               \
                是                 否
                 |                 |
              update()    should_update()?
                 |          /           \
                 +<--------是           否
                 |          |            |
                 +<---update()          |
                 |                      |
            load() <-------------------+
                 |
              List[T]
```

### 抽象方法（子类必须实现）

| 方法 | 签名 | 说明 |
|------|------|------|
| `file_name()` | `() -> str` | 返回 CSV 文件路径 |
| `should_initialize(timestamp?)` | `(Timestamp) -> bool` | 判断文件是否需要初始化（如文件不存在） |
| `should_update(timestamp?)` | `(Timestamp) -> bool` | 判断数据是否需要更新（如数据过期） |
| `update()` | `() -> None` | 更新数据到文件（类型已固定，无需参数） |

### 具体方法（基类提供）

| 方法 | 签名 | 说明 |
|------|------|------|
| `load()` | `() -> List[T]` | 从 CSV 加载数据，返回类型 T 的列表 |
| `save(data)` | `(List[T]) -> None` | 将数据列表写入 CSV |
| `checkout()` | `() -> List[T]` | **自动更新 + 加载**：先判断是否需要初始化/更新，再加载返回 |

### checkout() 流程图

```
checkout()
    |
    ├── should_initialize() == True? ──→ update()
    ├── should_update()     == True? ──→ update()
    |
    └── load() → List[T]
```

### 构造函数

```python
def __init__(self, data_type: Type[T]) -> None:
    self._data_type = data_type   # 绑定的数据类型
    self._file_name = self.file_name()  # 调用子类实现的 file_name()
```

关键设计：构造函数中立即调用 `file_name()`，将文件名缓存到 `_file_name`，后续 `load()`/`save()` 直接使用。

---

## BasedataFileStorage[T] — 基础数据文件存储

继承自 `FileStorage[T]`，额外绑定一个 **Instrument**（证券标的）。

### 额外属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `_inst` | `Instrument` | 关联的证券标的（股票代码、交易所等） |

### 构造函数

```python
def __init__(self, data_type: Type[T], inst: Instrument) -> None:
    self._inst = inst
    super().__init__(data_type)
```

### 典型使用场景

用于存储与特定证券相关的基础数据，如 K 线数据、分时数据、财务数据等。子类实现 `file_name()` 时通常会结合 `_inst.symbol()` 生成路径。

```python
class DayKlineStorage(BasedataFileStorage[DayKline]):
    def file_name(self) -> str:
        return f"cache/day/{self._inst.symbol()}.csv"

    def should_initialize(self, timestamp=Timestamp.now()) -> bool:
        return not os.path.exists(self._file_name)

    def should_update(self, timestamp=Timestamp.now()) -> bool:
        mtime = os.path.getmtime(self._file_name)
        return timestamp.value() - mtime > 86400000  # 超过1天

    def update(self) -> None:
        data = fetch_day_kline(self._inst)
        self.save(data)
```

---

## MetaFileStorage[T] — 元数据文件存储

继承自 `FileStorage[T]`，自动根据类型名生成文件名。

### 已实现方法

| 方法 | 实现 |
|------|------|
| `file_name()` | `"{data_type.__name__}.csv"`，如 `SecurityInfo.csv` |

### 构造函数

```python
def __init__(self, data_type: Type[T]) -> None:
    super().__init__(data_type)
```

### 典型使用场景

用于存储与类型绑定的元数据，如证券列表、行业分类、日历数据等。文件名由类型名自动推导，无需手动指定。

```python
class SecurityListStorage(MetaFileStorage[SecurityInfo]):
    def should_initialize(self, timestamp=Timestamp.now()) -> bool:
        return not os.path.exists(self._file_name)

    def should_update(self, timestamp=Timestamp.now()) -> bool:
        return timestamp.only_date() != self._last_update_date

    def update(self) -> None:
        data = fetch_security_list()
        self.save(data)
```

---

## 各语言实现对照

| 概念 | Python | C++ | Rust | Go |
|------|--------|-----|------|-----|
| 抽象基类 | `class FileStorage(ABC, Generic[T])` | `template<typename T> class FileStorage` (纯虚方法) | `trait FileStorage<T>` | `interface FileStorage[T any]` |
| 类型参数 | `Generic[T]` / `Type[T]` | `template<typename T>` | 泛型 trait `T: Serialize + DeserializeOwned` | 泛型 `[T any]` |
| 基础数据类 | `BasedataFileStorage(FileStorage)` | `BasedataFileStorage<T> : public FileStorage<T>` | `struct BasedataFileStorage<T>` | `struct BasedataFileStorage[T]` |
| 元数据类 | `MetaFileStorage(FileStorage)` | `MetaFileStorage<T> : public FileStorage<T>` | `struct MetaFileStorage<T>` | `struct MetaFileStorage[T]` |
| CSV 读取 | `csv_to_slice()` | `encoding::csv::csv_to_slices<T>()` | `csv_to_vec<T>()` (内置) | `CsvToSlice()` |
| CSV 写入 | `slice_to_csv()` | `encoding::csv::slices_to_csv<T>()` | `vec_to_csv<T>()` (内置) | `SliceToCsv()` |
| 时间戳参数 | `Timestamp.now()` 默认值 | 无默认值（虚方法限制），checkout 中提供默认值 | `Option<Timestamp>` | `...meta.Timestamp` 可变参数 |

---

## 核心设计原则

1. **单一类型**：每个 `FileStorage` 实例只处理一种数据类型，类型在构造时固定。
2. **关注点分离**：子类只关心 *何时更新* 和 *如何更新*，不关心 *如何读写*。
3. **文件名缓存**：构造时调用 `file_name()` 并缓存，避免重复计算。
4. **checkout 模式**：`checkout()` 封装了最常见的"获取最新数据"场景，自动处理初始化→更新→加载全流程。
5. **Instrument 绑定**：`BasedataFileStorage` 将证券标的作为一等成员，便于子类在文件名和更新逻辑中引用。

---

## 文件映射

```
quant1x/data/storage/
├── storage.py       # Python 实现（参考源）
├── storage.h        # C++ 实现
├── storage.rs       # Rust 实现
├── storage.go       # Go 实现
├── mod.rs           # Rust 模块声明
├── csv.py           # Python CSV 辅助
├── csv.go           # Go CSV 辅助
└── __init__.py      # Python 包导出
```
