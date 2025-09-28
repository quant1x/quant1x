#[test]
fn print_stock_code_count() {
    // `exchange` items are re-exported at crate root, call the public wrapper
    let list = quant1x::get_stock_code_list();
    println!("STOCK_CODE_COUNT:{}", list.len());
    assert!(list.len() > 0);
}
