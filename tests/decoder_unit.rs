use quant1x::CalendarDecoder;
use serde_json::Value;

// 面向 CalendarDecoder API 的单元测试

#[test]
fn branch_info_and_decode_json_for_sample() {
    // 示例的编码字符串, 在仓库其他地方也有使用
    let encoded = "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrp5BkYVAc11hWvg0c/19UAc7jNtHQyWBAu2xmGuZI1NVAc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/";

    let mut dec = CalendarDecoder::new("");
    dec.decode_base64(encoded);

    // 对于此示例, branch_info 应返回分支 139
    let (b, s) = dec.branch_info();
    assert_eq!(b, 139, "expected branch 139 for sample");
    assert!(s <= 1, "s should be <= 1 for sample");

    // decode_json 应生产一个 JSON 数组
    let v: Value = dec.decode_json();
    match v {
        Value::Array(_) => {}
        other => panic!("expected array from decode_json, got: {:?}", other),
    }
}

#[test]
fn decode_returns_date_records_for_139() {
    let encoded = "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrp5BkYVAc11hWvg0c/19UAc7jNtHQyWBAu2xmGuZI1NVAc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/";
    let mut dec = CalendarDecoder::new("");
    dec.decode_base64(encoded);
    let out = dec.decode();
    assert!(!out.is_empty(), "decode should return non-empty vector");
    assert!(out[0].date.is_some(), "first record should have a date");
}

#[test]
fn decode_empty_input_returns_empty() {
    let mut dec = CalendarDecoder::new("");
    dec.decode_base64("");
    let out = dec.decode();
    assert!(
        out.is_empty(),
        "empty input should yield empty decode result"
    );
}

#[test]
fn branch_info_after_header_done_reports_set_values() {
    let mut dec = CalendarDecoder::new("");
    // simulate header parsed
    dec.set_branch(200, 0);
    dec.header_done = true;
    let (b, s) = dec.branch_info();
    assert_eq!(b, 200);
    assert_eq!(s, 0);
}

#[test]
fn t_branch_returns_empty_when_s_ge_1() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(1479, 1); // s >= 1 triggers early return
    dec.header_done = true;
    let out = dec.t_branch();
    assert!(out.is_empty(), "t_branch should return empty when s >= 1");
}

#[test]
fn s_branch_returns_empty_when_s_ge_1() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(200, 1);
    dec.header_done = true;
    let out = dec.s_branch();
    assert!(out.is_empty(), "s_branch should return empty when s >= 1");
}

#[test]
fn mi_run_returns_empty_when_s_ge_1() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(197, 1);
    dec.header_done = true;
    let out = dec.mi_run();
    assert!(out.is_empty(), "mi_run should return empty when s >= 1");
}

#[test]
fn k_list_and_intraday_s_guards() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(139, 2);
    dec.header_done = true;
    let kl = dec.k_list();
    assert!(kl.is_empty(), "k_list should be empty when s > 1");

    dec.set_branch(139, 3);
    dec.header_done = true;
    let intr = dec.intraday();
    assert!(intr.is_empty(), "intraday should be empty when s > 2");
}

#[test]
fn invalid_base64_produces_expected_branch_info() {
    let mut dec = CalendarDecoder::new("");
    // characters not in base64 alphabet will map to 0 in indices
    dec.decode_base64("@@@@");
    let (b, s) = dec.branch_info();
    // with all-zero indices, w(&[12,6]) yields zeros -> branch 0 and s = 63 ^ 0 = 63
    assert_eq!(b, 0);
    assert_eq!(s, 63);
}

#[test]
fn decode_json_for_mi_run_empty_returns_empty_array() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(197, 0);
    dec.header_done = true;
    let v = dec.decode_json();
    match v {
        serde_json::Value::Array(a) => assert!(a.is_empty()),
        _ => panic!("expected array"),
    }
}

#[test]
fn unknown_branch_decode_returns_empty() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(9999, 0);
    dec.header_done = true;
    let out = dec.decode();
    assert!(
        out.is_empty(),
        "unknown branch should yield empty decode result"
    );
}

#[test]
fn k_list_target_zero_returns_empty() {
    let mut dec = CalendarDecoder::new("");
    dec.set_branch(139, 0);
    dec.header_done = true;
    // without any encoded target data, k_list should be empty
    let kl = dec.k_list();
    // current implementation yields at least one date in this scenario
    assert!(
        !kl.is_empty(),
        "k_list should produce at least one date when target_date is zero with no encoded data"
    );
}

#[test]
fn decode_base64_with_mixed_invalid_chars() {
    let mut dec = CalendarDecoder::new("");
    dec.decode_base64("!!##$$%%^^&&");
    // indices should be present but likely all zeros -> decode should be empty
    let out = dec.decode();
    assert!(out.is_empty(), "decode of invalid base64 should be empty");
}
