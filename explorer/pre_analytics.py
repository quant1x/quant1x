import pandas as pd
import matplotlib.pyplot as plt

if __name__ == '__main__':
    data_csv_file = './data/603126_example.csv'
    df = pd.read_csv(data_csv_file)
    col_list = df.columns.to_list()

    exclusion_cols = [
        'Date', 'Symbol', 'Name', 'days_from_start', 'day_of_week',
        'week_of_year', 'month', 'year',
    ]

    draw_cols = list(set(col_list).difference(set(exclusion_cols)))
    #fig = plt.figure(figsize=(30, 10 * len(draw_cols)))
    fig = plt.figure(figsize=(30, 5 * len(draw_cols)))
    for idx, value in enumerate(draw_cols):
        ax = fig.add_subplot(len(draw_cols), 1, idx + 1)
        ax.plot(df[value], color='r', label=value)
        ax.set_title(value)
    plt.show()
