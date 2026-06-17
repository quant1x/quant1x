use std::sync::Once;
use crate::base::config::QUANT1X_CACHE_CONFIG;
use crate::data::exchange::last_trade_date;
use crate::exchange::symbol;
use anyhow::Result;
use csv::StringRecord;
use std::collections::HashMap;
use serde::Deserialize;
use std::path::PathBuf;
use std::sync::OnceLock;
use crate::base::runtime;

// 数据结构定义
#[derive(Debug, Deserialize)]
#[derive(Clone)]
pub struct Security {
    #[serde(alias = "Code", alias = "code", alias = "CODE")]
    code: String,
    #[serde(alias = "Name", alias = "name", alias = "NAME")]
    name: String,
}


//const CRON_INIT_SPEC: &str = "0 0 9  * * *";
const CRON_INIT_SPEC: &str = "*/5 * * * * *";

static INIT: Once = Once::new();
/// 缓存证券代码和证券名称的映射
#[allow(static_mut_refs)] // 抑制警告
#[allow(mutable_transmutes)]
static DATA_SECURITY_MAP: OnceLock<HashMap<String, String>> = OnceLock::new();

fn get_security_map() -> &'static HashMap<String, String> {
    DATA_SECURITY_MAP.get_or_init(|| {
        let mut map = HashMap::new();
        if let Ok(list) = lazy_load_security_list() {
            for v in list.iter() {
                map.insert(v.code.clone(), v.name.clone());
            }
        }
        map
    })
}

/// 带缓存的证券列表读取(移除参数)
/// 修改缓存宏并调整错误处理
fn lazy_load_security_list() -> anyhow::Result<Vec<Security>> {
    let path = QUANT1X_CACHE_CONFIG.meta_path("securities.csv");
    let mut reader = csv::Reader::from_path(path)?;

    reader.deserialize()
        .map(|record: Result<Security, csv::Error>| {
            record.map(|mut s| {
                s.code = s.code.to_lowercase();
                s
            })
        })
        .collect::<Result<Vec<_>, _>>()
        .map_err(Into::into)
}


// 股票名称查询
pub fn get_stock_name(code: &str) -> String {
    let corrected = symbol::correct_security_code(code);
    let map = get_security_map();
    let name = map.get(&corrected);
    name.unwrap().clone()
}

/// 数据缓存子目录名去掉完整的证券代码后三位
const CACHE_CODE_SUFFIX: u8 = 3;

/// 根据证券代码构造目录结构
fn cache_sub_path(code :&str) -> (String, String) {
    let corrected = symbol::correct_security_code(code);
    let length = corrected.len();
    let subpath = corrected[..length-usize::from(CACHE_CODE_SUFFIX)].to_string();
    (corrected, subpath)
}

// fn cache_day_path(code :&str) -> String {
//     let corrected = correct_security_code(code);
//     let length = corrected.len();
//     corrected[..length-usize::from(CACHE_CODE_SUFFIX)].to_string()
// }

// fn cache_day_filepath(code :&str) -> String {
//     let corrected = symbol::correct_security_code(code);
//     let length = corrected.len();
//     let subpath = corrected[..length-usize::from(CACHE_CODE_SUFFIX)].to_string();
//
//     let dir = QUANT1X_CACHE_CONFIG.day_path(subpath.as_str());
//     let path = dir.join(format!("{}.csv", corrected));
//     path.display().to_string()
// }

/// 日线数据读取
pub fn klines(code: &str) -> Result<Vec<StringRecord>> {
    let (security_code,sub_path) = cache_sub_path(code);
    let filepath = QUANT1X_CACHE_CONFIG.day_path(sub_path.as_str());
    let path = filepath.join(format!("{}.csv", security_code));
    println!("path: {:?}", path);
    csv::Reader::from_path(path)?
        .records()
        .map(|result| result.map_err(|e| anyhow::anyhow!(e))) // 添加错误转换层
        .collect()
}

/// 板块记录
#[derive(Debug, Deserialize,Clone)]
pub struct SectorRecord {
    /// 名称
    #[serde(rename = "name")]
    pub name: String,

    /// 代码
    #[serde(alias = "Code", alias = "code", alias = "CODE")]
    pub code: String,

    /// 类型
    #[serde(rename = "type")]
    pub type_: i32,

    /// 成分股数量
    #[serde(rename = "count")]
    pub count: i32,

    /// 通达信板块编码
    #[serde(rename = "block")]
    pub block: String,

    /// 板块成分股
    #[serde(rename = "ConstituentStocks")]
    constituent_stocks__: String,
    #[serde(skip)]
    pub stocks: Vec<String>,
}

/// 缓存板块列表
static DATA_SECTORS: OnceLock<Vec<SectorRecord>> = OnceLock::new();

// 板块文件名生成
// 修改函数签名
fn __get_sector_filename(date: Option<&str>) -> PathBuf {
    // 将 Option<&str> 转换为 Option<String>
    let date_str = date
        .map(|s| s.to_string())
        .unwrap_or_else(|| last_trade_date());
    QUANT1X_CACHE_CONFIG.meta_path(format!("blocks.{}", date_str).as_str())
}

// 板块列表读取
fn __lazy_load_sector_list() -> Result<Vec<SectorRecord>> {
    let path = __get_sector_filename(None);
    let mut reader = csv::Reader::from_path(&path)?;
    reader.deserialize()
        .map(|result| {
            // 1. 先处理反序列化错误, 转换为 anyhow::Error
            let mut sr:SectorRecord = result.map_err(|e| anyhow::anyhow!("证券代码列表缓存文件{}解析失败: {}",path.display(),e))?;
            // 2. 修改 code 字段(确保 correct_security_code 返回 String)
            sr.code = symbol::correct_security_code(sr.code.as_str());
            {
                let cs = serde_json::from_str::<Vec<String>>(&sr.constituent_stocks__)
                    .map(|list| {
                        list.into_iter()
                            .map(|s| symbol::correct_security_code(&s))
                            .collect()
                    }).unwrap();
                //println!("cs={:?}", cs);
                sr.stocks = cs;
            }
            // 3. 返回修改后的成功值
            Ok(sr)
        })
        .collect()
}

