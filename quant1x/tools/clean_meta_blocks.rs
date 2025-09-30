use quant1x::get_meta_path;
use std::fs;
use std::path::PathBuf;
fn main() {
    let meta = get_meta_path();
    println!("meta path: {}", meta);
    let files = ["block.dat", "block_fg.dat", "block_gn.dat", "block_zs.dat"];
    for f in files.iter() {
        let mut p = PathBuf::from(&meta);
        p.push(f);
        if p.exists() {
            match fs::remove_file(&p) {
                Ok(_) => println!("removed {}", p.display()),
                Err(e) => println!("failed to remove {}: {}", p.display(), e),
            }
        } else {
            println!("not found {}", p.display());
        }
    }
}
