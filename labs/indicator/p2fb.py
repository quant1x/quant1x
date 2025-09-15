#!/usr/bin/python
# -*- coding: UTF-8 -*-
import inspect

import pandas

from quant1x import basic
from quant1x.formula.formula import *


def p2fb(code: str, data: pandas.DataFrame):
    """
    趋势反包
    :param code:
    :param data:
    :return:
    """
    #fun_name = sys._getframe().f_code.co_name
    indicator_name = inspect.stack()[0][3]
    df = data.copy()
    OPEN = df['open']
    CLOSE = df['close']
    HIGH = df['high']
    LOW = df['low']
    VOL = df['volume']
    AMOUNT = df['amount']

    # ZTBL:="BASIC.涨停比例";
    ZTBL = basic.ZTBL(code)
    # ZTJ:=ZTPRICE(REF(CLOSE,1),ZTBL);
    ZTJ = basic.ZTPRICE(REF(CLOSE, 1), ZTBL)
    # C0:=CLOSE;
    C0 = CLOSE
    # X:=CLOSE=ZTJ;
    X = CLOSE == ZTJ
    # N0:=BARSLAST(X=1);
    N0 = BARSLAST(X == 1)
    # P:=REF(OPEN,N0-1);
    P = REF(OPEN, N0 - 1)
    # XV:=REF(VOL,N0-1);
    XV = REF(VOL, N0 - 1)
    # 涨停已过:N0,NODRAW;
    # M0:=CROSS(MA(VOL,5), MA(VOL,10));
    M01 = MA(VOL, 5)
    M02 = MA(VOL, 10)
    M0 = CROSS(M01, M02)
    # VM:=BARSLAST(M0)>5;
    VM = BARSLAST(M0) > 5
    # CONDTION:=VM AND N0>=5 AND N0 <=30;
    CONDTION = VM & (N0 >= 5) & (N0 <= 30)
    # P1:=CLOSE/REF(OPEN,N0), NODRAW;
    P1 = CLOSE / REF(OPEN, N0)
    # TSL:=CONDTION AND P1<1.010;
    TSL = CONDTION & (P1 < 1.010)
    # 缩量成立: TSL*1, COLORCCFF00;
    缩量成立 = TSL * 1
    # 买入:2*(CONDTION AND SUM(VOL,N0)>=XV), COLORYELLOW;
    买入 = 2 * (CONDTION & (SUM(VOL, N0) >= XV))
    # B1:CONDTION AND MA(VOL,N0)<XV/3, NODRAW;
    B1 = CONDTION & (MA(VOL, N0) < (XV / 3))
    #
    #
    # ZN:=10;
    ZN = 10
    # {表示求10周期线性正回归线的斜率}
    # A:=SLOPE(CLOSE,ZN)/CLOSE*100;
    A = SLOPE(CLOSE, ZN) / CLOSE * 100
    # {表示求10周期线性回归预测本周期正收盘价}
    # B:=FORCAST(CLOSE,ZN)/CLOSE;
    B = FORCAST(CLOSE, ZN) / CLOSE
    # {表示求10周期线性负回归线的斜率}
    # D:=-(SLOPE(CLOSE,ZN)/CLOSE*100);
    D = -(SLOPE(CLOSE, ZN) / CLOSE * 100)
    # {表示10周期内价格回归存在斜率预测,股票之友预测定位模型}
    # F:=(A*CLOSE+B*CLOSE)/CLOSE,COLORRED;
    F = (A * CLOSE + B * CLOSE) / CLOSE
    # 买:=CROSS(A,D) OR CROSS(F,D);
    m1 = CROSS(A, D)
    m2 = CROSS(F, D)
    M = m1 | m2
    # 回归转向:4*FILTER(买=1,5),COLORE970DC;
    HGZX = 4 * FILTER(M, 5)
    df[indicator_name+'.hgzx'] = HGZX
    #df['fb.A'] = A
    #df['fb.B'] = B
    #df['fb.D'] = D
    #df['fb.F'] = F
    #
    # M1:=MA(CLOSE,5);
    M1 = MA(CLOSE, 5)
    # M2:=MA(CLOSE,10);
    M2 = MA(CLOSE, 10)
    # 均线粘合:ABS(M1/M2-1)*100,NODRAW;
    均线粘合 = ABS(M1 / M2 - 1) * 100

    return df
