import pandas

# 列名与数据对其显示
pandas.set_option('display.unicode.ambiguous_as_wide', True)
pandas.set_option('display.unicode.east_asian_width', True)
# 显示所有列
pandas.set_option('display.max_columns', None)

filename = '600600.csv'
df = pandas.read_csv(filename)
print(df)
print(df['vol'].sum())
# df1 = df.groupby('buyorsell')
# print(df1.sum())
bos = df.groupby('buyorsell')
print(bos.get_group(0)['vol'].sum())
print(bos.get_group(1)['vol'].sum())
print(bos.get_group(2)['vol'].sum())