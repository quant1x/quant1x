//! TDX 服务器配置模块
//!
//! 对应 Python `_config.py`:
//! - 标准/扩展服务器候选列表
//! - 服务器缓存读写 (YAML)
//! - 并行探测可用服务器
//!
//! 与 `level1/config.rs` 的关系:
//! - `level1/config.rs` 为 level1 模块提供独立的服务器配置 (标准行情)
//! - 本文件是 `tdx/` 模块的完整配置实现, 对齐 Python `_config.py`

use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;
use std::fs;
use std::io::{Read, Write};
use std::net::{Shutdown, TcpStream as StdTcpStream, ToSocketAddrs};
use std::path::PathBuf;
use std::thread;
use std::time::{Duration, Instant};

use crate::config::get_meta_path;

// ============================================================
// 常量
// ============================================================

/// 最大连接数上限
pub const MAX_CONNECTIONS: usize = 10;

/// 最大探测耗时 (ms)
pub const MAX_ELAPSED_TIME_MS: i64 = 100;

/// 默认连接超时 (ms)
pub const DEFAULT_CONNECT_TIMEOUT_MS: i32 = 1000;

/// 缓存文件名
const CACHE_FILENAME: &str = "server.bin";

/// cron 表达式: 每个交易日 8:55 AM 运行一次, 初始化服务器列表
pub const CRON_EXPR_SERVER_INIT: &str = "0 55 8 * * MON-FRI";

// ============================================================
// ServerInfo
// ============================================================

/// 服务器信息, 对应 Python `_config.py` 中的服务器字典。
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub struct ServerInfo {
    pub source: String,
    pub name: String,
    pub host: String,
    pub port: u16,
    #[serde(default)]
    pub latency_ms: i64,
}

impl ServerInfo {
    pub fn addr(&self) -> String {
        format!("{}:{}", self.host, self.port)
    }
}

// ============================================================
// 标准服务器候选列表 (来自通达信、中信证券、华泰证券、国泰君安)
// 完整对齐 Python `_config.py:StandardServerList`
// ============================================================

