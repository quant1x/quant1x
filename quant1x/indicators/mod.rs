//! indicators —— **通达信指标, 语义以 quant1x/indicators/*.py 为准**
//!
//! 与 C++ `quant1x/indicators/*.h` 逐函数对应; 输入 `&[Bar]`, 输出结构化列。
//! 所有算子都走 `crate::formula`, 口径与 Python / C++ 一致。

use crate::data::schema::Bar;
use crate::formula as f;
use crate::formula::{Bools, Series};

/// 取某一列
fn column(bars: &[Bar], pick: fn(&Bar) -> f64) -> Series {
    bars.iter().map(pick).collect()
}

/// 抄底逃顶输出列
pub struct CdtdResult {
    /// 趋势线
    pub wen: Series,
    /// 卖出信号
    pub s1: Bools,
    /// 买入信号
    pub b: Bools,
    /// 趋势线 <= 5
    pub s2: Bools,
    /// 连续两周期满足买入条件
    pub s3: Bools,
}

/// 抄底逃顶
pub fn cdtd(bars: &[Bar]) -> CdtdResult {
    let close = column(bars, |bar| bar.close);
    let high = column(bars, |bar| bar.high);
    let low = column(bars, |bar| bar.low);

    let n1: i64 = 3;
    let n2: i64 = 9;
    let n3: i64 = 27;
    let n4: i64 = 5;

    let high_low_span = f::sub(&f::hhv(&high, n3), &f::llv(&low, n3));
    let close_low_span = f::sub(&close, &f::llv(&low, n3));
    let rsv1 = f::mul_scalar(
        &f::div(
            &f::sub(&close, &f::llv(&low, n2)),
            &f::sub(&f::hhv(&high, n2), &f::llv(&low, n2)),
        ),
        100.0,
    );
    let rsv2 = f::mul_scalar(&f::div(&close_low_span, &high_low_span), 100.0);

    let smooth = f::sma(&rsv2, n4, 1);
    let wen = f::sub(
        &f::mul_scalar(&smooth, n1 as f64),
        &f::mul_scalar(&f::sma(&smooth, n1, 1), 2.0),
    );

    let j1 = f::sma(&rsv1, n1, 1);
    let j2 = f::sma(&j1, n1, 1);

    let s1 = f::logic_and(&f::cross(&j2, &j1), &f::greater_scalar(&j2, 85.0));
    let s2 = f::less_equal_scalar(&wen, 5.0);
    let buy = f::logic_and(&f::less(&wen, &f::ref_value(&wen, 1)), &s2);
    let s3 = f::equal_scalar(&f::count(&buy, 2), 2.0);

    CdtdResult {
        wen,
        s1,
        b: buy,
        s2,
        s3,
    }
}

/// 八九空间输出列
pub struct F89kResult {
    /// 止损线
    pub zs: Series,
    /// 买点
    pub b: Bools,
    /// 放量上攻的起点距今周期数
    pub dn: Series,
}

