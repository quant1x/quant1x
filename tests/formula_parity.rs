// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

//! formula 跨语言对拍: 读 `tests/fixtures/formula/cases.csv`, 逐值比较 Rust 与 Python 输出。
//!
//! 期望值只由 Python spec 生成; 本用例只做比对, 不参与生成。
//! 浮点按逐位相等比较, NaN 视为相等。

use std::fs;
use std::path::PathBuf;

use quant1x::formula as f;
use quant1x::num;

/// 一行样本: 每个位置一行, 序列由同组多行反推
#[derive(Clone)]
struct Row {
    label: String,
    case: String,
    a: f64,
    b: f64,
    c: f64,
    w: Option<i64>,
    n: Option<f64>,
    m: Option<f64>,
    b_present: bool,
    expected: f64,
}

fn parse_double(text: &str) -> f64 {
    if text.is_empty() || text == "nan" {
        return f64::NAN;
    }
    text.parse::<f64>().unwrap_or(f64::NAN)
}

fn locate() -> PathBuf {
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    root.join("tests")
        .join("fixtures")
        .join("formula")
        .join("cases.csv")
}

fn same(left: f64, right: f64) -> bool {
    (left.is_nan() && right.is_nan()) || left == right
}

fn compare(actual: &[f64], expected: &[f64]) -> Result<(), String> {
    if actual.len() != expected.len() {
        return Err(format!(
            "长度不一致: {} vs {}",
            actual.len(),
            expected.len()
        ));
    }
    for (index, (left, right)) in actual.iter().zip(expected).enumerate() {
        if !same(*left, *right) {
            return Err(format!("位置 {}: {} vs {}", index, left, right));
        }
    }
    Ok(())
}

fn column(rows: &[Row], pick: fn(&Row) -> f64) -> Vec<f64> {
    rows.iter().map(pick).collect()
}

fn window_of(rows: &[Row], has_window: bool, ni: i64) -> num::Window {
    if has_window {
        num::Window::Series(rows.iter().map(|row| row.w.unwrap_or(-1)).collect())
    } else {
        num::Window::Scalar(ni)
    }
}