/// 返回完整的标准行情服务器候选列表。
pub fn standard_server_list() -> Vec<ServerInfo> {
    vec![
        // ======================== 通达信 ========================
        ServerInfo { source: "通达信".into(), name: "深圳双线主站1".into(), host: "110.41.147.114".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站2".into(), host: "110.41.2.72".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站3".into(), host: "110.41.4.4".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站4".into(), host: "47.113.94.204".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站5".into(), host: "8.129.174.169".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站6".into(), host: "110.41.154.219".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站1".into(), host: "124.70.176.52".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站2".into(), host: "47.100.236.28".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站3".into(), host: "123.60.186.45".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站4".into(), host: "123.60.164.122".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站5".into(), host: "47.116.105.28".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站6".into(), host: "124.70.199.56".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站1".into(), host: "121.36.54.217".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站2".into(), host: "121.36.81.195".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站3".into(), host: "123.249.15.60".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站1".into(), host: "124.71.85.110".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站2".into(), host: "139.9.51.18".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站3".into(), host: "139.159.239.163".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站7".into(), host: "106.14.201.131".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站8".into(), host: "106.14.190.242".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站9".into(), host: "121.36.225.169".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站10".into(), host: "123.60.70.228".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站11".into(), host: "123.60.73.44".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站12".into(), host: "124.70.133.119".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站13".into(), host: "124.71.187.72".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站14".into(), host: "124.71.187.122".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "武汉电信主站1".into(), host: "119.97.185.59".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站7".into(), host: "47.107.64.168".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站4".into(), host: "124.70.75.113".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站4".into(), host: "124.71.9.153".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "上海双线主站15".into(), host: "123.60.84.66".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "深圳双线主站8".into(), host: "47.107.228.47".into(), port: 7719, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站5".into(), host: "120.46.186.223".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站6".into(), host: "124.70.22.210".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "北京双线主站7".into(), host: "139.9.133.247".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站5".into(), host: "116.205.163.254".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站6".into(), host: "116.205.171.132".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "广州双线主站7".into(), host: "116.205.183.150".into(), port: 7709, latency_ms: 0 },
        // ======================== 中信证券 ========================
        ServerInfo { source: "中信证券".into(), name: "上海电信主站Z1".into(), host: "180.153.18.170".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "上海电信主站Z2".into(), host: "180.153.18.171".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "北京联通主站Z1".into(), host: "202.108.253.130".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "北京联通主站Z2".into(), host: "202.108.253.131".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州电信主站J1".into(), host: "60.191.117.167".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州电信主站J2".into(), host: "115.238.56.198".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州电信主站J3".into(), host: "218.75.126.9".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州电信主站J4".into(), host: "115.238.90.165".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州联通主站J1".into(), host: "124.160.88.183".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州联通主站J2".into(), host: "60.12.136.250".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州华数主站J1".into(), host: "218.108.98.244".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州华数主站J2".into(), host: "218.108.47.69".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "济南联通主站W1".into(), host: "27.221.115.131".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "青岛电信主站W1".into(), host: "58.56.180.60".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "深圳电信主站Z1".into(), host: "14.17.75.71".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "云行情上海电信Z1".into(), host: "114.80.63.12".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "云行情上海电信Z2".into(), host: "114.80.63.35".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "上海电信主站Z3".into(), host: "180.153.39.51".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "云行情北京联通Z1".into(), host: "123.125.108.23".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "云行情北京联通Z2".into(), host: "123.125.108.24".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "云行情广州电信Z1".into(), host: "121.201.83.106".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "云行情成都电信Z1".into(), host: "218.6.170.55".into(), port: 7709, latency_ms: 0 },
        // ======================== 华泰证券 ========================
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京电信一)".into(), host: "180.101.48.170".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京电信二)".into(), host: "180.101.48.171".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京移动一)".into(), host: "120.195.71.155".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京移动二)".into(), host: "120.195.71.156".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京联通一)".into(), host: "122.96.107.242".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京联通二)".into(), host: "122.96.107.243".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(亚马逊一)".into(), host: "52.83.39.241".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(亚马逊二)".into(), host: "52.83.199.101".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华南阿里云一)".into(), host: "8.135.57.58".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华南阿里云二)".into(), host: "8.135.62.177".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华东华为云一)".into(), host: "124.70.183.173".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华东华为云二)".into(), host: "124.71.163.106".into(), port: 7709, latency_ms: 0 },
        // ======================== 国泰君安 ========================
        ServerInfo { source: "国泰君安".into(), name: "郑州网通行情一".into(), host: "182.118.47.141".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "郑州网通行情二".into(), host: "182.118.47.168".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "郑州网通行情三".into(), host: "182.118.47.169".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "武汉电信行情一".into(), host: "119.97.164.184".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "武汉电信行情二".into(), host: "119.97.164.189".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "武汉电信行情三".into(), host: "116.211.121.102".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "武汉电信行情四".into(), host: "116.211.121.108".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "武汉电信行情五".into(), host: "116.211.121.31".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "新疆电信云行情一".into(), host: "202.100.166.117".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "新疆电信云行情二".into(), host: "202.100.166.118".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情八".into(), host: "222.73.139.166".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情九".into(), host: "222.73.139.167".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情十".into(), host: "222.73.139.168".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海BGP行情一".into(), host: "103.251.85.90".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "北京联通行情一".into(), host: "123.125.108.213".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "北京联通行情二".into(), host: "123.125.108.214".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情六".into(), host: "222.73.139.151".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情七".into(), host: "222.73.139.152".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "成都BGP行情一".into(), host: "148.70.110.41".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "成都BGP行情二".into(), host: "148.70.93.117".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "成都BGP行情三".into(), host: "148.70.31.16".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "成都BGP行情四".into(), host: "148.70.111.63".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情一".into(), host: "139.159.143.228".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情二".into(), host: "139.159.183.76".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情三".into(), host: "139.159.193.118".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情四".into(), host: "139.159.195.177".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情五".into(), host: "139.159.202.253".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情六".into(), host: "139.159.214.78".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情七".into(), host: "139.9.38.206".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情八".into(), host: "139.9.43.104".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情九".into(), host: "139.9.43.31".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情十".into(), host: "139.9.50.246".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情十一".into(), host: "139.9.52.158".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "广州BGP行情十二".into(), host: "139.9.90.169".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情十一".into(), host: "101.226.180.73".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海电信行情十二".into(), host: "101.226.180.74".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海BGP行情六".into(), host: "103.251.85.200".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海BGP行情七".into(), host: "103.251.85.201".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情一".into(), host: "103.221.142.65".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情二".into(), host: "103.221.142.66".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情三".into(), host: "103.221.142.67".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情四".into(), host: "103.221.142.68".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情五".into(), host: "103.221.142.69".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情六".into(), host: "103.221.142.70".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情七".into(), host: "103.221.142.71".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "南京电信行情八".into(), host: "103.221.142.72".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情一".into(), host: "117.34.114.13".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情二".into(), host: "117.34.114.14".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情三".into(), host: "117.34.114.15".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情四".into(), host: "117.34.114.16".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情五".into(), host: "117.34.114.17".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情六".into(), host: "117.34.114.18".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情七".into(), host: "117.34.114.20".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情八".into(), host: "117.34.114.27".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "西安电信行情九".into(), host: "117.34.114.30".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "上海BGP行情八".into(), host: "103.251.85.202".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情一".into(), host: "183.60.224.142".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情二".into(), host: "183.60.224.143".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情三".into(), host: "183.60.224.144".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情四".into(), host: "183.60.224.145".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情五".into(), host: "183.60.224.146".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情六".into(), host: "183.60.224.147".into(), port: 7709, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "东莞电信行情七".into(), host: "183.60.224.148".into(), port: 7709, latency_ms: 0 },
    ]
}

