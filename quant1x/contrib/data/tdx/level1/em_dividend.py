import requests
import json, csv

url = f"https://datacenter.eastmoney.com/securities/api/data/v1/get"
#params = {"from": "USD", "to": "HKD"}
params = {'reportName': 'RPT_HKF10_MAIN_DIVBASIC',
          'columns': 'SECURITY_CODE,UPDATE_DATE,REPORT_TYPE,EX_DIVIDEND_DATE,DIVIDEND_DATE,TRANSFER_END_DATE,YEAR,PLAN_EXPLAIN,IS_BFP',
          #'columns':'ALL',
          'quoteColumns':'',
          'filter': '(SECURITY_CODE="00008")(IS_BFP="0")',
          'pageNumber':'1',
          'pageSize':'1000',
          'sortTypes':'-1,-1',
          'sortColumns':'NOTICE_DATE,EX_DIVIDEND_DATE',
          'source':'F10',
          'client':'PC',
          }
resp = requests.get(url, params=params, timeout=120)
resp.raise_for_status()
json_data = resp.json()
print(json_data)
# 解析JSON
#data = json.loads(json_data)
records = json_data['result']['data']

# 定义CSV字段（按业务逻辑排序）
fieldnames = [
    'SECURITY_CODE',      # 证券代码
    'YEAR',               # 归属年份
    'REPORT_TYPE',        # 分配类型
    'EX_DIVIDEND_DATE',   # 除权除息日
    'DIVIDEND_DATE',      # 派息日
    'TRANSFER_END_DATE',  # 过户截止日
    'PLAN_EXPLAIN',       # 分配方案说明
    'UPDATE_DATE',        # 数据更新日期
    'IS_BFP'              # 是否Boardroom文件
]

# 保存为CSV文件
output_file = 'HK_dividend_00005.csv'
with open(output_file, 'w', newline='', encoding='utf-8-sig') as f:
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    
    for record in records:
        # 数据清洗：处理null值
        cleaned_record = {k: (v if v is not None else '') for k, v in record.items()}
        writer.writerow(cleaned_record)

print(f"✅ 已成功保存 {len(records)} 条记录到 {output_file}")
