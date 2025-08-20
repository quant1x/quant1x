import akshare as ak

stock_zh_a_stop_em_df = ak.stock_zh_a_stop_em()
print(stock_zh_a_stop_em_df)
stock_zh_a_stop_em_df.to_csv("em-stop.csv", index=False)

import akshare as ak

stock_info_sh_delist_df = ak.stock_info_sh_delist()
print(stock_info_sh_delist_df)
stock_info_sh_delist_df.to_csv('delist.csv', index=False)