// ============================================================
// 扩展服务器候选列表 (通达信官方, 支持港股金融指数)
// 完整对齐 Python `_config.py:ExtensionServerList`
// ============================================================

/// 返回通达信官方的扩展市场服务器候选列表 (支持港股金融指数)。
pub fn extension_server_list() -> Vec<ServerInfo> {
    vec![
        // ======================== 通达信 ========================
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线1".into(), host: "112.74.214.43".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线2".into(), host: "120.25.218.6".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线3".into(), host: "43.139.173.246".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线4".into(), host: "159.75.90.107".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线5".into(), host: "106.52.170.195".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线1".into(), host: "150.158.9.199".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线2".into(), host: "150.158.20.127".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线3".into(), host: "49.235.119.116".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线4".into(), host: "49.234.13.160".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场广州双线1".into(), host: "116.205.143.214".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场广州双线2".into(), host: "124.71.223.19".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场广州双线3".into(), host: "139.9.191.175".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场广州双线4".into(), host: "113.45.175.47".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线5".into(), host: "123.60.173.210".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线6".into(), host: "118.89.69.202".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线7".into(), host: "175.24.47.69".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线3".into(), host: "47.107.75.159".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线4".into(), host: "47.106.204.218".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线5".into(), host: "47.106.209.131".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场武汉主站1".into(), host: "119.97.185.5".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场深圳双线6".into(), host: "47.115.94.72".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线1".into(), host: "106.14.95.149".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线2".into(), host: "47.102.108.214".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线3".into(), host: "47.103.86.229".into(), port: 7727, latency_ms: 0 },
        ServerInfo { source: "通达信".into(), name: "扩展市场上海双线4".into(), host: "47.103.88.146".into(), port: 7727, latency_ms: 0 },
    ]
}

// ============================================================
// 券商版扩展服务器候选列表 (不支持港股金融指数)
// 完整对齐 Python `_config.py:ExtensionServerList2`
// ============================================================

