use std::fs;
use std::io::Write;

fn main() {
    // Use crate config to determine meta path via public wrapper
    let meta = quant1x::get_meta_path();
    println!("meta path: {}", meta);
    if let Err(e) = fs::create_dir_all(&meta) {
        eprintln!("failed to create meta dir {}: {}", meta, e);
        std::process::exit(1);
    }
    let secfile = format!("{}/securities.csv", meta);
    println!("writing securities to {}", secfile);
    let codes = vec!["sh600000", "sz000001", "sh600839"];
    match fs::File::create(&secfile) {
        Ok(mut f) => {
            for c in codes {
                if let Err(e) = writeln!(f, "{}", c) {
                    eprintln!("failed to write to {}: {}", secfile, e);
                    std::process::exit(1);
                }
            }
            println!("wrote {} codes to {}", 3, secfile);
        }
        Err(e) => {
            eprintln!("failed to create {}: {}", secfile, e);
            std::process::exit(1);
        }
    }
}
