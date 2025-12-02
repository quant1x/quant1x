#!/usr/bin/python
# -*- coding: UTF-8 -*-
import pandas

from quant1x.formula.formula import *


def f89k(data:pandas.DataFrame):
    """
    89K图形指标
    :param code:
    :return:
    """
    df = data.copy()
    OPEN = df['open']
    CLOSE = df['close']
    HIGH = df['high']
    LOW = df['low']
    VOL = df['volume']
    AMOUNT = df['amount']

    # {89K图形}
    #
    N1 = 89
    # MA89:=MA(CLOSE,N1);
    MA89 = MA(CLOSE,N1)
    # {计算N1日内最低价}
    # L89:=LLV(LOW,N1);
    L89 = LLV(LOW,N1)
    # {计算N1日内最高价}
    # H89:=HHV(HIGH,N1);
    H89 = HHV(HIGH, N1)
    #
    # {确定⑤}
    # K5:L89,NODRAW,COLORGREEN;
    K5 = L89
    # {确定⑦}
    # N7:BARSLAST(H89=HIGH),NODRAW,COLORLIGRAY;
    N7 = BARSLAST(H89==HIGH)
    # T7:=REF(HIGH,N7);
    T7 = REF(HIGH, N7)
    # K7:T7,NODRAW,COLORRED;
    K7 = T7
    # {确定⑧}
    # T8:=LLV(LOW,N7),NODRAW,COLORGREEN;
    T8 = LLV(LOW,N7)
    # N8:BARSLAST(T8=LOW AND N1>N7),NODRAW,COLORLIGRAY;
    N11 = np.repeat(N1, len(CLOSE))
    N8C1 = T8==LOW
    N8C2 = N11>N7
    N8 = BARSLAST(N8C1 & N8C2)
    # K8:T8,NODRAW,COLORGREEN;
    K8 = T8
    # {确定⑨}
    # T9:=HHV(HIGH,N8);
    T9 = HHV(HIGH, N8)
    # N9X:=BARSLAST(T9=HIGH AND N7>N8);
    N9XC1 = T9 == HIGH
    N9XC2 = N7>N8
    N9X = BARSLAST(N9XC1 & N9XC2)
    # N9:IFF(N9X=0,N9X+1,N9X),NODRAW,COLORLIGRAY;
    N9 = IFF(N9X==0, N9X+1, N9X)
    # K9:REF(HIGH,N9X),NODRAW,COLORRED;
    K9 = REF(HIGH,N9X)
    # {确定⑩}
    # K10:LLV(LOW,N9),NODRAW,COLORGREEN;
    K10 = LLV(LOW,N9)
    # N10:BARSLAST(K10=LOW AND N8>N9),NODRAW,COLORLIGRAY;
    N10C1 = K10==LOW
    N10C2 = N8>N9
    N10 = BARSLAST(N10C1 & N10C2)
    #
    # {比对周期长度}
    # C_N:=5;
    C_N = 5
    # {量价关系最低校对比率}
    # C_S:=0.191;
    C_S = 0.191
    # {涂画与股价的纵向比率}
    # C_PX:=0.002;
    C_PX = 0.002
    # {真阳线}
    # {C_ISMALE:=(CLOSE > REF(CLOSE,1)) AND (CLOSE > OPEN);}
    # C_ISMALE:=CLOSE > REF(CLOSE,1);
    C_ISMALE = CLOSE > REF(CLOSE, 1)
    # {成交量较上一个周期放大}
    # C_VOL:= VOL>REF(VOL,1);
    C_VOL = VOL > REF(VOL, 1)
    # {成交量均线周期}
    # VOL_PERIOD:=5;
    VOL_PERIOD = 5
    # {成交量比例}
    # VOLSCALE:=1+C_S;
    VOLSCALE = 1 + C_S
    # {高股价或指数的计算方法, 比MAVOL5高出C_S/10且比前一日方法}
    # X_INDEX:=VOL>=MA(VOL, VOL_PERIOD)*(1 + C_S/10);
    X_INDEX = VOL >= MA(VOL, VOL_PERIOD) * (1 + C_S / 10)
    # {一般股价的计算方法}
    # X_GENERAL:=VOL>=MA(VOL, VOL_PERIOD)*VOLSCALE;
    X_GENERAL = VOL >= MA(VOL, VOL_PERIOD) * VOLSCALE
    # {指数类或者高股价类的成交量不太可能像个股那样成倍放量, 这里做一个降级处理}
    # X:=IFF(CLOSE>=100, X_INDEX, X_GENERAL) AND C_ISMALE AND C_VOL;
    #X = IFF(CLOSE >= 100, X_INDEX, X_GENERAL) & C_ISMALE & C_VOL
    X0 = IFF(CLOSE>=100, X_INDEX, X_GENERAL)
    X1 = X0 & C_ISMALE
    X = X1 & C_VOL
    # {确定X上一次确立成立是在哪一天}
    # DN:=BARSLAST(X);
    DN = BARSLAST(X)
    # {放量上攻作为一个小阶段的起点, 该起点K线的最低价作为止盈止损线}
    # ZS_VOL:=REF(VOL,DN);
    # ZS_LOW:=REF(LOW,DN);
    ZS_LOW = REF(LOW, DN)
    # ZS:ZS_LOW,NODRAW,COLORLIGRAY;
    # N11:BARSLAST(CLOSE<ZS_LOW AND N9>N10),NODRAW,COLORLIGRAY;
    # {股价突破止损线}
    # {C0:BARSSINCEN(CLOSE<ZS_LOW,DN),NODRAW;}
    # {C0:FINDHIGHBARS(CLOSE,1,DN-1,1),NODRAW;}
    # C00:=BARSLASTCOUNT(CLOSE<ZS_LOW),NODRAW;
    # C01:=BARSLASTCOUNT(CLOSE>=ZS_LOW),NODRAW;
    # C02:=IFF(C01>0,REF(C00,1),C01),NODRAW;
    # C03:=REF(CLOSE,C02-1),NODRAW;
    # C1:BARSLAST(CLOSE>ZS_LOW AND N9>N10),NODRAW,COLORLIGRAY;
    C1C = pandas.eval('CLOSE>ZS_LOW & N9>N10')
    C1 = BARSLAST(C1C)
    # {股价跌破止损线}
    # C2:BARSLAST(CLOSE<ZS_LOW AND N9>N10),NODRAW,COLORLIGRAY;
    C2C = pandas.eval('CLOSE<ZS_LOW & N9>N10')
    C2 = BARSLAST(C2C)
    # C3:N10>0,NODRAW;
    C3 = N10 > 0
    # B0:=C1=0 AND C2=1 AND C3;
    B0 = C1 == 0
    B1 = C2 == 1
    B2 = C3
    # B:50*B0,COLORYELLOW;
    B = pandas.eval('B0 & B1 & B2')
    date = df['date']
    df = pd.DataFrame()
    df['date'] = date
    df['C1C'] = C1C
    df['N9'] = N9
    df['N10'] = N10
    df['C1'] = C1
    df['C2'] = C2
    df['C3'] = C3
    df['CLOSE'] = CLOSE
    df['f89k.zs'] = ZS_LOW
    df['f89k.b'] = B
    df['f89k.DN'] = DN
    #
    # {筹码锁定因子}
    # CM0:=50-LFS;
    # CM:CM0,COLORSTICK;
    # CM1:EMA(CM0,3);
    #
    # {斜率}
    # XL:(K7-K9)/(N7-N9),NODRAW;
    # P0:CLOSE,COLORWHITE;
    # P1:-1*XL*N9+K9,COLORCYAN;
    return df