/// 返回券商版本的扩展市场服务器候选列表 (不支持港股金融指数)。
pub fn extension_server_list2() -> Vec<ServerInfo> {
    vec![
        // ======================== 中信证券 ========================
        ServerInfo { source: "中信证券".into(), name: "上海电信主站Z1".into(), host: "180.153.18.176".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "北京联通主站Z1".into(), host: "202.108.253.154".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州电信主站J1".into(), host: "115.238.56.196".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州电信主站J2".into(), host: "115.238.90.170".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州联通主站J1".into(), host: "60.12.136.251".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "杭州华数主站J1".into(), host: "218.108.98.244".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "济南联通主站W1".into(), host: "27.221.115.133".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "青岛电信主站W1".into(), host: "58.56.180.60".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "深圳电信主站Z1".into(), host: "14.17.75.71".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "中信证券".into(), name: "广州云电信主站Z1".into(), host: "121.201.83.104".into(), port: 7721, latency_ms: 0 },
        // ======================== 华泰证券 ========================
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京电信一)".into(), host: "180.101.48.170".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京电信二)".into(), host: "180.101.48.171".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京移动一)".into(), host: "120.195.71.155".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京移动二)".into(), host: "120.195.71.156".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京联通一)".into(), host: "122.96.107.242".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(南京联通二)".into(), host: "122.96.107.243".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(亚马逊一)".into(), host: "52.83.39.241".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(亚马逊二)".into(), host: "52.83.199.101".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华南阿里云一)".into(), host: "8.135.57.58".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华南阿里云二)".into(), host: "8.135.62.177".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华东华为云一)".into(), host: "124.70.183.173".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "华泰证券".into(), name: "华泰证券(华东华为云二)".into(), host: "124.71.163.106".into(), port: 7721, latency_ms: 0 },
        // ======================== 国泰君安 ========================
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站1".into(), host: "103.221.142.80".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站2".into(), host: "114.118.82.205".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站3".into(), host: "117.34.114.31".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站4".into(), host: "139.9.52.158".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站5".into(), host: "103.251.85.204".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站6".into(), host: "114.118.82.204".into(), port: 7721, latency_ms: 0 },
        ServerInfo { source: "国泰君安".into(), name: "扩展行情主站7".into(), host: "103.221.142.73".into(), port: 7721, latency_ms: 0 },
    ]
}

// ============================================================
// 缓存读写
// ============================================================

/// 获取缓存文件路径: `<meta_path>/server.bin`
fn cache_filename() -> Option<PathBuf> {
    let meta = get_meta_path();
    let mut p = PathBuf::from(&meta);
    if !p.exists() {
        if let Err(e) = fs::create_dir_all(&p) {
            log::warn!("failed to create meta dir {:?}: {}", p, e);
            return None;
        }
    }
    p.push(CACHE_FILENAME);
    Some(p)
}

/// 将服务器字典写入 YAML 缓存文件。
///
/// 对应 Python `_config.py:save_cached_servers()`。
pub fn save_cached_servers(servers: &BTreeMap<String, Vec<ServerInfo>>) {
    let Some(path) = cache_filename() else {
        return;
    };
    match serde_yaml::to_string(servers) {
        Ok(yaml_str) => {
            if let Err(e) = fs::write(&path, yaml_str) {
                log::warn!("failed to write server cache {:?}: {}", path, e);
            }
        }
        Err(e) => log::warn!("failed to serialize servers for cache: {}", e),
    }
}

/// 从 YAML 缓存文件中读取服务器列表。
///
/// 对应 Python `_config.py:load_cached_servers()`。
/// - key: "standard" 或 "extension"
pub fn load_cached_servers(key: &str) -> Vec<ServerInfo> {
    let Some(path) = cache_filename() else {
        return Vec::new();
    };
    if !path.exists() {
        return Vec::new();
    }

    let content = match fs::read_to_string(&path) {
        Ok(s) => s,
        Err(e) => {
            log::warn!("failed to read server cache {:?}: {}", path, e);
            return Vec::new();
        }
    };

    // Try to parse as BTreeMap<String, Vec<ServerInfo>> (new format)
    if let Ok(map) = serde_yaml::from_str::<BTreeMap<String, Vec<ServerInfo>>>(&content) {
        return map.get(key).cloned().unwrap_or_default();
    }

    // Fallback: parse as legacy Vec<ServerInfo> format
    if let Ok(legacy) = serde_yaml::from_str::<Vec<ServerInfo>>(&content) {
        if key == "standard" {
            return legacy;
        }
    }

    Vec::new()
}

// ============================================================
// 服务器探测
// ============================================================

