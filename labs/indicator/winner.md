```text
大智慧、通达信软件，公式中有一个winner函数，函数的作用是计算收盘获利比率。即计算按照目前收盘价，计算有多少比例持仓是盈利的。要计算获利比率，首先得计算筹码分布，就是持仓价格分布。由于无法知道真实的交易情况，只能大致的估计。首先举个例子，假如某只股票有1000万股，第一天平均交易价格10元，第二天换手率20%，平均交易价格11元，那第二天的筹码分布为，10元：1000 * （1 - 0.2）=800万，11元：1000 * 0.2 = 200万。第三天换手率30%，平均交易价格12元，那第三天的筹码分布为，10元：1000 * （1 - 0.2）* （1 - 0.3）= 560万，11元：1000 * 0.2 * （1 - 0.3） = 140万，12元：1000 * 0.3 = 300万，以此类推。代码如下：

def winner_core(ContextInfo, close):
	close_price = close[-1]
	
	#获取上市日期
	#ipo_date = ContextInfo.get_open_date(ContextInfo.get_universe()[0])
	df = ContextInfo.get_market_data(['volume', 'amount'], stock_code = ContextInfo.get_universe(), skip_paused = True, period = '1d', dividend_type = 'front', end_time = close.index[-1], count = 250)
	df = df.loc[df['volume'] != 0]
	df['mean'] = df['amount'] / df['volume'] / 100
	turnover_rate = ContextInfo.get_turnover_rate(ContextInfo.get_universe(), df.index[0], df.index[-1])
	df['turnover'] = turnover_rate['000001.SZ'].values
	df['turnover'][0] = 0
	#1减去换手率
	df['1_turnover'] = 1 - df['turnover']
	df['2_turnover'] = df['1_turnover'][::-1].values
	df['3_turnover'] = df['2_turnover'].shift(periods = 1)
	df['3_turnover'][0] = 1
	#print(df[['1_turnover', '2_turnover', '3_turnover']])
	df['4_turnover'] = df['3_turnover'].cumprod()[::-1].values
	df['turnover'][0] = 1
	df['chouma'] = df['turnover'] * df['4_turnover']
	return df.loc[df['mean'] < close_price]['chouma'].sum()
 
def winner(ContextInfo, close):
	result = []
	n = len(close)
	for i in range(n):
		res = winner_core(ContextInfo, close[: i + 1])
		result.append(res)
	return pd.Series(result, index = close.index)

------------------------------------------------------------
LFS 筹码锁定因子	
a:=1.1*CLOSE;
b:=0.9*CLOSE;
活动筹码 :100*(WINNER(a)-WINNER(b));

```