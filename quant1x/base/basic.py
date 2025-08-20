#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""
最基础的工具
"""


# A_YI:=100000000;
# 亿:A_YI;
# 股价:CLOSE<30;
# 市值:FINANCE(40)<(2000*A_YI);
# 涨停比例:IF(FINANCE(3)=2,0.3,IF((FINANCE(3)=4 OR FINANCE(3)=3),0.2,0.1));
def ZTBL(code: str) -> float:
    """
    涨停比例, 只支持A股

    .. note::
      30 68开头的个股涨停比例20%, 其余10%

    :param code:
    :return:
    """
    symbol = code[-6:]
    prefix = symbol[0:2]
    if prefix in ['30', '68']:
        return 0.2
    else:
        return 0.1


def ZTPRICE(close, ztbl) -> float:
    """
    计算涨停价
    :param close:
    :param ztbl:
    :return:
    """
    return close * (1 + ztbl)

# 停牌:DYNAINFO(8)<=0;
# ST股:NAMELIKE('ST') OR NAMELIKE('*ST') OR NAMELIKE('S');
# 真阳线:CLOSE>REF(CLOSE,1);
# 筹码锁定:BARSLASTCOUNT(LFS>REF(LFS,1))>=2 AND LFS>=30;
# 浮筹比例:(WINNER(C*1.1)-WINNER(C*0.9))/WINNER(HHV(H,0))*100;
#
# JXN1:=5;
# JXN2:=10;
# M1:=MA(CLOSE,JXN1);
# M2:=MA(CLOSE,JXN2);
# 均线粘合:ABS(M1/M2-1)<0.01;
#
# {K线数据}
# X_HIGH:HIGH;
# X_LOW:LOW;
# X_OPEN:OPEN;
# X_CLOSE:CLOSE;
# X_VOL:VOL;
# X_AMOUNT:AMOUNT;
#
# {斜率}
# C_WIDTH:0.009499432279;
# C_PI:3.141592654;
