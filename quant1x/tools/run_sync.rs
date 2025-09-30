use quant1x::exchange;

fn main() {
    env_logger::init();
    log::info!("run_sync: starting sync_block_files()");
    let list = exchange::sync_block_files();
    log::info!("run_sync: completed sync_block_files(), parsed {} blocks", list.len());
    for (i, b) in list.iter().enumerate() {
        println!("#{}: code='{}' name='{}' num={} constituents={}", i, b.code, b.name, b.num, b.constituent_stocks.len());
    }
}
