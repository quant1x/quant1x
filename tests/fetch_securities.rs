// 用于触发证券列表刷新并打印结果的集成测试
// 该测试会调用公共的 init_securities()，该函数执行 level1 抓取并写入 CSV 文件。

#[test]
fn fetch_securities_and_print_count() {
    // 通过 crate 的公共封装触发证券初始化/抓取
    quant1x::init_securities();

    // 然后通过公共封装读取配置的证券文件名并打印其行数
    let fname = quant1x::get_security_filename();
    match std::fs::read_to_string(&fname) {
        Ok(s) => {
            let non_empty_lines: Vec<&str> = s.lines().filter(|l| !l.trim().is_empty()).collect();
            println!("security file: {}\nlines: {}", fname, non_empty_lines.len());
            // 如果存在表头，至少期望 1 行（表头）；但成功抓取后我们期望更多行。
            assert!(non_empty_lines.len() >= 1);
        }
        Err(e) => panic!("failed to read security file {}: {}", fname, e),
    }
}
