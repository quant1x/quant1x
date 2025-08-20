import pandas as pd
import matplotlib.pyplot as plt

if __name__ == '__main__':
    data_csv_file = '../wb002812.csv'
    new_csv_file = '../explorer/data/002812.csv'
    df = pd.read_csv(data_csv_file)

    clos_map = {
        '交易日(Date)':'Date', '股票代码(Symbol)':'Symbol', '股票名称(Name)':'Name',
        '当日开盘价(Open)':'Open', '当日收盘价(Close)':'Close', '当日最高价(High)':'High',
        '当日最低价(Low)':'Low', '平均价(AvgPrice)':'AvgPrice', '昨日收盘价(PrevClose)':'PrevClose',
        '涨跌额(Change)':'Change', '涨跌幅(PctChg)':'PctChg', '振幅(Amplitude)':'Amplitude',
        '换手率(TurnoverRatio)':'TurnoverRatio', '成交量(Volume)':'Volume', '成交额(Turnover)':'Turnover',
        '总市值(MarketCAP)':'MarketCAP', '总股本(SharedOutstanding)':'SharedOutstanding', '流通值(FloatCAP)':'FloatCAP',
        '流通股(ShsFloat)':'ShsFloat', '市盈率TTM(PETTM)':'PETTM', '市盈率静(PEStatic)': 'PEStatic',
        '市净率(PB)':'PB', '委比(BidAskPct)':'BidAskPct', '量比(VolumePct)':'VolumePct',
        '净流入量(NetInflowVolume)':'NetInflowVolume', '净流入额(NetInflowAmount)':'NetInflowAmount'
    }
    
    new_df = pd.DataFrame()

    for key, val in clos_map.items():
        new_df[val] = df[key]

    exclusion_cols = [
        'Date', 'Symbol', 'Name'
    ]

    col_list = new_df.columns.to_list()
    draw_cols = list(set(col_list).difference(set(exclusion_cols)))
    for col in draw_cols:
        new_df[col] = new_df[col].interpolate(method=’linear’)

    #fig = plt.figure(figsize=(30, 10 * len(draw_cols)))
    fig = plt.figure(figsize=(30, 5 * len(draw_cols)))
    for idx, value in enumerate(draw_cols):
        ax = fig.add_subplot(len(draw_cols), 1, idx + 1)
        ax.plot(new_df[value], color='r', label=value)
        ax.set_title(value)
    plt.show()

    new_df.to_csv(new_csv_file, index=None)