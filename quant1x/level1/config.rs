use std::net::{ToSocketAddrs, TcpStream};
use std::time::{Duration, Instant};
use std::thread;
use std::fs;
use std::path::PathBuf;
use serde::{Deserialize, Serialize};

/// ServerInfo mirrors the C++ struct used by the level1 client detection logic.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ServerInfo {
    pub name: String,
    pub desc: String,
    pub host: String,
    pub port: u16,
    pub latency_ms: i64,
}

impl ServerInfo {
    pub fn addr(&self) -> String { format!("{}:{}", self.host, self.port) }
}

/// Default cached server filename. Match C++ implementation which stores the
/// detected server list in the crate meta path as `server.bin`.
fn default_config_path() -> Option<PathBuf> {
    // Use crate config's meta path (parity with C++)
    let meta = crate::config::get_meta_path();
    let mut p = PathBuf::from(meta);
    if !p.exists() {
        if let Err(e) = fs::create_dir_all(&p) {
            log::warn!("failed to create meta dir {:?}: {}", p, e);
            return None;
        }
    }
    p.push("server.bin");
    Some(p)
}

/// Load cached server list from the crate meta `server.bin` (if present).
pub fn load_cached_servers() -> Option<Vec<ServerInfo>> {
    if let Some(path) = default_config_path() {
        if path.exists() {
            match fs::read_to_string(&path) {
                Ok(s) => match serde_yaml::from_str::<Vec<ServerInfo>>(&s) {
                    Ok(v) => return Some(v),
                    Err(e) => log::warn!("failed to parse cached config {:?}: {}", path, e),
                },
                Err(e) => log::warn!("failed to read cached config {:?}: {}", path, e),
            }
        }
    }
    None
}

/// Save servers to the crate meta `server.bin` (best-effort). This mirrors the
/// C++ behavior where the detected server list is stored in
/// config::get_meta_path() + "/server.bin".
pub fn save_cached_servers(servers: &[ServerInfo]) {
    if let Some(path) = default_config_path() {
        match serde_yaml::to_string(servers) {
            Ok(s) => {
                if let Err(e) = fs::write(&path, s) {
                    log::warn!("failed to write cached config {:?}: {}", path, e);
                }
            }
            Err(e) => log::warn!("failed to serialize servers for cache: {}", e),
        }
    }
}

