import math
import numpy as np
import pandas as pd
import datetime

if __name__ == '__main__':
    raw_data_file = './data/600243.csv'
    training_data_file = './data/600243_example.csv'

    raw_data = pd.read_csv(raw_data_file)
    raw_data.replace('None', np.NaN, inplace=True)

    dates = pd.to_datetime(raw_data['Date'].values)
    raw_data['days_from_start'] = (dates - datetime.datetime(1990, 12, 19)).days
    raw_data.sort_values('days_from_start', inplace=True)
    #dont remove
    dates = pd.to_datetime(raw_data['Date'].values)

    raw_data['id'] = raw_data['days_from_start']
    raw_data['day_of_week'] = dates.dayofweek
    raw_data['day_of_month'] = dates.day
    raw_data['week_of_year'] = pd.Index(dates.isocalendar().week)
    raw_data['month'] = dates.month
    raw_data['year'] = dates.year

    input_seq_len = 128
    output_size = 5

    for i in range(output_size):
        idx = 'day' + str(i + 1)
        #raw_data[idx] = raw_data['Open'].shift(-(i+1))
    
    cols = ['Close', 'Open', 'High', 'Low', 'Volume', 'Turnover']
    for col in cols:
        raw_data[col] = raw_data[col].apply(lambda x: np.NaN if x < 0.0000001 else x)
    raw_data.fillna(method='ffill', inplace=True)
    raw_data[cols] = raw_data[cols].rolling(2).mean()
    raw_data.dropna(inplace=True)
    
    raw_data.to_csv(training_data_file)

    print(training_data_file)


