// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! Series 基本语义: 与 `quant1x/pandas/series.py` 的同一批口径对齐。

use quant1x::Series;

fn approx(actual: f64, expected: f64) {
    assert!(
        (actual - expected).abs() < 1e-12,
        "expected {expected}, got {actual}"
    );
}

#[test]
fn shift_diff_cumsum() {
    let s = Series::new(vec![1.0, 2.0, 3.0, 4.0, 5.0]);
    let shifted = s.shift(1);
    let values = shifted.float_values().unwrap();
    assert!(values[0].is_nan());
    assert_eq!(&values[1..], &[1.0, 2.0, 3.0, 4.0]);

    let diff = s.diff(1);
    let values = diff.float_values().unwrap();
    assert!(values[0].is_nan());
    assert_eq!(&values[1..], &[1.0, 1.0, 1.0, 1.0]);

    assert_eq!(
        s.cumsum().float_values().unwrap(),
        &[1.0, 3.0, 6.0, 10.0, 15.0]
    );
}

#[test]
fn arithmetic_and_comparison() {
    let s = Series::new(vec![1.0, 2.0, 3.0, 4.0, 5.0]);
    let doubled = s.add(&s);
    assert_eq!(doubled.float_values().unwrap(), &[2.0, 4.0, 6.0, 8.0, 10.0]);

    let plus_one = s.add_scalar(1.0);
    assert_eq!(plus_one.float_values().unwrap(), &[2.0, 3.0, 4.0, 5.0, 6.0]);

    let mask = s.gt_scalar(3.0);
    assert_eq!(mask.dtype(), "bool");
    assert_eq!(
        mask.bool_values().unwrap(),
        &[false, false, false, true, true]
    );
}

#[test]
fn logic() {
    let a = Series::from_bools(vec![true, false, true, false]);
    let b = Series::from_bools(vec![true, true, false, false]);
    assert_eq!(
        a.and(&b).bool_values().unwrap(),
        &[true, false, false, false]
    );
    assert_eq!(a.or(&b).bool_values().unwrap(), &[true, true, true, false]);
    assert_eq!(a.not().bool_values().unwrap(), &[false, true, false, true]);
}

#[test]
fn reductions() {
    let s = Series::new(vec![1.0, 2.0, 3.0, 4.0, 5.0]);
    approx(s.sum(), 15.0);
    approx(s.mean(), 3.0);
    approx(s.median(), 3.0);
    approx(s.min(), 1.0);
    approx(s.max(), 5.0);
    assert_eq!(s.argmin(), 0);
    assert_eq!(s.argmax(), 4);
    approx(s.var(), 2.0);
    approx(s.std(), 2f64.sqrt());
}

#[test]
fn rolling_and_ewm() {
    let s = Series::new(vec![1.0, 2.0, 3.0, 4.0, 5.0]);
    let mean = s.rolling(3).mean();
    let values = mean.float_values().unwrap();
    assert!(values[0].is_nan());
    assert!(values[1].is_nan());
    approx(values[2], 2.0);
    approx(values[3], 3.0);
    approx(values[4], 4.0);

    let sum = s.rolling(3).sum();
    assert_eq!(&sum.float_values().unwrap()[2..], &[6.0, 9.0, 12.0]);

    let ewm = s.ewm(0.5).mean();
    let values = ewm.float_values().unwrap();
    approx(values[0], 1.0);
    approx(values[1], 1.5);
    approx(values[2], 2.25);
    approx(values[3], 3.125);
    approx(values[4], 4.0625);
}

#[test]
fn fill_na_and_name() {
    let s = Series::new(vec![1.0, f64::NAN, 3.0]).with_name("close");
    assert_eq!(s.name(), Some("close"));
    let filled = s.fill_na(0.0);
    assert_eq!(filled.float_values().unwrap(), &[1.0, 0.0, 3.0]);
    assert_eq!(s.len(), 3);
    let sliced = s.slice(0, 2);
    let values = sliced.float_values().unwrap();
    approx(values[0], 1.0);
    assert!(values[1].is_nan());
}