/// A small standard server list (partial) copied from the C++ port for detection.
fn standard_server_list() -> Vec<ServerInfo> {
    vec![
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站1".to_string(), host: "110.41.147.114".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站2".to_string(), host: "110.41.2.72".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站3".to_string(), host: "110.41.4.4".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站4".to_string(), host: "47.113.94.204".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站5".to_string(), host: "8.129.174.169".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站6".to_string(), host: "110.41.154.219".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站1".to_string(), host: "124.70.176.52".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站2".to_string(), host: "47.100.236.28".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站3".to_string(), host: "123.60.186.45".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站4".to_string(), host: "123.60.164.122".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站5".to_string(), host: "47.116.105.28".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站6".to_string(), host: "124.70.199.56".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站1".to_string(), host: "121.36.54.217".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站2".to_string(), host: "121.36.81.195".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站3".to_string(), host: "123.249.15.60".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站1".to_string(), host: "124.71.85.110".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站2".to_string(), host: "139.9.51.18".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站3".to_string(), host: "139.159.239.163".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站7".to_string(), host: "106.14.201.131".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站8".to_string(), host: "106.14.190.242".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站9".to_string(), host: "121.36.225.169".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站10".to_string(), host: "123.60.70.228".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站11".to_string(), host: "123.60.73.44".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站12".to_string(), host: "124.70.133.119".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站13".to_string(), host: "124.71.187.72".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站14".to_string(), host: "124.71.187.122".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "武汉电信主站1".to_string(), host: "119.97.185.59".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站7".to_string(), host: "47.107.64.168".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站4".to_string(), host: "124.70.75.113".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站4".to_string(), host: "124.71.9.153".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "上海双线主站15".to_string(), host: "123.60.84.66".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "深圳双线主站8".to_string(), host: "47.107.228.47".to_string(), port: 7719, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站5".to_string(), host: "120.46.186.223".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站6".to_string(), host: "124.70.22.210".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "北京双线主站7".to_string(), host: "139.9.133.247".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站5".to_string(), host: "116.205.163.254".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站6".to_string(), host: "116.205.171.132".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "广州双线主站7".to_string(), host: "116.205.183.150".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "上海电信主站Z1".to_string(), host: "180.153.18.170".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "上海电信主站Z2".to_string(), host: "180.153.18.171".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "北京联通主站Z1".to_string(), host: "202.108.253.130".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "北京联通主站Z2".to_string(), host: "202.108.253.131".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州电信主站J1".to_string(), host: "60.191.117.167".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州电信主站J2".to_string(), host: "115.238.56.198".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州电信主站J3".to_string(), host: "218.75.126.9".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州电信主站J4".to_string(), host: "115.238.90.165".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州联通主站J1".to_string(), host: "124.160.88.183".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州联通主站J2".to_string(), host: "60.12.136.250".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州华数主站J1".to_string(), host: "218.108.98.244".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "杭州华数主站J2".to_string(), host: "218.108.47.69".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "济南联通主站W1".to_string(), host: "27.221.115.131".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "青岛电信主站W1".to_string(), host: "58.56.180.60".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "深圳电信主站Z1".to_string(), host: "14.17.75.71".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "云行情上海电信Z1".to_string(), host: "114.80.63.12".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "云行情上海电信Z2".to_string(), host: "114.80.63.35".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "上海电信主站Z3".to_string(), host: "180.153.39.51".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "云行情北京联通Z1".to_string(), host: "123.125.108.23".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "云行情北京联通Z2".to_string(), host: "123.125.108.24".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "云行情广州电信Z1".to_string(), host: "121.201.83.106".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "中信证券".to_string(), desc: "云行情成都电信Z1".to_string(), host: "218.6.170.55".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(南京电信一) placeholder".to_string(), host: "180.101.48.170".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(南京电信二) placeholder".to_string(), host: "180.101.48.171".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(南京移动一) placeholder".to_string(), host: "120.195.71.155".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(南京移动二) placeholder".to_string(), host: "120.195.71.156".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(南京联通一) placeholder".to_string(), host: "122.96.107.242".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(南京联通二) placeholder".to_string(), host: "122.96.107.243".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(亚马逊一) placeholder".to_string(), host: "52.83.39.241".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(亚马逊二) placeholder".to_string(), host: "52.83.199.101".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(华南阿里云一) placeholder".to_string(), host: "8.135.57.58".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(华南阿里云二) placeholder".to_string(), host: "8.135.62.177".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(华东华为云一) placeholder".to_string(), host: "124.70.183.173".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "通达信".to_string(), desc: "华泰证券(华东华为云二) placeholder".to_string(), host: "124.71.163.106".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "郑州网通行情一".to_string(), host: "182.118.47.141".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "郑州网通行情二".to_string(), host: "182.118.47.168".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "郑州网通行情三".to_string(), host: "182.118.47.169".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "武汉电信行情一".to_string(), host: "119.97.164.184".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "武汉电信行情二".to_string(), host: "119.97.164.189".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "武汉电信行情三".to_string(), host: "116.211.121.102".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "武汉电信行情四".to_string(), host: "116.211.121.108".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "武汉电信行情五".to_string(), host: "116.211.121.31".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "新疆电信云行情一".to_string(), host: "202.100.166.117".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "新疆电信云行情二".to_string(), host: "202.100.166.118".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情八".to_string(), host: "222.73.139.166".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情九".to_string(), host: "222.73.139.167".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情十".to_string(), host: "222.73.139.168".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海BGP行情一".to_string(), host: "103.251.85.90".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "北京联通行情一".to_string(), host: "123.125.108.213".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "北京联通行情二".to_string(), host: "123.125.108.214".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情六".to_string(), host: "222.73.139.151".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情七".to_string(), host: "222.73.139.152".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "成都BGP行情一".to_string(), host: "148.70.110.41".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "成都BGP行情二".to_string(), host: "148.70.93.117".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "成都BGP行情三".to_string(), host: "148.70.31.16".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "成都BGP行情四".to_string(), host: "148.70.111.63".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情一".to_string(), host: "139.159.143.228".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情二".to_string(), host: "139.159.183.76".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情三".to_string(), host: "139.159.193.118".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情四".to_string(), host: "139.159.195.177".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情五".to_string(), host: "139.159.202.253".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情六".to_string(), host: "139.159.214.78".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情七".to_string(), host: "139.9.38.206".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情八".to_string(), host: "139.9.43.104".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情九".to_string(), host: "139.9.43.31".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情十".to_string(), host: "139.9.50.246".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情十一".to_string(), host: "139.9.52.158".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "广州BGP行情十二".to_string(), host: "139.9.90.169".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情十一".to_string(), host: "101.226.180.73".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海电信行情十二".to_string(), host: "101.226.180.74".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海BGP行情六".to_string(), host: "103.251.85.200".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海BGP行情七".to_string(), host: "103.251.85.201".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情一".to_string(), host: "103.221.142.65".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情二".to_string(), host: "103.221.142.66".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情三".to_string(), host: "103.221.142.67".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情四".to_string(), host: "103.221.142.68".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情五".to_string(), host: "103.221.142.69".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情六".to_string(), host: "103.221.142.70".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情七".to_string(), host: "103.221.142.71".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "南京电信行情八".to_string(), host: "103.221.142.72".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情一".to_string(), host: "117.34.114.13".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情二".to_string(), host: "117.34.114.14".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情三".to_string(), host: "117.34.114.15".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情四".to_string(), host: "117.34.114.16".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情五".to_string(), host: "117.34.114.17".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情六".to_string(), host: "117.34.114.18".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情七".to_string(), host: "117.34.114.20".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情八".to_string(), host: "117.34.114.27".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "西安电信行情九".to_string(), host: "117.34.114.30".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "上海BGP行情八".to_string(), host: "103.251.85.202".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情一".to_string(), host: "183.60.224.142".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情二".to_string(), host: "183.60.224.143".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情三".to_string(), host: "183.60.224.144".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情四".to_string(), host: "183.60.224.145".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情五".to_string(), host: "183.60.224.146".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情六".to_string(), host: "183.60.224.147".to_string(), port: 7709, latency_ms: 0 },
        ServerInfo { name: "国泰君安".to_string(), desc: "东莞电信行情七".to_string(), host: "183.60.224.148".to_string(), port: 7709, latency_ms: 0 },
    ]
}

