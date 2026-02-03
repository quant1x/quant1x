use crate::std::filesystem;
use serde::{Deserialize, Serialize};
use serde_yaml;
use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;
use std::sync::OnceLock;

const DEFAULT_BASE_PATH_TEMPLATE: &str = "~/.q1x-rs";
const QUANT1X_CONFIG_FILENAME: &str = "quant1x.yaml";

static QUANT1X_BASE_PATH: OnceLock<String> = OnceLock::new();
static CACHE_CFG: OnceLock<BaseConfig> = OnceLock::new();

// BaseConfig 基础配置结构体
#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct BaseConfig {
    #[serde(default)]
    pub debug: bool, // 是否处于调试模式
    pub basedir: Option<String>, // 基础目录
    pub logdir: Option<String>,  // 日志目录
    #[serde(skip)]
    filename: String, // 配置文件路径
    #[serde(skip)]
    config_map: HashMap<String, serde_yaml::Value>, // 配置数据映射
}

impl BaseConfig {
    fn parse_yaml_config(filename: &str) -> Result<Self, Box<dyn std::error::Error>> {
        let mut config = BaseConfig::default();
        config.filename = filename.trim().to_string();
        config.config_map = HashMap::new();

        // 若配置文件不存在：使用默认 BaseDir/LogDir，并保留空 map
        if !PathBuf::from(filename).exists() {
            config.basedir = Some(get_base_path().to_string());
            config.logdir = Some(format!("{}/logs", config.basedir.as_ref().unwrap()));
            return Ok(config);
        }

        let data = fs::read_to_string(filename)?;
        let node: HashMap<String, serde_yaml::Value> = serde_yaml::from_str(&data)?;
        config.config_map = node;

        // 解析到强类型配置
        let mut typed_config: BaseConfig = serde_yaml::from_str(&data)?;
        typed_config.filename = filename.to_string();
        typed_config.config_map = config.config_map.clone();

        // 处理basedir
        if let Some(ref mut basedir) = typed_config.basedir {
            *basedir = basedir.trim().to_string();
            if basedir.is_empty() {
                *basedir = get_base_path().to_string();
            } else {
                // 展开用户目录
                *basedir = crate::std::filesystem::expand_user(basedir).unwrap_or(basedir.clone());
            }
        } else {
            typed_config.basedir = Some(get_base_path().to_string());
        }

        // 处理logdir
        if let Some(ref mut logdir) = typed_config.logdir {
            let trimmed = logdir.trim();
            if trimmed.is_empty() {
                typed_config.logdir =
                    Some(format!("{}/logs", typed_config.basedir.as_ref().unwrap()));
            } else {
                typed_config.logdir =
                    Some(crate::std::filesystem::expand_user(trimmed).unwrap_or(trimmed.to_string()));
            }
        } else {
            typed_config.logdir = Some(format!("{}/logs", typed_config.basedir.as_ref().unwrap()));
        }

        // 归一化后的值也写回 map
        if let Some(basedir) = &typed_config.basedir {
            typed_config.config_map.insert(
                "basedir".to_string(),
                serde_yaml::Value::String(basedir.clone()),
            );
        }
        if let Some(logdir) = &typed_config.logdir {
            typed_config.config_map.insert(
                "logdir".to_string(),
                serde_yaml::Value::String(logdir.clone()),
            );
        }
        typed_config.config_map.insert(
            "debug".to_string(),
            serde_yaml::Value::Bool(typed_config.debug),
        );

        Ok(typed_config)
    }
}

fn lazy_init_base_path() -> String {
    filesystem::expand_user(DEFAULT_BASE_PATH_TEMPLATE)
        .unwrap_or_else(|_| DEFAULT_BASE_PATH_TEMPLATE.to_string())
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

// 全局函数，与Go/C++版本保持一致的调用方式
pub fn get_configfile_path() -> &'static str {
    ensure_initialized();
    CACHE_CFG.get().unwrap().filename.as_str()
}

pub fn get_logs_path() -> &'static str {
    ensure_initialized();
    CACHE_CFG.get().unwrap().logdir.as_ref().unwrap().as_str()
}

pub fn get_data_path() -> &'static str {
    get_base_path()
}

pub fn get_config_map() -> HashMap<String, serde_yaml::Value> {
    ensure_initialized();
    CACHE_CFG.get().unwrap().config_map.clone()
}

pub fn get_config_map_ref() -> &'static HashMap<String, serde_yaml::Value> {
    ensure_initialized();
    &CACHE_CFG.get().unwrap().config_map
}

fn ensure_initialized() {
    if CACHE_CFG.get().is_none() {
        lazy_init_cache_config().expect("Failed to initialize config");
    }
}

fn lazy_init_cache_config() -> Result<(), Box<dyn std::error::Error>> {
    let cfg_filename = format!("{}/{}", get_base_path(), QUANT1X_CONFIG_FILENAME);
    let config = BaseConfig::parse_yaml_config(&cfg_filename)?;
    CACHE_CFG.set(config).map_err(|_| "Failed to set config")?;
    Ok(())
}
