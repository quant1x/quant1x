use std::convert::TryFrom;
use crate::base::config;
use crate::exchange::symbol;
use anyhow::Result;
use serde::{Deserialize, Serialize};
use serde_repr::{Deserialize_repr, Serialize_repr};

const CACHE_TRANS_PATH:&str= "trans";  // 成交数据目录名

/// 目录结构${trans}/${YYYY}/${YYYYMMDD}/${SecurityCode}.csv
pub fn get_trans_filepath(code :&str, date : &str) -> String {
    assert_eq!(date.len(), 8);
    let security_code = symbol::correct_security_code(code);
    let year = &date[0..4];
    let trans_path = config::get_main_path(CACHE_TRANS_PATH);
    let path = trans_path.join(&year).join(date).join(format!("{}.csv",security_code));
    path.display().to_string()

}
/// 历史成交数据结构
#[derive(Debug, Serialize, Deserialize)]
pub struct HistoricalTrade {
    #[serde(rename = "time")]
    pub time: String,

    #[serde(rename = "price")]
    pub price: f64,

    #[serde(rename = "vol")]
    pub volume: i32,

    #[serde(rename = "num", default, skip_serializing_if = "Option::is_none")]
    pub num: Option<i32>,

    #[serde(rename = "amount")]
    pub amount: f64,

    #[serde(rename = "buyorsell")]
    pub direction: TradeDirection,
}

/// 交易类型枚举（对应Go中的TradeType）
///
/// 基础类型为i32，与Go的int32保持一致
#[derive(Debug, Clone, Copy, PartialEq, Serialize_repr, Deserialize_repr)]
#[repr(i32)]
pub enum TradeDirection {
    /// 买入 (对应Go的TDX_TICK_BUY)
    Buy = 0,

    /// 卖出 (对应Go的TDX_TICK_SELL)
    Sell = 1,

    /// 中性盘 (对应Go的TDX_TICK_NEUTRAL)
    Neutral = 2,

    /// 未知类型（出现在09:27历史数据中，暂时归类为中性盘）
    Unknown = 3,
}

impl TradeDirection {
    /// 从i32值安全转换到枚举类型
    ///
    /// # 参数
    /// * `value` - 输入的i32值
    ///
    /// # 返回
    /// * `Some(TradeDirection)` - 有效值时返回对应枚举
    /// * `None` - 无效值时返回None
    pub fn from_i32(value: i32) -> Option<Self> {
        match value {
            0 => Some(Self::Buy),
            1 => Some(Self::Sell),
            2 => Some(Self::Neutral),
            3 => Some(Self::Unknown),
            _ => None,
        }
    }
}

impl TryFrom<i32> for TradeDirection {
    type Error = &'static str;

    fn try_from(value: i32) -> Result<Self, Self::Error> {
        Self::from_i32(value).ok_or("Invalid TradeDirection value")
    }
}

impl From<TradeDirection> for i32 {
    fn from(t: TradeDirection) -> Self {
        t as i32
    }
}

// fn read_csv_to_struct(path: &str) -> Result<Vec<HistoricalTrade>, Box<dyn std::error::Error>> {
//     // 读取 CSV 文件
//     let df = CsvReadOptions::default()
//         .with_has_header(true)
//         .try_into_reader_with_file_path(Some(path.into()))?
//         .finish()?;
//     println!("CSV 数据预览:\n{}", df);
//
//     // 转换为行记录
//     let iter = df.iter().map(|row| {
//         let values: Vec<_> = row.iter().collect();
//
//         // 手动解析字段（根据实际类型调整）
//         HistoricalTrade {
//             time: values[0].extract::<&str>().unwrap_or("").to_string(),
//             price: values[1].extract::<f64>().unwrap(),
//             volume: values[2].extract::<i32>().unwrap(),
//             num: Option::from(values[3].extract::<i32>().unwrap()),
//             amount: values[4].extract::<f64>().unwrap(),
//             direction: values[5].extract::<TradeDirection>().unwrap(),
//         }
//     });
//
//     Ok(iter.collect())
// }



#[cfg(test)]
mod tests {
    use super::*;
    use crate::data::data;
    use crate::data::data::ChipDistribution;
    use bytes::Buf;
    use bytes::BytesMut;
    use itertools::Itertools;
    use polars::prelude::*;
    use prost::Message;
    use std::io::Read;
    use std::io::BufReader;
    use tokio::io::AsyncReadExt;
    use memmap2::Mmap;
    use polars::prelude::SerReader;
    use polars::prelude::*;
    use std::fs::File;

    /// 使用内存映射加速 CSV 读取
    fn read_csv_with_mmap(path: &str) -> Result<DataFrame> {
        // 创建内存映射
        let file = File::open(path)?;
        let mmap = unsafe { Mmap::map(&file)? };

        // 转换为字节流
        let cursor = std::io::Cursor::new(mmap.as_ref());

        // 配置 Polars 读取器
        let df = CsvReader::new(cursor)
            .has_header(true)
            .with_try_parse_dates(true)  // 自动解析日期
            .infer_schema(Some(10000)) // 优化推断性能
            .with_chunk_size(1_000_000)  // 分块处理
            .finish()?;
        Ok(df)
    }
    const PB_TEST_BIN:&str = "data-chips.bin";
    #[test]
    fn test_hist_trans_filename() {
        let filename = get_trans_filepath("600600", "20250321");
        println!("filename: {:?}", filename);
        let result = std::fs::metadata(&filename);
        println!("result: {:?}", result);
    }

