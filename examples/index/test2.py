# #一个人至少有一个梦想，有一个理由去坚强
import numpy as np
import pandas as pd
import pylab as pl

df=pd.DataFrame({'x':[1,2,3,4,5,6,7,8,9]})
#df['y1']=df['x'].ewm(com=2,adjust=False).mean()
df['y2']=df['x'].ewm(span=5,adjust=False).mean()
#t1=df['x'].ewm(span=7,adjust=False)
#print(t1)
#df['y2']=t1.mean()
df['y3']=df['x'].ewm(alpha=1/3.0,adjust=False).mean()
df['y4']=df['x'].rolling(5).mean()
df['y5']=df['x'].ewm(span=5,adjust=True).mean()
print(df)