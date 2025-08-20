import math

import numpy as np
import pandas as pd
import datetime

if __name__ == '__main__':
    symbol = '002812'
    raw_data_file = './data/' + symbol + '.csv'
    training_data_file = './data/' + symbol + '_example.csv'

    df = pd.read_csv(raw_data_file)
    df.replace('None', np.NaN, inplace=True)

    dates = pd.to_datetime(df['Date'].values)
    df['days_from_start'] = (dates - datetime.datetime(1990, 12, 19)).days
    df.sort_values('days_from_start', inplace=True)
    dates = pd.to_datetime(df['Date'].values)

    df['day_of_week'] = dates.dayofweek
    df['day_of_month'] = dates.day
    df['week_of_year'] = pd.Index(dates.isocalendar().week)
    df['month'] = dates.month
    df['year'] = dates.year

    cols = ['Close', 'Open', 'High', 'Low', 'Volume', 'Turnover']
    #df[cols] = df[cols].rolling(8).mean()
    
    for col in cols:
        df[col] = df[col].apply(lambda x: np.NaN if x < 0.0000001 else x)
    df.fillna(method='ffill', inplace=True)

    #for col in cols:
    #    df[col] = df[col].pct_change()

    min_return = df[cols].min(axis=0)
    max_return = df[cols].max(axis=0)
    min_max_rslt = pd.concat([min_return, max_return], axis=1)
    #for col, val in min_max_rslt.iterrows():
    #    df[col] = df[col].apply(lambda x: (x - val[0])/(val[1] - val[0]))

    #min_volume = df['Volume'].min(axis=0)
    #max_volume = df['Volume'].max(axis=0)

    #df['Volume'] = (df['Volume'] - min_volume) / (max_volume - min_volume)
    df = df.dropna()

    df.to_csv(training_data_file, index=None)

    print(training_data_file)