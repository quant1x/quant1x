import pandas as pd
import os
import glob
import matplotlib.pyplot as plt
plt.rcParams["font.sans-serif"] = ["SimHei"]
plt.rcParams["axes.unicode_minus"] = False
from quant1x import cache, exchange

# 1. 读取并合并数据
#881075 贵金属
sector_code = '880675'
sector_name = cache.stock_name(sector_code)
list = cache.get_sector_constituents(sector_code)
df_combined = pd.DataFrame()

for symbol in list:
    df = cache.klines(symbol)
    if df is None:
        continue
    df = df[-55:]
    df = df[['date','volume']]
    df.rename(columns={'volume':symbol}, inplace=True)
    df_combined = pd.merge(df_combined, df, on="date", how="outer") if not df_combined.empty else df

df_combined.fillna(0, inplace=True)
df_combined.set_index("date", inplace=True)
df_combined.sort_index(inplace=True)

# 2. 计算HHI
def calculate_hhi(row):
    total_volume = row.sum()
    return 0 if total_volume == 0 else (row / total_volume).pow(2).sum() * 10000

df_combined["HHI"] = df_combined.iloc[:, :-1].apply(calculate_hhi, axis=1)

# 3. 动态阈值
df_combined["HHI_MA20"] = df_combined["HHI"].rolling(20).mean()
df_combined["HHI_STD20"] = df_combined["HHI"].rolling(20).std()
df_combined["Upper_Band"] = df_combined["HHI_MA20"] + 2 * df_combined["HHI_STD20"]

# 4. 可视化
plt.figure(figsize=(14, 6))
plt.plot(df_combined.index, df_combined["HHI"], label="HHI", color="blue", alpha=0.7)
plt.plot(df_combined.index, df_combined["Upper_Band"], label="风险阈值", linestyle="--", color="red")
plt.title(f'{sector_name}({sector_code})板块成交量集中度分析')
plt.xlabel('日期')
plt.xticks(rotation=90)
plt.ylabel("HHI")
plt.legend()
plt.grid(True)
plt.show()
print(df_combined['HHI'])