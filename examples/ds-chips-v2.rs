use csv::ReaderBuilder;
use indicatif::{MultiProgress, ProgressBar, ProgressStyle};
use quant1x::datasets as data;
use quant1x::exchange::code as symbol;
use quant1x::exchange;
use std::fs::File;
use std::io::{self, BufReader, Read};
use serde::{Deserialize, Serialize};
use serde_repr::{Deserialize_repr, Serialize_repr};

/// 自定义 Reader，跟踪已读字节数并更新进度条
struct ProgressReader<R> {
    inner: R,
    progress: ProgressBar,
}

impl<R: Read> Read for ProgressReader<R> {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let bytes_read = self.inner.read(buf)?;
        self.progress.inc(bytes_read as u64); // 更新进度条
        Ok(bytes_read)
    }
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
#[derive(Debug, Clone, Copy, PartialEq, Serialize_repr, Deserialize_repr)]
#[repr(i32)]
pub enum TradeDirection {
    Buy = 0,
    Sell = 1,
    Neutral = 2,
    Unknown = 3,
}

fn main() {
    let lastday = exchange::last_trading_day(quant1x::Timestamp::now()).only_date();
    //println!("last day: {:?}", lastday);
    let begin_date = "2025-03-01";
    let end_date = lastday.as_str();
    
    let begin_ts = quant1x::Timestamp::parse(begin_date).unwrap();
    let end_ts = quant1x::Timestamp::parse(end_date).unwrap();
    let dates_ts = exchange::date_range(begin_ts, end_ts, false);
    let dates: Vec<String> = dates_ts.iter().map(|ts| ts.only_date()).collect();

    //println!("dates: {:?}", dates);
    let count = dates.len();
    if count == 0 {
        println!("No trading dates found.");
        return;
    }
    println!(
        "修复 {}个交易日 筹码分布 {} => {}: ",
        count,
        dates[0],
        dates[count - 1]
    );
    let code = "600600";
    let sc = symbol::correct_security_code(code);
    // 初始化多进度条管理器
    let mp = MultiProgress::new();
    mp.set_draw_target(indicatif::ProgressDrawTarget::stderr());
    // 创建新的进度条
    let pb_main = mp.add(ProgressBar::new(count as u64));
    pb_main.set_style(ProgressStyle::default_bar()
        .template("{prefix} {msg}  {spinner:.green} [{elapsed_precise}] [{bar:60.cyan/blue}] {pos:>7}/{len:7} @ {per_sec}")
        .unwrap()
        .progress_chars("#>-"));
    pb_main.set_prefix("筹码分布");
    for date in dates.iter() {
        // 成交记录的日期格式是YYYYMMDD, 去掉-间隔符
        let td = date.replace("-", "");
        pb_main.set_message(format!("{}", date));
        //println!("{}", td);
        let filepath = data::trans::get_trans_filepath(sc.as_str(), td.as_str());
        // 打开文件并获取总大小
        let file = match File::open(filepath) {
            Ok(file) => file,
            Err(_) => continue,
        };
        let file_size = match file.metadata() {
            Ok(metadata) => metadata.len(),
            Err(_) => continue,
        };

        // 创建文件处理的进度条
        let pb = mp.add(ProgressBar::new(file_size));
        pb.set_style(ProgressStyle::default_bar()
            .template("{prefix} {msg} {spinner:.green} [{elapsed_precise}] [{bar:60.cyan/blue}] {bytes}/{total_bytes} @ {bytes_per_sec}, {eta}")
            .unwrap()
            .progress_chars("#>-"));
        pb.set_prefix(format!("{}", date));
        // 将文件包装为 ProgressReader
        let reader = BufReader::new(file); // 使用 BufReader 提升性能
        let progress_reader = ProgressReader {
            inner: reader,
            progress: pb.clone(),
        };
        // 创建 CSV 读取器
        let mut csv_reader = ReaderBuilder::new()
            .has_headers(true) // 根据 CSV 是否有标题调整
            .from_reader(progress_reader);
        let mut trans: Vec<HistoricalTrade> = Vec::new();
        //let mut num = 0;
        // 逐行处理 CSV
        for row in csv_reader.deserialize() {
            //num += 1;
            //pb.set_message(format!("{}", num));
            let record: HistoricalTrade = match row {
                Ok(record) => record,
                Err(_) => continue,
            };
            //sleep(std::time::Duration::from_millis(10));
            trans.push(record);
        }
        // 完成进度条
        pb.finish_with_message("done");
        //pb.finish_and_clear();
        
        pb_main.inc(1);
    }
    pb_main.finish_with_message("done");
    //pb_main.finish_and_clear();
}
