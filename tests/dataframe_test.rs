// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! DataFrame 契约用例：列序、取列、子集、写入与列长校验。

use quant1x::{DataFrame, Series};

fn sample() -> DataFrame {
    DataFrame::from_columns(vec![
        ("open".to_string(), Series::new(vec![1.0, 2.0, 3.0])),
        ("close".to_string(), Series::new(vec![1.5, 2.5, 3.5])),
    ])
    .expect("构造失败")
}

#[test]
fn test_columns_and_shape() {
    let frame = sample();
    assert_eq!(frame.columns(), vec!["open", "close"]);
    assert_eq!(frame.shape(), (3, 2));
    assert_eq!(frame.len(), 3);
    assert_eq!(frame.column_count(), 2);
    assert!(!frame.is_empty());
    assert!(DataFrame::new().is_empty());
    assert_eq!(DataFrame::new().shape(), (0, 0));
}

#[test]
fn test_get_and_select() {
    let frame = sample();
    let close = frame.get("close").expect("close 列缺失");
    assert_eq!(close.to_f64(), vec![1.5, 2.5, 3.5]);
    assert!(frame.get("volume").is_none());
    assert!(frame.contains("open"));

    let subset = frame.select(&["close"]).expect("子集失败");
    assert_eq!(subset.columns(), vec!["close"]);
    assert_eq!(subset.shape(), (3, 1));
    assert!(frame.select(&["volume"]).is_err());
}

#[test]
fn test_set_replaces_in_place() {
    let mut frame = sample();
    frame
        .set("open", Series::new(vec![10.0, 20.0, 30.0]))
        .expect("替换失败");
    assert_eq!(frame.columns(), vec!["open", "close"]);
    assert_eq!(
        frame.get("open").expect("open 列缺失").to_f64(),
        vec![10.0, 20.0, 30.0]
    );
}

#[test]
fn test_set_appends() {
    let mut frame = sample();
    frame
        .set("volume", Series::new(vec![100.0, 200.0, 300.0]))
        .expect("追加失败");
    assert_eq!(frame.columns(), vec!["open", "close", "volume"]);
    assert_eq!(frame.shape(), (3, 3));
}

#[test]
fn test_set_length_mismatch() {
    let mut frame = sample();
    assert!(frame.set("volume", Series::new(vec![1.0])).is_err());
    assert_eq!(frame.columns(), vec!["open", "close"]);
}

#[test]
fn test_copy_is_deep() {
    let frame = sample();
    let mut copied = frame.copy();
    copied
        .set("open", Series::new(vec![0.0, 0.0, 0.0]))
        .expect("替换失败");
    assert_eq!(
        frame.get("open").expect("open 列缺失").to_f64(),
        vec![1.0, 2.0, 3.0]
    );
    assert_eq!(
        copied.get("open").expect("open 列缺失").to_f64(),
        vec![0.0, 0.0, 0.0]
    );
}

#[test]
fn test_display() {
    let frame = sample();
    assert_eq!(
        frame.to_string(),
        "DataFrame(shape=(3, 2), columns=[open, close])"
    );
}
