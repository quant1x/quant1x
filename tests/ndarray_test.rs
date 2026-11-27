// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! NdArray 基本语义: 与 `quant1x/num/ndarray.py` 的同一批口径对齐。

use quant1x::num::NdArray;

fn approx(actual: f64, expected: f64) {
    assert!(
        (actual - expected).abs() < 1e-12,
        "expected {expected}, got {actual}"
    );
}

#[test]
fn construct_and_props() {
    let a = NdArray::new(vec![1.0, 2.0, 3.0]);
    assert_eq!(a.len(), 3);
    assert_eq!(a.dtype(), "float64");
    assert_eq!(a.to_f64(), vec![1.0, 2.0, 3.0]);
    assert_eq!(NdArray::zeros(2).to_f64(), vec![0.0, 0.0]);
    assert_eq!(NdArray::ones(2).to_f64(), vec![1.0, 1.0]);
    assert_eq!(NdArray::full(2, 5.0).to_f64(), vec![5.0, 5.0]);
    assert_eq!(NdArray::arange(0.0, 3.0, 1.0).to_f64(), vec![0.0, 1.0, 2.0]);
    assert_eq!(NdArray::linspace(0.0, 1.0, 3).to_f64(), vec![0.0, 0.5, 1.0]);
}

#[test]
fn arithmetic() {
    let a = NdArray::new(vec![1.0, 2.0, 3.0]);
    let b = NdArray::new(vec![10.0, 20.0, 30.0]);
    assert_eq!(a.add(&b).to_f64(), vec![11.0, 22.0, 33.0]);
    assert_eq!(a.sub(&b).to_f64(), vec![-9.0, -18.0, -27.0]);
    assert_eq!(a.mul(&b).to_f64(), vec![10.0, 40.0, 90.0]);
    assert_eq!(b.div(&a).to_f64(), vec![10.0, 10.0, 10.0]);
    assert_eq!(a.add_scalar(1.0).to_f64(), vec![2.0, 3.0, 4.0]);
    assert_eq!(a.abs().to_f64(), vec![1.0, 2.0, 3.0]);
    assert_eq!(NdArray::new(vec![4.0, 9.0]).sqrt().to_f64(), vec![2.0, 3.0]);
}

#[test]
fn compare_and_logic() {
    let a = NdArray::new(vec![1.0, 2.0, 3.0]);
    assert_eq!(a.gt_scalar(2.0).to_bool(), vec![false, false, true]);
    assert_eq!(a.equal_scalar(2.0).to_bool(), vec![false, true, false]);

    let x = NdArray::from_bools(vec![true, true, false, false]);
    let y = NdArray::from_bools(vec![true, false, true, false]);
    assert_eq!(x.logic_and(&y).to_bool(), vec![true, false, false, false]);
    assert_eq!(x.logic_or(&y).to_bool(), vec![true, true, true, false]);
    assert_eq!(x.logic_not().to_bool(), vec![false, false, true, true]);
}

#[test]
fn reductions() {
    let a = NdArray::new(vec![1.0, 2.0, 3.0, 4.0, 5.0]);
    approx(a.sum(), 15.0);
    approx(a.mean(), 3.0);
    approx(a.median(), 3.0);
    approx(a.min(), 1.0);
    approx(a.max(), 5.0);
    assert_eq!(a.argmin(), 0);
    assert_eq!(a.argmax(), 4);
    approx(a.var(), 2.0);
    approx(a.std(), 2f64.sqrt());
    assert_eq!(
        NdArray::new(vec![1.0, 2.0, 3.0]).cumsum().to_f64(),
        vec![1.0, 3.0, 6.0]
    );
}

#[test]
fn windows() {
    let a = NdArray::new(vec![1.0, 2.0, 3.0, 4.0]);
    let s = a.shift(1);
    assert!(s.get_f64(0).unwrap().is_nan());
    assert_eq!(s.to_f64()[1..], [1.0, 2.0, 3.0]);

    let d = a.diff(1);
    assert!(d.get_f64(0).unwrap().is_nan());
    assert_eq!(d.to_f64()[1..], [1.0, 1.0, 1.0]);

    let mean = a.rolling_mean(2);
    let values = mean.to_f64();
    assert!(values[0].is_nan());
    approx(values[1], 1.5);
    approx(values[2], 2.5);
    approx(values[3], 3.5);

    let ewm = NdArray::new(vec![1.0, 2.0, 3.0]).ewm_mean(0.3);
    let values = ewm.to_f64();
    approx(values[0], 1.0);
    approx(values[1], 1.3);
}

#[test]
fn astype_and_slice() {
    let a = NdArray::new(vec![1.0, 0.0, 2.0]);
    let mask = a.astype("bool").unwrap();
    assert_eq!(mask.dtype(), "bool");
    assert_eq!(mask.to_bool(), vec![true, false, true]);
    assert_eq!(
        mask.astype("float64").unwrap().to_f64(),
        vec![1.0, 0.0, 1.0]
    );
    assert_eq!(a.slice(0, 2).to_f64(), vec![1.0, 0.0]);
}
