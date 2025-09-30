import csv
import os
import statistics
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

CSV_PATH = os.path.join(os.path.dirname(__file__), 'perf_samples_200.csv')
HIST_PNG = os.path.join(os.path.dirname(__file__), 'perf_hist.png')
BOX_PNG = os.path.join(os.path.dirname(__file__), 'perf_box.png')
REPORT_MD = os.path.join(os.path.dirname(__file__), 'perf_report.md')

# Read samples
samples = []
with open(CSV_PATH, 'r', encoding='utf-8') as f:
    reader = csv.reader(f)
    for row in reader:
        if not row: 
            continue
        try:
            v = float(row[0])
            samples.append(v)
        except Exception:
            continue

if not samples:
    raise SystemExit('No numeric samples found in ' + CSV_PATH)

# stats
count = len(samples)
mean = statistics.mean(samples)
median = statistics.median(samples)
stdev = statistics.stdev(samples) if count > 1 else 0.0
p25 = np.percentile(samples, 25)
p75 = np.percentile(samples, 75)
min_v = min(samples)
max_v = max(samples)

# histogram
plt.figure(figsize=(8,4))
plt.hist(samples, bins=30, color='#2b8cbe', edgecolor='black')
plt.title('Vyukov MPMC throughput (ops/sec) - histogram')
plt.xlabel('ops/sec')
plt.ylabel('frequency')
plt.grid(axis='y', alpha=0.3)
plt.tight_layout()
plt.savefig(HIST_PNG, dpi=150)
plt.close()

# boxplot
plt.figure(figsize=(6,3))
plt.boxplot(samples, vert=False)
plt.title('Vyukov MPMC throughput (ops/sec) - boxplot')
plt.xlabel('ops/sec')
plt.tight_layout()
plt.savefig(BOX_PNG, dpi=150)
plt.close()

# write markdown report
with open(REPORT_MD, 'w', encoding='utf-8') as f:
    f.write('# Vyukov MPMC throughput report\n\n')
    f.write('数据文件: `perf_samples_200.csv`\n\n')
    f.write('## 统计摘要\n\n')
    f.write(f'- 样本数: {count}\n')
    f.write(f'- 平均 (mean): {mean:.2f} ops/sec\n')
    f.write(f'- 中位数 (median): {median:.2f} ops/sec\n')
    f.write(f'- 标准差 (stddev): {stdev:.2f} ops/sec\n')
    f.write(f'- 最小值: {min_v:.2f} ops/sec\n')
    f.write(f'- 最大值: {max_v:.2f} ops/sec\n')
    f.write(f'- 25th percentile: {p25:.2f} ops/sec\n')
    f.write(f'- 75th percentile: {p75:.2f} ops/sec\n\n')
    f.write('## 直方图\n\n')
    f.write('![](perf_hist.png)\n\n')
    f.write('## 箱形图\n\n')
    f.write('![](perf_box.png)\n\n')
    f.write('## 说明\n\n')
    f.write('该报告基于在本机运行 `vyukov_runner` 收集的 200 条样本。吞吐受系统调度、CPU 频率、电源管理等影响，建议在受控环境下重复测量以获得更稳健结果。\n')

print('Wrote', HIST_PNG, BOX_PNG, REPORT_MD)