/// Probe the standard servers concurrently and return the best ones with latency < elapsed_time.
///
/// This is a simplified port of the C++ `detect()` function. It connects using blocking
/// TCP connect with a timeout and measures round-trip connect latency. It does not perform
/// the level1 protocol handshake — it's a fast network probe to choose responsive endpoints.
pub fn detect(elapsed_time: i64, conn_limit: i32, connect_timeout_milliseconds: i32) -> Vec<ServerInfo> {
    let servers = standard_server_list();
    let n = servers.len();
    if n == 0 { return Vec::new(); }

    let num_threads = match std::thread::available_parallelism() {
        Ok(nthreads) => std::cmp::min(nthreads.get() as usize, n),
        Err(_) => std::cmp::min(4usize, n),
    };
    let servers_per_thread = (n + num_threads - 1) / num_threads;

    let mut handles = Vec::new();
    for i in 0..num_threads {
        let slice = servers.clone();
        let start = i * servers_per_thread;
        let end = std::cmp::min(start + servers_per_thread, n);
        let elapsed_time = elapsed_time;
        let timeout = Duration::from_millis(connect_timeout_milliseconds as u64);
        handles.push(thread::spawn(move || {
            let mut found: Vec<ServerInfo> = Vec::new();
            for j in start..end {
                if j >= slice.len() { break; }
                let s = &slice[j];
                let addr_str = s.addr();
                // Resolve and connect with timeout
                if let Ok(mut addrs) = (&addr_str[..]).to_socket_addrs() {
                    if let Some(sock) = addrs.find(|_| true) {
                        let start_time = Instant::now();
                        let res = TcpStream::connect_timeout(&sock, timeout);
                        if let Ok(stream) = res {
                            let duration = start_time.elapsed();
                            let mut si = s.clone();
                            si.latency_ms = duration.as_millis() as i64;
                            // only accept servers faster than elapsed_time
                            if si.latency_ms as i64 <= elapsed_time {
                                // close immediately
                                let _ = stream.shutdown(std::net::Shutdown::Both);
                                found.push(si);
                            }
                        }
                    }
                }
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

    // sort by latency and trim to conn_limit
    results.sort_by_key(|s| s.latency_ms);
    let limit = std::cmp::min(results.len(), conn_limit as usize);
    results.into_iter().take(limit).collect()
}
