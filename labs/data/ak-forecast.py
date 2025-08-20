from quant1x.data import D

# def stock_profit_forecast(symbol: str = ""):
#     """
#     东方财富网-数据中心-研究报告-盈利预测
#     https://data.eastmoney.com/report/profitforecast.jshtml
#     :param symbol: "", 默认为获取全部数据; symbol="船舶制造", 则获取具体行业板块的数据; 行业板块可以通过 ak.stock_board_industry_name_em() 接口获取
#     :type symbol: str
#     :return: 盈利预测
#     :rtype: pandas.DataFrame
#     """
#     url = "https://datacenter-web.eastmoney.com/api/data/v1/get"
#     params = {
#         'reportName': 'RPT_VALUEANALYSIS_DET',
#         'columns': 'ALL',
#         'pageNumber': '1',
#         'pageSize': '5000',
#         'sortColumns': 'TRADE_DATE',
#         'sortTypes': '1',
#         'source': 'WEB',
#         'client': 'WEB',
#         'filter': '',
#         '_': '1676520096781',
#     }
#     if symbol:
#         params.update({'filter': f'(SECURITY_CODE="{symbol}")'})
#
#     r = requests.get(url, params=params)
#     data_json = r.json()
#     page_num = int(data_json['result']['pages'])
#     df = pd.DataFrame(data_json['result']['data'])
#     return df


code = "600600"
# df = stock_profit_forecast(symbol=code)
# print(df)
# df.to_csv(code + '-forecast.csv', index=False)
df = D.forecast(code)
print(df)
