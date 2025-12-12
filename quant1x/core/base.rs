use std::path::PathBuf;
use std::sync::OnceLock;
use crate::std::filepath;

const DEFAULT_BASE_PATH_TEMPLATE: &str = "~/.q1x-rust";

static QUANT1X_BASE_PATH: OnceLock<String> = OnceLock::new();

fn lazy_init_base_path() -> String {
    filepath::expand_user(DEFAULT_BASE_PATH_TEMPLATE).unwrap_or_else(|_| {
        DEFAULT_BASE_PATH_TEMPLATE.to_string()
    })
}

/// 返回默认的基础路径，如果无法展开用户目录则返回默认路径
pub fn get_base_path() -> &'static str {
    QUANT1X_BASE_PATH.get_or_init(lazy_init_base_path)
}

/// 返回元数据存储的基础路径
///
/// meta目录位于基础路径下的meta子目录中
pub fn get_meta_path() -> PathBuf {
    let base = get_base_path();
    PathBuf::from(base).join("meta")
}
