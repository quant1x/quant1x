import akshare as ak

stock_individual_fund_flow_df = ak.stock_individual_fund_flow(stock="000678", market="sz")
print(stock_individual_fund_flow_df)
stock_individual_fund_flow_df.to_csv('fund-sz000678.csv', index=False)