/// 八九空间
pub fn f89k(bars: &[Bar]) -> F89kResult {
    let close = column(bars, |bar| bar.close);
    let high = column(bars, |bar| bar.high);
    let low = column(bars, |bar| bar.low);
    let volume = column(bars, |bar| bar.volume);

    let n1: i64 = 89;
    let c_s = 0.191;

    let high_89 = f::hhv(&high, n1);
    let n7 = f::barslast(&f::equal(&high_89, &high));
    let t8 = f::llv(&low, f::series_to_window(&n7));
    let n8 = f::barslast(&f::logic_and(
        &f::equal(&t8, &low),
        &f::less_scalar(&n7, n1 as f64),
    ));
    let t9 = f::hhv(&high, f::series_to_window(&n8));
    let n9x = f::barslast(&f::logic_and(&f::equal(&t9, &high), &f::greater(&n7, &n8)));
    let n9 = f::if_select(&f::equal_scalar(&n9x, 0.0), &f::add_scalar(&n9x, 1.0), &n9x);
    let k10 = f::llv(&low, f::series_to_window(&n9));
    let n10 = f::barslast(&f::logic_and(&f::equal(&k10, &low), &f::greater(&n8, &n9)));

    let male = f::greater(&close, &f::ref_value(&close, 1));
    let volume_up = f::greater(&volume, &f::ref_value(&volume, 1));
    let volume_average = f::ma(&volume, 5);
    let index_volume = f::greater_equal(&volume, &f::mul_scalar(&volume_average, 1.0 + c_s / 10.0));
    let general_volume = f::greater_equal(&volume, &f::mul_scalar(&volume_average, 1.0 + c_s));
    let branch = f::if_select_bools(
        &f::greater_equal_scalar(&close, 100.0),
        &index_volume,
        &general_volume,
    );
    let breakout = f::logic_and(&f::logic_and(&branch, &male), &volume_up);

    let dn = f::barslast(&breakout);
    let stop_loss = f::ref_window(&low, &f::series_to_window(&dn));

    let above = f::logic_and(&f::greater(&close, &stop_loss), &f::greater(&n9, &n10));
    let below = f::logic_and(&f::less(&close, &stop_loss), &f::greater(&n9, &n10));
    let c1_zero = f::equal_scalar(&f::barslast(&above), 0.0);
    let c2_one = f::equal_scalar(&f::barslast(&below), 1.0);
    let n10_positive = f::greater_scalar(&n10, 0.0);
    let buy = f::logic_and(&f::logic_and(&c1_zero, &c2_one), &n10_positive);

    F89kResult {
        zs: stop_loss,
        b: buy,
        dn,
    }
}

/// 黄金坑输出列
pub struct HjkResult {
    /// 买点
    pub b: Bools,
}

/// 黄金坑
pub fn hjk(bars: &[Bar], limit_up_ratio: f64) -> HjkResult {
    let close = column(bars, |bar| bar.close);
    let high = column(bars, |bar| bar.high);
    let volume = column(bars, |bar| bar.volume);

    let previous_close = f::ref_value(&close, 1);
    let limit_price = f::ztprice(&previous_close, limit_up_ratio);
    let volume_ratio = f::div(&volume, &f::ref_value(&volume, 1));

    let doubled = f::greater_equal_scalar(&volume_ratio, 2.0);
    let broken = f::less_equal(&close, &limit_price);
    let near_limit = f::greater_equal(&high, &f::mul_scalar(&limit_price, 0.98));
    let pattern = f::logic_and(&f::logic_and(&doubled, &broken), &near_limit);

    let pn = f::barslast(&pattern);
    let pn_window = f::series_to_window(&pn);
    let previous_high = f::ref_window(&close, &pn_window);
    let count_above = f::count(&f::greater_equal(&previous_high, &close), pn_window.clone());
    let volume_high = f::hhv(&volume, f::series_to_window(&f::add_scalar(&pn, 1.0)));
    let shrink = f::div(&volume, &volume_high);

    let c21 = f::less_equal_scalar(&shrink, 0.50);
    let c22 = f::logic_or(
        &f::greater_equal_scalar(&f::div(&count_above, &pn), 0.50),
        &f::equal(&count_above, &pn),
    );
    let c23 = f::logic_and(
        &f::greater_equal_scalar(&pn, 4.0),
        &f::less_scalar(&pn, 7.0),
    );
    let buy = f::logic_and(&f::logic_and(&c21, &c22), &c23);

    HjkResult { b: buy }
}

/// 一线天输出列
pub struct Ma1xResult {
    /// 放量上攻起点距今周期数
    pub dn: Series,
    /// 止盈止损线
    pub zs: Series,
    /// 压力线
    pub yl: Series,
    /// 回踩
    pub hc: Bools,
    /// 突破
    pub tp: Bools,
}