/// 探测单个候选服务器的连通性。
///
/// 对应 Python `_config.py:_try_probe_one()`。
/// 使用标准协议握手进行验证。
fn try_probe_one(
    candidate: &ServerInfo,
    connect_timeout_ms: i32,
) -> Option<ServerInfo> {
    let host = &candidate.host;
    let port = candidate.port;
    let addr_str = format!("{}:{}", host, port);

    log::debug!("Probing {}:{} ({}) - timeout: {} ms", host, port, candidate.name, connect_timeout_ms);

    let timeout = Duration::from_millis(connect_timeout_ms as u64);
    let start = Instant::now();

    // Resolve address
    let sock_addr = match (&addr_str[..]).to_socket_addrs() {
        Ok(mut addrs) => match addrs.next() {
            Some(a) => a,
            None => {
                log::warn!("Probe failed for {}:{} ({}) - no address resolved", host, port, candidate.name);
                return None;
            }
        },
        Err(e) => {
            log::warn!("Probe failed for {}:{} ({}) - resolve error: {}", host, port, candidate.name, e);
            return None;
        }
    };

    // Connect with timeout
    let mut std_stream = match StdTcpStream::connect_timeout(&sock_addr, timeout) {
        Ok(s) => s,
        Err(e) => {
            log::warn!("Probe timed out for {}:{} ({}) after {} ms: {}", host, port, candidate.name, connect_timeout_ms, e);
            return None;
        }
    };

    // Set TCP_NODELAY
    let _ = std_stream.set_nodelay(true);
    let _ = std_stream.set_read_timeout(Some(timeout));
    let _ = std_stream.set_write_timeout(Some(timeout));

    // Perform protocol handshake via level1 Hello1 + Hello2
    let handshake_result = (|| -> std::io::Result<()> {
        let mut req1 = crate::contrib::data::tdx::level1::std::Hello1Request::new();
        crate::contrib::data::tdx::protocol::process_level1_stream(&mut std_stream, &mut req1)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e.to_string()))?;
        if req1.info.trim().is_empty() {
            return Err(std::io::Error::new(std::io::ErrorKind::Other, "Hello1 response empty"));
        }
        let mut req2 = crate::contrib::data::tdx::level1::std::Hello2Request::new();
        crate::contrib::data::tdx::protocol::process_level1_stream(&mut std_stream, &mut req2)
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e.to_string()))?;
        if req2.info.trim().is_empty() {
            return Err(std::io::Error::new(std::io::ErrorKind::Other, "Hello2 response empty"));
        }
        Ok(())
    })();

    match handshake_result {
        Ok(()) => {
            let elapsed = start.elapsed().as_millis() as i64;
            let _ = std_stream.shutdown(Shutdown::Both);
            log::debug!("Probe succeeded for {}:{} ({}) - {} ms", host, port, candidate.name, elapsed);
            Some(ServerInfo {
                source: candidate.source.clone(),
                name: candidate.name.clone(),
                host: candidate.host.clone(),
                port: candidate.port,
                latency_ms: elapsed,
            })
        }
        Err(e) => {
            let _ = std_stream.shutdown(Shutdown::Both);
            log::warn!("Handshake failed for {}:{} ({}) - Error: {}", host, port, candidate.name, e);
            None
        }
    }
}

/// 并行探测并筛选可用的服务器。
///
/// 对应 Python `_config.py:detect()`。
///
/// # 参数
/// * `elapsed_time_ms` - 探测超时时间 (毫秒), 默认 100ms
/// * `conn_limit` - 每种协议类型返回的最大服务器数量, 默认 10
/// * `connect_timeout_ms` - 单个连接的超时时间 (毫秒), 默认 1000ms
///
/// # 返回值
/// BTreeMap 按协议类型分组:
/// - "standard": 标准行情服务器列表
/// - "extension": 扩展行情服务器列表 (目前返回空, ExtensionProtocolHandler 待实现)
pub fn detect(
    elapsed_time_ms: i64,
    conn_limit: usize,
    connect_timeout_ms: i32,
) -> BTreeMap<String, Vec<ServerInfo>> {
    let mut selected: BTreeMap<String, Vec<ServerInfo>> = BTreeMap::new();

    // ---- 探测标准行情服务器 ----
    let standard_candidates = standard_server_list();
    log::info!("Starting detection for standard servers, total candidates: {}", standard_candidates.len());

    let standard_servers = detect_servers(&standard_candidates, elapsed_time_ms, conn_limit, connect_timeout_ms);
    log::info!("Detection completed for standard servers: {}/{} available", standard_servers.len(), standard_candidates.len());
    selected.insert("standard".to_string(), standard_servers);

    // ---- 探测扩展行情服务器 ----
    // TODO: 扩展行情服务器探测需要 ExtensionProtocolHandler，
    // 当前 ExtensionProtocolHandler 尚未迁移到 Rust。
    // Python 中: resources = [("standard", StandardServerList, StandardProtocolHandler()),
    //                        ("extension", ExtensionServerList, ExtensionProtocolHandler())]
    let extension_candidates = extension_server_list();
    log::info!("Starting detection for extension servers, total candidates: {} (ExtensionProtocolHandler not yet implemented in Rust, skipping)", extension_candidates.len());
    selected.insert("extension".to_string(), Vec::new());

    selected
}

