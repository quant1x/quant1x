use quant1x::factors::base::get_cross_section_forward_adjusted_klines;

fn main() {
    let code = "sh600000";
    let as_of_date = "2024-12-26";

    // 获取前复权K线数据
    let klines = get_cross_section_forward_adjusted_klines(code, as_of_date);

    println!("Loaded {} adjusted kline records for {}", klines.len(), code);

    // 显示最近5条记录
    let start = if klines.len() > 5 { klines.len() - 5 } else { 0 };
    for kline in &klines[start..] {
        println!("Date: {}, Open: {:.2}, Close: {:.2}",
                kline.date, kline.open, kline.close);
    }
}