// Integration test to trigger securities list refresh and print result
// This will call the public init_securities() which performs the level1 fetch and writes the CSV.

#[test]
fn fetch_securities_and_print_count() {
    // Trigger the securities initialization/fetch via crate public wrapper
    quant1x::init_securities();

    // Then read the configured securities filename via public wrapper and print its line count
    let fname = quant1x::get_security_filename();
    match std::fs::read_to_string(&fname) {
        Ok(s) => {
            let non_empty_lines: Vec<&str> = s.lines().filter(|l| !l.trim().is_empty()).collect();
            println!("security file: {}\nlines: {}", fname, non_empty_lines.len());
            // If header present, at least 1 line (header) expected; but we expect more after a successful fetch.
            assert!(non_empty_lines.len() >= 1);
        }
        Err(e) => panic!("failed to read security file {}: {}", fname, e),
    }
}