/// 一线天
pub fn ma1x(bars: &[Bar]) -> Ma1xResult {
    let close = column(bars, |bar| bar.close);
    let high = column(bars, |bar| bar.high);
    let low = column(bars, |bar| bar.low);
    let volume = column(bars, |bar| bar.volume);

    let c_s = 0.191;

    let male = f::greater(&close, &f::ref_value(&close, 1));
    let volume_up = f::greater(&volume, &f::ref_value(&volume, 1));
    let volume_average = f::ma(&volume, 5);
    let index_volume = f::greater_equal(&volume, &f::mul_scalar(&volume_average, 1.0 + c_s / 10.0));
    let general_volume = f::greater_equal(&volume, &f::mul_scalar(&volume_average, 1.0 + c_s));
    let branch = f::if_select_bools(
        &f::greater_equal_scalar(&close, 100.0),
        &index_volume,
        &general_volume,
    );
    let breakout = f::logic_and(&f::logic_and(&branch, &male), &volume_up);

    let dn = f::barslast(&breakout);
    let stop_loss = f::ref_window(&low, &f::series_to_window(&dn));

    let h0 = f::hhv(&high, 5);
    let resistance_pattern = f::logic_and(&f::logic_and(&male, &f::equal(&high, &h0)), &volume_up);
    let yl_n = f::barslast(&resistance_pattern);
    let resistance_high = f::ref_window(&high, &f::series_to_window(&yl_n));

    let dn_zero = f::equal_scalar(&dn, 0.0);
    let support_period = f::if_select(
        &dn_zero,
        &f::add_scalar(&f::ref_value(&stop_loss, 1), 1.0),
        &dn,
    );
    let support_line = f::if_select(&dn_zero, &f::ref_value(&stop_loss, 1), &stop_loss);

    let previous_high = f::ref_value(&high, 1);
    let wash = f::mul_scalar(
        &f::div(&f::sub(&previous_high, &low), &previous_high),
        100.0,
    );
    let pullback = f::logic_and(
        &f::logic_and(
            &f::greater_equal_scalar(&support_period, 2.0),
            &f::greater_equal_scalar(&wash, 2.33),
        ),
        &f::logic_and(
            &f::less(&low, &support_line),
            &f::greater_equal(&close, &support_line),
        ),
    );

    let yl_zero = f::equal_scalar(&yl_n, 0.0);
    let resistance_period = f::if_select(
        &yl_zero,
        &f::add_scalar(&f::ref_value(&yl_n, 1), 1.0),
        &yl_n,
    );
    let resistance_line = f::if_select(
        &yl_zero,
        &f::ref_value(&resistance_high, 1),
        &resistance_high,
    );

    let touched = f::logic_or(
        &f::logic_or(
            &f::less(&f::ref_value(&close, 1), &resistance_line),
            &f::less(&f::ref_value(&low, 1), &resistance_line),
        ),
        &f::less(&low, &resistance_line),
    );
    let recovered = f::greater_equal(&close, &resistance_line);
    let penetrated = f::logic_and(
        &f::logic_and(&touched, &recovered),
        &f::greater_equal_scalar(&resistance_period, 2.0),
    );

    Ma1xResult {
        dn,
        zs: stop_loss,
        yl: resistance_high,
        hc: pullback,
        tp: penetrated,
    }
}

/// 趋势反包输出列
pub struct P2fbResult {
    /// 回归转向
    pub hgzx: Series,
}

/// 趋势反包
pub fn p2fb(bars: &[Bar], limit_up_ratio: f64) -> P2fbResult {
    let close = column(bars, |bar| bar.close);
    let _ = limit_up_ratio;

    let slope_period: i64 = 10;

    let a = f::mul_scalar(&f::div(&f::slope(&close, slope_period), &close), 100.0);
    let b = f::div(&f::forcast(&close, slope_period), &close);
    let d = f::mul_scalar(&a, -1.0);
    let term = f::div(&f::add(&f::mul(&a, &close), &f::mul(&b, &close)), &close);
    let turn = f::logic_or(&f::cross(&a, &d), &f::cross(&term, &d));
    let hgzx = f::mul_scalar(&f::filter(&turn, 5), 4.0);

    P2fbResult { hgzx }
}
