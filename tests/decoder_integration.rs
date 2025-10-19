use quant1x::CalendarDecoder;
use serde_json::Value;

#[test]
fn sample_decodes_and_reports_branch() {
    let encoded = "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrp5BkYVAc11hWvg0c/19UAc7jNtHQyWBAu2xmGuZI1NVAc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/";

    let mut dec = CalendarDecoder::new("");
    dec.decode_base64(encoded);

    let bi = dec.branch_info();
    // 该示例已知属于分支 139（k_list/日期列表）
    assert_eq!(bi.0, 139);
    // 对于此示例，s 应 <= 1
    assert!(bi.1 <= 1);

    let v: Value = dec.decode_json();
    // 确保返回的是数组
    match v {
        Value::Array(_) => (),
        _ => panic!("expected JSON array from decode_json"),
    }
}