fn evaluate(label: &str, rows: &[Row]) -> Result<Vec<f64>, String> {
    let a = column(rows, |row| row.a);
    let b = column(rows, |row| row.b);
    let c = column(rows, |row| row.c);
    let n = rows[0].n.unwrap_or(0.0);
    let m = rows[0].m.unwrap_or(0.0);
    let ni = n as i64;
    let mi = m as i64;
    let has_window = rows[0].w.is_some();
    let has_b = rows[0].b_present;
    let has_n = rows[0].n.is_some();

    let result = match label {
        "abs" => f::abs(&a),
        "sqrt" => f::sqrt(&a),
        "ln" => f::log(&a),
        "pow" => f::pow(&a, n),
        "max" => f::maximum(&a, &b),
        "min" => f::minimum(&a, &b),
        "if" => f::if_select(&f::as_bools(&a), &b, &c),
        "ref" => {
            if has_window {
                f::ref_window(&a, &window_of(rows, has_window, ni))
            } else {
                f::ref_value(&a, ni)
            }
        }
        "diff" => f::diff(&a, ni),
        "sum" => f::sum(&a, window_of(rows, has_window, ni)),
        "const" => f::const_series(&a),
        "hhv" => f::hhv(&a, window_of(rows, has_window, ni)),
        "llv" => f::llv(&a, window_of(rows, has_window, ni)),
        "hhvbars" => f::hhvbars(&a, ni),
        "llvbars" => f::llvbars(&a, ni),
        "ma" => f::ma(&a, window_of(rows, has_window, ni)),
        "ema" => f::ema(&a, ni),
        "sma" => f::sma(&a, ni, mi),
        "wma" => f::wma(&a, ni),
        "std" => f::stddev(&a, ni),
        "avedev" => f::avedev(&a, ni),
        "slope" => f::slope(&a, ni),
        "forcast" => f::forcast(&a, ni),
        "dma" => {
            if has_b {
                f::dma_series(&a, &b)
            } else {
                f::dma(&a, n)
            }
        }
        "last" => f::as_series(&f::last(&f::as_bools(&a), ni, mi)),
        "count" => f::count(&f::as_bools(&a), ni),
        "every" => f::as_series(&f::every(&f::as_bools(&a), ni)),
        "exist" => f::as_series(&f::exist(&f::as_bools(&a), ni)),
        "filter" => f::filter(&f::as_bools(&a), ni),
        "barslast" => f::barslast(&f::as_bools(&a)),
        "barslastcount" => f::barslastcount(&f::as_bools(&a)),
        "barssincen" => f::barssincen(&f::as_bools(&a), ni),
        "cross" => f::as_series(&f::cross(&a, &b)),
        "longcross" => f::as_series(&f::longcross(&a, &b, ni)),
        "valuewhen" => f::valuewhen(&f::as_bools(&a), &b),
        "between" => f::as_series(&f::between(&a, n, m)),
        "toprange" => f::toprange(&a),
        "lowrange" => f::lowrange(&a),
        "rsi" => f::rsi(&a, if has_n { ni } else { 24 }),
        "ztprice" => f::ztprice(&a, n),
        "macd.dif" => f::macd(&a, 12, 26, 9).0,
        "macd.dea" => f::macd(&a, 12, 26, 9).1,
        "macd.macd" => f::macd(&a, 12, 26, 9).2,
        "kdj.k" => f::kdj(&a, &b, &c, 9, 3, 3).0,
        "kdj.d" => f::kdj(&a, &b, &c, 9, 3, 3).1,
        "kdj.j" => f::kdj(&a, &b, &c, 9, 3, 3).2,
        "boll.upper" => f::boll(&a, 20, 2.0).0,
        "boll.mid" => f::boll(&a, 20, 2.0).1,
        "boll.lower" => f::boll(&a, 20, 2.0).2,
        other => return Err(format!("未覆盖: {}", other)),
    };
    Ok(result)
}

#[test]
fn formula_parity() {
    let content = fs::read_to_string(locate()).expect("读取 cases.csv 失败");
    let mut lines = content.lines();
    lines.next(); // 表头

    let mut groups: Vec<Vec<Row>> = Vec::new();
    for line in lines {
        if line.trim().is_empty() {
            continue;
        }
        let fields: Vec<&str> = line.split(',').collect();
        assert_eq!(fields.len(), 11, "列数错误: {}", line);
        let row = Row {
            label: fields[0].to_string(),
            case: fields[1].to_string(),
            a: parse_double(fields[3]),
            b: parse_double(fields[4]),
            c: parse_double(fields[5]),
            w: if fields[7].is_empty() {
                None
            } else {
                Some(parse_double(fields[7]) as i64)
            },
            n: if fields[8].is_empty() {
                None
            } else {
                Some(parse_double(fields[8]))
            },
            m: if fields[9].is_empty() {
                None
            } else {
                Some(parse_double(fields[9]))
            },
            b_present: !fields[4].is_empty(),
            expected: parse_double(fields[10]),
        };
        match groups.last_mut() {
            Some(group) if group[0].label == row.label && group[0].case == row.case => {
                group.push(row);
            }
            _ => groups.push(vec![row]),
        }
    }

    let mut failures = 0;
    let mut cases = 0;
    for group in &groups {
        cases += 1;
        let label = &group[0].label;
        let case = &group[0].case;
        let expected: Vec<f64> = group.iter().map(|row| row.expected).collect();
        match evaluate(label, group) {
            Ok(actual) => {
                if let Err(message) = compare(&actual, &expected) {
                    failures += 1;
                    eprintln!("不一致: {} / {}: {}", label, case, message);
                }
            }
            Err(message) => {
                failures += 1;
                eprintln!("错误: {} / {}: {}", label, case, message);
            }
        }
    }

    println!("formula parity: {} 组, {} 失败", cases, failures);
    assert_eq!(failures, 0, "{} 组不一致", failures);
}
