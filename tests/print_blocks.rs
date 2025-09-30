use quant1x;

#[test]
fn print_blocks_from_meta() {
    // Print runtime meta path and block list to help debugging where the data is loaded from.
    let meta = quant1x::get_meta_path();
    println!("meta path: {}", meta);

    // Force a sync (reload) and print the results
    let list = quant1x::sync_block_files();
    println!("sync_block_files returned {} blocks", list.len());
    for b in list.iter().take(200) {
        println!("code={} name={} tp={} num={} block={} constituents={}",
            b.code, b.name, b.tp, b.num, b.block, b.constituent_stocks.len());
    }
}
