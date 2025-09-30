use quant1x::get_meta_path;
use std::fs;
fn main() {
    let meta = get_meta_path();
    println!("meta path: {}", meta);
    match fs::read_dir(&meta) {
        Ok(rd) => {
            for e in rd {
                if let Ok(ent) = e {
                    if let Ok(md) = ent.metadata() {
                        println!("{}\t{}", ent.file_name().to_string_lossy(), md.len());
                    }
                }
            }
        }
        Err(e) => println!("failed to read meta dir: {}", e),
    }
}
