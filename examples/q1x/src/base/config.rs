use std::env;
use std::path::{Path, PathBuf};
use anyhow::Context;
use ctor::ctor;
use log::info;
use once_cell::sync::Lazy;
use serde::Deserialize;

/// 缓存路径
#[derive(Debug)]
pub struct CachePath {
    #[allow(dead_code)]
    pub home_path: String, // 宿主目录, 也可能是通过环境变量GOX_HOME设置的
    #[allow(dead_code)]
    pub main_path: String, // 系统数据的主目录
    #[allow(dead_code)]
    pub basedir: String, // 配置文件指定的基础路径
    pub data_meta_path: String, // 元数据路径
    pub data_day_path: String, // 日线路径
}

impl CachePath {
    /// 元数据路径
    pub fn meta_path(&self, path: &str  ) -> PathBuf {
        PathBuf::from(&self.data_meta_path).join(path)
    }

    /// 主路径
    pub fn main_path(&self, path: &str) -> PathBuf {
        PathBuf::from(&self.main_path).join(path)
    }

    pub fn day_path(&self, path: &str) -> PathBuf {
        PathBuf::from(&self.data_day_path).join(path)
    }
}

pub(crate) static QUANT1X_CACHE_CONFIG: Lazy<CachePath> = Lazy::new(|| {
   lazy_local_config()
});

pub fn get_meta_path(path_or_filename: &str) -> PathBuf {
    QUANT1X_CACHE_CONFIG.meta_path(path_or_filename)
}

pub fn get_main_path(path: &str) -> PathBuf {
    QUANT1X_CACHE_CONFIG.main_path(path)
}

pub fn get_day_path(path_or_filename: &str) -> PathBuf {
    QUANT1X_CACHE_CONFIG.day_path(path_or_filename)
}


/// 初始化缓存
#[ctor]
fn cache_init() {
    info!("base.config[{}:{}] 自动初始化...", file!(), line!());
    // let quant1x_home = get_quant1x_home();
    // let config = load_config().unwrap();
    // let basedir = config.basedir;
    // let data_meta_path = Path::join(quant1x_home.as_ref(), "data");
    // let data_day_path = Path::join(basedir.as_ref(), "day");
    // let cfg = CachePath {
    //     home_path: quant1x_home,
    //     main_path: basedir.to_string(),
    //     basedir:basedir.to_string(),
    //     data_meta_path: data_meta_path.display().to_string(),
    //     data_day_path: data_day_path.display().to_string(),
    // };
}

/// 加载配置文件
fn lazy_local_config() -> CachePath {
    let user_home = get_quant1x_user_home();
    let quant1x_home = Path::join(user_home.as_ref(), ".quant1x").display().to_string();
    let config = load_config().unwrap();
    let basedir = config.basedir;
    let data_meta_path = Path::join(quant1x_home.as_ref(), "meta");
    let data_day_path = Path::join(basedir.as_ref(), "day");
    let cfg = CachePath {
        home_path: quant1x_home,
        main_path: basedir.to_string(),
        basedir:basedir.to_string(),
        data_meta_path: data_meta_path.display().to_string(),
        data_day_path: data_day_path.display().to_string(),
    };
    cfg
}

/// 获取用户宿主目录, 环境变量GOX_HOME优先与HOME
fn get_quant1x_user_home() -> String {
    let gox_home = env::var("GOX_HOME");
    match gox_home {
        Ok(home_path) => {
            home_path
        }
        Err(_) => {
            let user_home = dirs::home_dir();
            match user_home {
                Some(path) => {
                    path.display().to_string()
                }
                None => {
                    env::temp_dir().display().to_string()
                }
            }
        }
    }
}

#[derive(Debug, Deserialize)]
struct Quant1xYamlConfig {
    basedir: String,
}

fn load_config() -> anyhow::Result<Quant1xYamlConfig> {
    let config_path = get_config_filename()?;
    let file = std::fs::File::open(&config_path)
        .with_context(|| format!("Config file not found: {}", config_path.display()))?;
    serde_yaml::from_reader(std::io::BufReader::new(file))
        .context("Failed to parse config")
}

fn get_config_filename() -> anyhow::Result<PathBuf> {
    let candidates = [
        PathBuf::from("~/.quant1x/quant1x.yaml"),
        PathBuf::from("~/runtime/etc/quant1x.yaml"),
    ];
    candidates.iter()
        .find_map(|p| shellexpand::tilde(&p.to_string_lossy()).parse().ok())
        .context("No valid config found")
}

// fn calculate_main_path(quant1x_home: &Path) -> anyhow::Result<PathBuf> {
//     let user_home = dirs::home_dir().context("Home directory not available")?;
//     Ok(if quant1x_home == user_home {
//         user_home.join(".quant1x")
//     } else {
//         user_home
//     })
// }

// fn process_basedir(raw: String, fallback: &Path) -> anyhow::Result<PathBuf> {
//     let trimmed = raw.trim();
//     Ok(if trimmed.is_empty() {
//         fallback.join("data")
//     } else {
//         PathBuf::from(shellexpand::tilde(trimmed).into_owned())
//     })
// }

#[cfg(test)]
mod tests1 {
    use super::*;

    #[test]
    fn test_auto_init() {
        println!("data_meta_path = {:?}", QUANT1X_CACHE_CONFIG.data_meta_path);
        println!(" data_day_path = {:?}", QUANT1X_CACHE_CONFIG.data_day_path);
    }

    #[test]
    fn test_load_config() {
        let quant1x_home = get_quant1x_user_home();
        println!("quant1x_home: {}", quant1x_home);
    }
}