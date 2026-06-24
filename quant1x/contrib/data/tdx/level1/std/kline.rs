// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// kline — K线类型枚举

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum KLineType {
    _5Min = 0,
    _15Min = 1,
    _30Min = 2,
    _1Hour = 3,
    Daily = 4,
    Weekly = 5,
    Monthly = 6,
    Exhq1Min = 7,
    _1Min = 8,
    RiK = 9,
    _3Month = 10,
    Yearly = 11,
}
