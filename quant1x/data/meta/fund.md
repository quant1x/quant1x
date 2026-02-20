# 基金类型编码规范（16-bit 整型）

本规范定义了一种 **16 位无符号整型（`uint16`）** 编码方案，用于紧凑、高效、可读地表示公募/私募基金的核心特征。  
编码遵循 **“从宏观到微观”** 的层级顺序，**资产类别置于最低位**，确保在默认配置下整型值直接等于资产类别编号，提升可读性与调试效率。

---

## 一、设计原则

- **正交维度**：每个属性独立，无语义重叠  
- **默认暴露**：当所有非资产字段为默认值（0）时，`code == asset_class`  
- **人类可读**：常见基金类型（如股票型）编码值小且直观  
- **面向未来**：关键维度预留扩展空间  
- **内存对齐**：16 bits = 2 字节，高效存储与传输

---

## 二、位布局（共 16 bits）

| 序号 | 维度名称         | 枚举类                     | 位宽 | 位位置（LSB=0） | 默认值 | 说明 |
|------|------------------|----------------------------|------|------------------|--------|------|
| 1    | 申赎机制         | `FundOpenness`             | 2    | [15:14]          | `OPEN = 0` | 最宏观：开放式 / 封闭式 / 定期开放 |
| 2    | 投资策略         | `FundStrategy`             | 1    | [13]             | `ACTIVE = 0` | 主动 / 被动（指数） |
| 3    | 发行类型         | `OfferingType`             | 1    | [12]             | `PUBLIC = 0` | 公募 / 私募 |
| 4    | 基金结构         | `FundStructure`            | 2    | [11:10]          | `DIRECT = 0` | 直投 / FOF / MOM |
| 5    | 投资范围         | `FundInvestmentScope`      | 2    | [9:8]            | `DOMESTIC = 0` | 境内 / QDII / QDLP |
| 6    | 交易机制         | `FundTradingMechanism`     | 3    | [7:5]            | `OTC_ONLY = 0` | 场外 / 上市 / 交易型（ETF） |
| 7    | **资产类别**     | **`FundAssetClass`**       | **5**| **[4:0]**        | —      | **最微观，直接暴露于低 5 位** |

> ✅ 总计：2 + 1 + 1 + 2 + 2 + 3 + 5 = **16 bits**

---

## 三、枚举定义

```python
from enum import IntEnum

# 1. 申赎机制 (2 bits, [15:14])
class FundOpenness(IntEnum):
    OPEN     = 0   # 开放式
    CLOSED   = 1   # 封闭式
    INTERVAL = 2   # 定期开放

# 2. 投资策略 (1 bit, [13])
class FundStrategy(IntEnum):
    ACTIVE  = 0   # 主动管理
    PASSIVE = 1   # 被动（指数跟踪）

# 3. 发行类型 (1 bit, [12])
class OfferingType(IntEnum):
    PUBLIC  = 0   # 公募
    PRIVATE = 1   # 私募

# 4. 基金结构 (2 bits, [11:10])
class FundStructure(IntEnum):
    DIRECT = 0    # 直接投资
    FOF    = 1    # 基金中基金
    MOM    = 2    # 管理人中管理人

# 5. 投资范围 (2 bits, [9:8])
class FundInvestmentScope(IntEnum):
    DOMESTIC = 0  # 境内
    QDII     = 1  # 合格境内机构投资者
    QDLP     = 2  # 合格境内有限合伙人

# 6. 交易机制 (3 bits, [7:5])
class FundTradingMechanism(IntEnum):
    OTC_ONLY  = 0   # 仅场外（普通基金）
    LISTED    = 1   # 上市交易（LOF、封闭式等）
    TRADABLE  = 2   # 交易型（ETF）

# 7. 资产类别 (5 bits, [4:0])
class FundAssetClass(IntEnum):
    UNKNOWN   = 0
    MONEY     = 1   # 货币
    BOND      = 2   # 债券
    STOCK     = 3   # 股票
    MIXED     = 4   # 混合
    COMMODITY = 5   # 商品
    REITs     = 6   # 不动产投资信托
    OTHER     = 7
    # 预留 8–31：支持 ESG、衍生品、加密资产等未来扩展