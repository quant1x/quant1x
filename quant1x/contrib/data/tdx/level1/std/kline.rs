// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BarFreq {
    Freq5Min = 0,
    Freq15Min = 1,
    Freq30Min = 2,
    Freq1Hour = 3,
    FreqDaily = 4,
    FreqWeekly = 5,
    FreqMonthly = 6,
    FreqExHQ1Min = 7,
    Freq1Min = 8,
    FreqRIK = 9,
    Freq3Month = 10,
    FreqYearly = 11,
}
