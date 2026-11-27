// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! indicators 跨语言对拍: 读 `tests/fixtures/formula/indicators/` 的 K 线与期望列,
//! 逐值比较 Rust 与 Python 输出。期望值只由 Python 生成。

use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;

use quant1x::data::schema::Bar;
use quant1x::indicators;

fn fixtures() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("tests")
        .join("fixtures")
        .join("formula")
        .join("indicators")
}

/// 读整张表: 表头 + 行
fn read_table(name: &str) -> (Vec<String>, Vec<Vec<String>>) {
    let path = fixtures().join(name);
    let content = fs::read_to_string(&path).unwrap_or_else(|_| panic!("读取 {:?} 失败", path));
    let mut lines = content.lines();
    let header: Vec<String> = lines
        .next()
        .expect("空表")
        .split(',')
        .map(|s| s.trim().to_string())
        .collect();
    let rows = lines
        .filter(|line| !line.trim().is_empty())
        .map(|line| line.split(',').map(|s| s.trim().to_string()).collect())
        .collect();
    (header, rows)
}

fn parse_double(text: &str) -> f64 {
    if text.is_empty() {
        return f64::NAN;
    }
    text.parse::<f64>().unwrap_or(f64::NAN)
}

fn parse_bool(text: &str) -> bool {
    !text.is_empty() && text != "0"
}

fn load_bars() -> Vec<Bar> {
    let (header, rows) = read_table("bars.csv");
    let index: HashMap<&str, usize> = header
        .iter()
        .enumerate()
        .map(|(i, h)| (h.as_str(), i))
        .collect();
    rows.iter()
        .map(|row| Bar {
            date: row[index["date"]].clone(),
            open: parse_double(&row[index["open"]]),
            close: parse_double(&row[index["close"]]),
            high: parse_double(&row[index["high"]]),
            low: parse_double(&row[index["low"]]),
            volume: parse_double(&row[index["volume"]]),
            amount: parse_double(&row[index["amount"]]),
            ..Default::default()
        })
        .collect()
}

/// 取期望列
fn expected_column(name: &str, column: &str) -> Vec<String> {
    let (header, rows) = read_table(name);
    let index = header
        .iter()
        .position(|h| h == column)
        .unwrap_or_else(|| panic!("{} 缺列 {}", name, column));
    rows.iter().map(|row| row[index].clone()).collect()
}

fn compare_series(actual: &[f64], expected: &[String], label: &str) -> usize {
    let mut failures = 0;
    for (index, (left, right)) in actual.iter().zip(expected).enumerate() {
        let value = parse_double(right);
        let equal = (left.is_nan() && value.is_nan()) || *left == value;
        if !equal {
            failures += 1;
            eprintln!("{} [{}]: {} vs {}", label, index, left, value);
        }
    }
    failures
}

fn compare_bools(actual: &[bool], expected: &[String], label: &str) -> usize {
    let mut failures = 0;
    for (index, (left, right)) in actual.iter().zip(expected).enumerate() {
        if *left != parse_bool(right) {
            failures += 1;
            eprintln!("{} [{}]: {} vs {}", label, index, left, right);
        }
    }
    failures
}

#[test]
fn indicators_parity() {
    let bars = load_bars();
    let mut failures = 0;

    let cdtd = indicators::cdtd(&bars);
    failures += compare_series(
        &cdtd.wen,
        &expected_column("cdtd.csv", "cdtd.wen"),
        "cdtd.wen",
    );
    failures += compare_bools(&cdtd.s1, &expected_column("cdtd.csv", "cdtd.S1"), "cdtd.S1");
    failures += compare_bools(&cdtd.b, &expected_column("cdtd.csv", "cdtd.B"), "cdtd.B");
    failures += compare_bools(&cdtd.s2, &expected_column("cdtd.csv", "cdtd.S2"), "cdtd.S2");
    failures += compare_bools(&cdtd.s3, &expected_column("cdtd.csv", "cdtd.S3"), "cdtd.S3");

    let f89k = indicators::f89k(&bars);
    failures += compare_series(&f89k.zs, &expected_column("f89k.csv", "f89k.zs"), "f89k.zs");
    failures += compare_bools(&f89k.b, &expected_column("f89k.csv", "f89k.b"), "f89k.b");
    failures += compare_series(&f89k.dn, &expected_column("f89k.csv", "f89k.DN"), "f89k.DN");

    let hjk = indicators::hjk(&bars, 0.10);
    failures += compare_bools(&hjk.b, &expected_column("hjk.csv", "hjk.b"), "hjk.b");

    let ma1x = indicators::ma1x(&bars);
    failures += compare_series(&ma1x.dn, &expected_column("ma1x.csv", "DN"), "ma1x.DN");
    failures += compare_series(&ma1x.zs, &expected_column("ma1x.csv", "ZS"), "ma1x.ZS");
    failures += compare_series(&ma1x.yl, &expected_column("ma1x.csv", "YL"), "ma1x.YL");
    failures += compare_bools(&ma1x.hc, &expected_column("ma1x.csv", "ma1x.hc"), "ma1x.hc");
    failures += compare_bools(&ma1x.tp, &expected_column("ma1x.csv", "ma1x.tp"), "ma1x.tp");

    let p2fb = indicators::p2fb(&bars, 0.10);
    failures += compare_series(
        &p2fb.hgzx,
        &expected_column("p2fb.csv", "p2fb.hgzx"),
        "p2fb.hgzx",
    );

    println!("indicators parity: {} 失败", failures);
    assert_eq!(failures, 0, "{} 个值不一致", failures);
}
