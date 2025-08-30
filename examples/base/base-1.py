from quant1x.formula.formula import *

# df=pd.DataFrame({'x':[1,2,3,4,5,6,7,8,9]})
# 逻辑表达式测试
df = pd.DataFrame({'a': [True, True, True, False, False, False, False],
                   'b': [True, True, False, False, True, False, True],
                   'c': [True, False, False, False, False, True, True]
                   })

df['a & b & c'] = df['a'] & df['b'] & df['c']
print(df)


df = pd.DataFrame({'x': [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]})
print(df)
s1 = df['x'].rolling(5).mean().values
print(s1)

s2 = np.array([1, 2, 3, 4, 3, 3, 2, 1,np.nan,np.nan,np.nan,np.nan])
v2 = MA(df['x'], s2)
print(v2)


from quant1x.formula.formula import *
f1 = [1, 2, 3, 4]
r1 = pd.Series(f1)[::-1]
print(r1)
r2=r1.cumsum()
print(r2)
r3 = r2.sum()
print(r3)