/// 加载板块列表
fn get_sector_list() -> &'static Vec<SectorRecord> {
    DATA_SECTORS.get_or_init(|| {
        let mut list = Vec::new();
        let result = __lazy_load_sector_list();
        match result {
            Ok(l ) => {
                for v in l.iter() {
                    list.push(v.clone());
                }
                list
            }
            Err(_) => {
                list
            }
        }
    })
}

/// 获取板块代码列表
/// 从证券列表中获取, sh880和sh881开头的是板块
pub fn get_sector_code_list() -> Vec<String> {
    let map = get_security_map();
    let filter_keys : Vec<_> = map.iter()
        .filter(|&(&ref k,_)|(k.starts_with("sh880") || k.starts_with("sh881")))
        .map(|(key,_)|key.clone())
        .collect();
    filter_keys
}


// 获取指定板块的成分股
// pub fn get_sector_constituents(code: &str) -> Result<Vec<String>> {
//     let code = correct_security_code(code);
//     let sectors = get_sector_list();
//     println!("{}", sectors.len());
//     let sector = sectors.iter()
//         .find(|s| { s.code == code })
//         .ok_or_else(|| anyhow::anyhow!("Sector {} not found", code))?;
//
//     serde_json::from_str::<Vec<String>>(&sector.constituent_stocks)
//         .map(|list| {
//             list.into_iter()
//                 .map(|s| correct_security_code(&s))
//                 .collect()
//         })
//         .map_err(|e| anyhow::anyhow!("Failed to parse constituent stocks: {}", e))
// }

// pub fn v1get_sector_constituents(code: &str) -> Result<Vec<String>> {
//     let code = correct_security_code(code);
//     let sectors = get_sector_list();
//     println!("{}", sectors.len());
//     let sector = sectors.iter()
//         .find(|s| { s.code == code })
//         .ok_or_else(|| anyhow::anyhow!("Sector {} not found", code))?;
//     println!("{:?}", sector);
//     Ok(sector.stocks.clone())
// }
//
// pub fn v2get_sector_constituents(code: &str) ->Vec<String> {
//     let code = correct_security_code(code);
//     let sectors = get_sector_list();
//     let mut list = Vec::new();
//     let sector = sectors.iter()
//         .find(|s| { s.code == code })
//         .ok_or_else(|| anyhow::anyhow!("Sector {} not found", code));
//     match sector {
//         Ok(sr) => {
//             println!("{:?}", sr.stocks);
//             list.extend(sr.stocks.clone())
//         }
//         Err(_) => {}
//     }
//     list
// }

pub fn get_sector_constituents(code: &str) ->Vec<String> {
    let code = symbol::correct_security_code(code);
    let sectors = get_sector_list();
    let sector = sectors.iter()
        .find(|s| { s.code == code })
        .ok_or_else(|| anyhow::anyhow!("Sector {} not found", code));
    match sector {
        Ok(sr) => {
            sr.stocks.clone()
        }
        Err(_) => {
            Vec::new()
        }
    }
}



#[cfg(test)]
mod tests {
    use super::*;
    use polars::prelude::*;

    #[test]
    fn test_sector_filename() {
        let filename = __get_sector_filename(None);
        println!("{:?}", filename.display());
    }

    #[test]
    fn test_sector_list() {
        let list = __lazy_load_sector_list();
        println!("{:?}", list);
    }

    #[test]
    fn test_sector_find() {
        // 获取板块成分股
        let constituents = get_sector_constituents("sh881428");
        println!("Constituents: {:?}", constituents);
    }

    #[test]
    fn test_security_list() -> Result<()> {
        // 获取证券列表
        let sec = get_security_map();
        println!("Total securities: {}", sec.len());

        // 查询股票名称
        let name = get_stock_name("600600");
        println!("Stock name: {}", name);

        // 获取板块成分股
        let constituents = get_sector_constituents("881428");
        println!("Constituents: {:?}", constituents);

        Ok(())
    }

    // #[test]
    // fn test_klines() -> Result<(), Box<dyn std::error::Error>> {
    //     //let rows = klines("600600");
    //     //println!("Klines: {:?}", rows);
    //     let kp = cache_day_filepath("600600");
    //     println!("kp: {:?}", kp);
    //     let df = CsvReadOptions::default()
    //         .with_has_header(true)
    //         .try_into_reader_with_file_path(Some(kp.into()))?
    //         .finish()?;
    //     println!("CSV 数据预览:\n{}", df);
    //     //let closes=df.column("close")?;
    //     // 提取并转换目标列
    //     //let age_series = df.column("close")?.cast(&DataType::Float64)?;
    //     let binding = df.column("high")?.cast(&DataType::Float64)?;
    //     let h = binding.f64()?;
    //     let binding = df.column("low")?.cast(&DataType::Float64)?;
    //     let l = binding.f64()?;
    //     let x = (h+l)/2;
    //     println!("x: {:?}", x);
    //     let n = x.len();
    //     println!("h = {:?}", h.get(n-1));
    //     println!("l = {:?}", l.get(n-1));
    //     println!("x=(h+l)/2= {:?}", x.get(n-1));
    //     Ok(())
    // }
}
