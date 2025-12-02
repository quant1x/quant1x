#!/usr/bin/python
# -*- coding: UTF-8 -*-
import pandas

from quant1x.formula.formula import *

np.seterr(divide='ignore', invalid='ignore')


def cdtd(data: pandas.DataFrame):
    """
    抄底逃顶
    :param data:
    :return:
    """
    df = data.copy()
    OPEN = df['open']
    CLOSE = df['close']
    HIGH = df['high']
    LOW = df['low']
    VOL = df['volume']
    AMOUNT = df['amount']

    # N1:=3;
    N1 = 3
    # N2:=9;
    N2 = 9
    # N3:=27;
    N3 = 27
    # N4:=5;
    N4 = 5
    # HHVLLV3:=HHV(HIGH,N3)-LLV(LOW,N3);
    HHVLLV3 = HHV(HIGH, N3) - LLV(LOW, N3)
    # df['cdtd.h3'] = HHV(HIGH, N3)
    # df['cdtd.l3'] = LLV(LOW, N3)
    # df['cdtd.hl3'] = HHVLLV3
    # CLLV3:=CLOSE-LLV(LOW,N3);
    CLLV3 = CLOSE - LLV(LOW, N3)
    # RSV1:=(CLOSE-LLV(LOW,N2))/(HHV(HIGH,N2)-LLV(LOW,N2))*100;
    RSV1 = (CLOSE - LLV(LOW, N2)) / (HHV(HIGH, N2) - LLV(LOW, N2)) * 100
    # RSV2:=CLLV3/HHVLLV3*100;
    RSV2 = CLLV3 / HHVLLV3 * 100
    # WEN:=N1*SMA(RSV2,N4,1)-2*SMA(SMA(RSV2,N4,1),N1,1);
    WEN = N1 * SMA(RSV2, N4, 1) - 2 * SMA(SMA(RSV2, N4, 1), N1, 1)
    df['cdtd.wen'] = WEN
    # J1:=SMA(RSV1,N1,1);
    J1 = SMA(RSV1, N1, 1)
    # J2:=SMA(J1,N1,1);
    J2 = SMA(J1, N1, 1)
    # W1:=SMA(RSV2,N1,1);
    W1 = SMA(RSV2, N1, 1)
    # W2:=SMA(W1,N1,1);
    W2 = SMA(W1, N1, 1)
    # 强弱界线:49,DOTLINE,LINETHICK1,COLOR9966CC;
    # 顶:100, DOTLINE,COLORCCFF00;
    # 底:0, DOTLINE, COLORRED;
    # 趋势线:WEN,LINETHICK1,COLORFF84FF;
    # 卖出:=CROSS(J2,J1) AND J2>85;
    S1 = CROSS(J2, J1)
    S2 = J2 > 85
    S = S1 & S2
    df['cdtd.S1'] = S
    # 卖点预警线:90,DOTLINE,LINETHICK1,COLORBLUE;
    # DRAWICON(卖出, 70, 2);
    # 买入:=趋势线<REF(趋势线,1) AND 趋势线<=5;
    B1 = WEN < REF(WEN, 1)
    B2 = WEN <= 5
    B = B1 & B2
    df['cdtd.B'] = B
    # 买点预警线:10,DOTLINE,LINETHICK1,COLORWHITE;
    # DRAWICON(买入, 30, 1);
    # G1:=W1{,LINETHICK2,COLORWHITE};
    # G2:=W2{,LINETHICK2,COLORCYAN};
    # STICKLINE(趋势线>=85,100,趋势线,5,1),COLORGREEN;
    # STICKLINE(趋势线<=5,0,趋势线,5,1),COLORYELLOW;
    df['cdtd.S2'] = B2
    # STICKLINE(COUNT(趋势线<REF(趋势线,1) AND 趋势线<=5,2)=2,0,20,8,0),COLORRED;
    df['cdtd.S3'] = COUNT(B1 & B2, 2) == 2
    # STICKLINE(CROSS(J2,J1) AND J2>=85,100,80,8,0),COLORGREEN;
    return df
