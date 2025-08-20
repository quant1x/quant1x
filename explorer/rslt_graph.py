import pandas as pd
import matplotlib.pyplot as plt

if __name__ == '__main__':
    df = pd.read_csv('./dst.csv', index_col=0)

    #df.T.tail(8).plot()
    df.T.plot()
    plt.show()
