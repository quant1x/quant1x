#!/usr/bin/python
# -*- coding: UTF-8 -*-
import pandas

from quant1x import basic
from quant1x.formula.formula import *


def hjk(code:str, data:pandas.DataFrame):
    """
    黄金坑
    """
    df = data.copy()
    OPEN = df['open']
    CLOSE = df['close']
    HIGH = df['high']
    LOW = df['low']
    VOL = df['volume']
    AMOUNT = df['amount']

    # ZTBL:="BASIC.涨停比例",NODRAW;
    ZTBL = basic.ZTBL(code)
    # R_CLOSE:=REF(CLOSE,1);
    R_CLOSE = REF(CLOSE, 1)
    # ZTJ:=ZTPRICE(R_CLOSE, ZTBL);
    ZTJ = basic.ZTPRICE(R_CLOSE, ZTBL)
    # BL:=VOL/REF(VOL,1),NODRAW;
    BL = VOL / REF(VOL, 1)
    # {倍量}
    # C11:=BL>=2.00,NODRAW;
    C11 = BL >= 2.00
    # {炸板}
    # C12:=CLOSE<=ZTJ,NODRAW;
    C12 = CLOSE <= ZTJ
    # {最高价大于涨停价的90%}
    # C13:=HIGH>=ZTJ*0.98,NODRAW;
    C13 = HIGH >= ZTJ * 0.98
    # PC:=C11 AND C12 AND C13,COLORYELLOW;
    #PC = pandas.eval('C11 & C12 & C13')
    PC = C11 & C12 & C13
    # PN:BARSLAST(PC),NODRAW;
    PN = BARSLAST(PC)
    # 前高:REF(CLOSE,PN),COLORWHITE;
    前高 = REF(CLOSE, PN)
    # 当前:CLOSE,COLORLIMAGENTA;
    当前 = CLOSE
    # 估值:FORCAST(CLOSE,PN),COLORYELLOW;
    估值 = FORCAST(CLOSE, PN)
    # 偏差:10*AVEDEV(CLOSE,PN),COLORLICYAN,NODRAW;
    偏差 = 10 * AVEDEV(CLOSE, PN)
    # PV:=REF(VOL,PN);
    PV = REF(VOL, PN)
    # PVH:=HHV(VOL,PN+1),NODRAW;
    PVH = HHV(VOL, PN + 1)
    # CJ:=COUNT(前高>=CLOSE,PN),NODRAW;
    CJ = COUNT(前高 >= CLOSE, PN)
    # SL:=VOL/PVH,NODRAW;
    SL = VOL / PVH
    # C21:SL<=0.50,NODRAW;
    C21 = SL <= 0.50
    # C22:CJ/PN>=0.50 OR CJ=PN,NODRAW;
    C22 = pandas.eval('CJ/PN>=0.50 | CJ==PN')
    # C23:PN>=4 AND PN<7,NODRAW;
    C23 = pandas.eval('PN>=4 & PN<7')
    # B:1*(C21 AND C22 AND C23),COLORRED,NODRAW;
    B = C21 & C22 & C23
    # DRAWICON(B>0,CLOSE*0.999,1);
    df['hjk.b'] = B
    # df['hjk.c21'] = C21
    # df['hjk.c22'] = C22
    # df['hjk.c23'] = C23
    return df