/// 并行探测给定候选服务器列表。
fn detect_servers(
    candidates: &[ServerInfo],
    elapsed_time_ms: i64,
    conn_limit: usize,
    connect_timeout_ms: i32,
) -> Vec<ServerInfo> {
    let n = candidates.len();
    if n == 0 {
        return Vec::new();
    }

    // 线程数 = min(cpu_count, candidates.len())
    let num_threads = match thread::available_parallelism() {
        Ok(nthreads) => std::cmp::min(nthreads.get(), n),
        Err(_) => std::cmp::min(4, n),
    };
    let servers_per_thread = (n + num_threads - 1) / num_threads;

    let mut handles = Vec::new();
    for i in 0..num_threads {
        let slice = candidates.to_vec();
        let start = i * servers_per_thread;
        let end = std::cmp::min(start + servers_per_thread, n);
        let cap = std::cmp::min(conn_limit, MAX_CONNECTIONS);

        handles.push(thread::spawn(move || {
            let mut found: Vec<ServerInfo> = Vec::new();
            for j in start..end {
                if j >= slice.len() {
                    break;
                }
                let candidate = &slice[j];
                if let Some(mut si) = try_probe_one(candidate, connect_timeout_ms) {
                    // 只保留延迟低于阈值的
                    if si.latency_ms < elapsed_time_ms {
                        // 如果延迟为0 (即测量失败), 设为一个大值方便排序
                        if si.latency_ms == 0 {
                            si.latency_ms = elapsed_time_ms;
                        }
                        found.push(si);
                    }
                }
            }
            // 每线程内排序截断
            if found.len() > cap {
                found.sort_by_key(|s| s.latency_ms);
                found.truncate(cap);
            }
            found
        }));
    }

    let mut results: Vec<ServerInfo> = Vec::new();
    for h in handles {
        if let Ok(mut v) = h.join() {
            results.append(&mut v);
        }
    }

    // 全局排序截断
    results.sort_by_key(|s| s.latency_ms);
    let limit = std::cmp::min(results.len(), std::cmp::min(conn_limit, MAX_CONNECTIONS));
    results.into_iter().take(limit).collect()
}

// ============================================================
// 测试
// ============================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_standard_server_list_not_empty() {
        let list = standard_server_list();
        assert!(!list.is_empty());
        // 验证第一个和最后一个服务器
        assert_eq!(list[0].source, "通达信");
        assert_eq!(list[0].name, "深圳双线主站1");
    }

    #[test]
    fn test_extension_server_list_not_empty() {
        let list = extension_server_list();
        assert!(!list.is_empty());
        assert_eq!(list[0].source, "通达信");
        assert!(list[0].name.starts_with("扩展市场"));
    }

    #[test]
    fn test_extension_server_list2_not_empty() {
        let list = extension_server_list2();
        assert!(!list.is_empty());
    }

    #[test]
    fn test_server_info_addr() {
        let s = ServerInfo {
            source: "通达信".into(),
            name: "test".into(),
            host: "127.0.0.1".into(),
            port: 7709,
            latency_ms: 0,
        };
        assert_eq!(s.addr(), "127.0.0.1:7709");
    }

    #[test]
    fn test_load_cached_servers_empty_on_no_file() {
        // 读取不存在的 key 应返回空列表
        let result = load_cached_servers("nonexistent_key");
        assert!(result.is_empty());
    }
}