    #[test]
    fn test_hist_trans_read_to_csv() -> Result<(), Box<dyn std::error::Error>> {
        let filename = get_trans_filepath("600600", "20250321");
        // 读取 10GB 级股票数据
        let df = read_csv_with_mmap(filename.as_ref())?;
        // for i in 0..df.height(){
        //     let row = df.get_row(i)?;
        //     println!("Row {}: {:?}", i, row.get("time"));
        // }
        // let _ = df.iter().map(|x| {
        //     println!("x = {:#?}", x);
        // }).collect_vec();
        // // 分块并行处理
        // let chunks = df.chunks();
        // let df1 = DataFrame::default();
        // df.iter().chunks().for_each(|chunk| {})
        // df.into_struct("row");
        //
        // // let chunks: Vec<DataFrame> = df.iter()
        // //     .split_ch(n_chunks=num_cpus::get())
        // //     .collect();
        // println!("{:?}", df);
        let mut result = df.lazy().group_by([col("price")])
            .agg([
                sum("vol"),
                sum("amount"),
            ])
            .sort(["price"], Default::default())
            .collect()?;
        println!("result: {:?}", result);
        let mut file = File::create("output.csv").expect("could not create file");
        CsvWriter::new(&mut file)
            .include_header(true)
            .with_separator(b',')
            .finish(&mut result)?;
        Ok(())
    }

    // #[test]
    // fn test_hist_trans_to_bin() -> Result<(), Box<dyn std::error::Error>> {
    //     let date = "20250321";
    //     let filename = get_trans_filepath("600600", date);
    //     // 读取 10GB 级股票数据
    //     let df = read_csv_with_mmap(filename.as_ref())?;
    //     let mut result = df.lazy().group_by([col("price")])
    //         .agg([
    //             sum("vol"),
    //             sum("amount"),
    //         ])
    //         .sort(["price"], Default::default())
    //         .collect()?;
    //     println!("result: {:?}", result);
    //     // 读取二进制文件
    //     let mut file = File::create(PB_TEST_BIN)?;
    //     let mut writer = BufWriter::new(file);
    //     let mut buffer = Vec::new();
    //
    //     // 获取列的引用（按类型）
    //     let price_series = result.column("price")?.f64()?;
    //     let vol_series = result.column("vol")?.i64()?;
    //     let amount_series = result.column("amount")?.f64()?;
    //     let mut map :HashMap<i32, f64> = HashMap::new();
    //     for i in 0..result.height() {
    //         let price = price_series.get(i).unwrap();
    //         let vol = vol_series.get(i).unwrap();
    //         let amount = amount_series.get(i).unwrap();
    //         map.insert((price*100f64) as i32, vol as f64);
    //     }
    //     let mut chips = data::Chips::default();
    //     chips.date = date.parse().unwrap();
    //     chips.dist = map;
    //     println!("chips: {:?}", chips);
    //     let mut cd = data::ChipDistribution::default();
    //     cd.list.push(chips);
    //     cd.encode_length_delimited(&mut buffer);
    //     println!("encoded length: {:?}", buffer.len());
    //     //println!("原始数据字节: {:02X?}", buffer.as_ref());
    //     writer.write_all(&buffer)?;
    //     writer.flush()?;
    //     Ok(())
    // }

    #[test]
    fn test_hist_trans_from_bin_v1() -> Result<(), Box<dyn std::error::Error>> {
        let buffer = std::fs::read(PB_TEST_BIN)?;
        println!("文件内容字节: {:02X?}", buffer); // 关键诊断输出

        // 尝试解码
        let distribution = ChipDistribution::decode_length_delimited(&buffer[..])?;
        println!("解码结果: {:#?}", distribution);
        Ok(())
    }

    #[test]
    fn test_hist_trans_from_bin_v2() -> Result<(), Box<dyn std::error::Error>> {
        // 打开文件并创建缓冲读取器
        let mut reader = BufReader::new(File::open(PB_TEST_BIN)?);
        let mut buffer = BytesMut::new();
        let mut read_buf = [0u8; 7]; // 8KB 读取块

        let mut message_length:usize = 0;
        loop {
            // 1. 填充缓冲区
            match reader.read(&mut read_buf) {
                Ok(0) => {
                    println!("the end");
                    break
                },   // 文件结束
                Ok(n) => {
                    println!("read {} bytes", n);
                    buffer.extend_from_slice(&read_buf[..n])
                },
                Err(e) => {
                    println!("read error: {}", e);
                    return Err(e.into())
                },
            };
            if message_length > 0  && buffer.len() < message_length {
                continue;
            }
            // 2. 循环解码缓冲区中的完整消息
            loop {
                let original_len = buffer.len();
                println!("1-buffer-length: {:?}", buffer.len());
                if message_length == 0 {
                    // 尝试解码消息头（长度前缀）
                    match prost::decode_length_delimiter(&mut buffer) {
                        Ok(msg_len) => {
                            message_length = msg_len;
                        }
                        Err(e) => return Err(e.into()),
                    }
                }
                // 检查是否包含完整消息体
                println!("buffer.len(): {}, total_needed: {}", buffer.len(), message_length);
                if buffer.len() < message_length {
                    break; // 数据不足，继续读取
                }
                let ldl = prost::length_delimiter_len(message_length);
                println!("ldl: {:?}", ldl);
                // 解码完整消息
                let msg = data::ChipDistribution::decode(&buffer[..])?;
                println!("Decoded: {:?}", msg);

                // 移除已处理数据
                buffer.advance(message_length);
                message_length = 0;
                println!("2-buffer-length: {:?}", buffer.len());
                // 防止无限循环的二次检查
                if buffer.len() == 0 || original_len == buffer.len() {
                    break;
                }
            }
        }

        // 3. 检查剩余数据（可能包含不完整消息）
        if !buffer.is_empty() {
            eprintln!("Warning: {} bytes unprocessed", buffer.len());
        }

        Ok(())
    }

}