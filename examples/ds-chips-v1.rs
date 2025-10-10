use csv::ReaderBuilder;
use indicatif::{MultiProgress, ProgressBar, ProgressStyle};
use q1x::data;
use q1x::exchange::symbol;
use std::fs::File;
use std::io::{self, BufReader, Read};

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

fn main() {
    let lastday = data::exchange::last_trade_date();
    println!("last day: {:?}", lastday);
    let begin_date = "2025-03-01";
    let end_date = lastday.as_str();
    let dates = data::exchange::trade_date_range(begin_date, end_date);
    println!("dates: {:?}", dates);
    let count = dates.len();
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
        .template("{msg} {spinner:.green} [{elapsed_precise}] [{bar:40.cyan/blue}] {pos:>7}/{len:7} @ {per_sec}")
        .unwrap()
        .progress_chars("#>-"));
    for date in dates.iter() {
        // 成交记录的日期格式是YYYYMMDD, 去掉-间隔符
        let td = date.replace("-", "");
        pb_main.set_message(format!("{}", td));
        //println!("{}", td);
        let filepath = data::datasets::get_trans_filepath(sc.as_str(), td.as_str());
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
            .template("{spinner:.green} [{elapsed_precise}] [{wide_bar:.cyan/blue}] {bytes} @ {bytes_per_sec}, {eta}")
            .unwrap()
            .progress_chars("#>-"));
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
        let mut trans: Vec<data::datasets::HistoricalTrade> = Vec::new();
        // 逐行处理 CSV
        for row in csv_reader.deserialize() {
            let record: data::datasets::HistoricalTrade = match row {
                Ok(record) => record,
                Err(_) => continue,
            };
            trans.push(record);
        }
        // 完成进度条
        pb.finish_with_message("处理完成");
        // match reader {
        //     Ok(ref mut r) => {
        //         let trans : Vec<data::datasets::HistoricalTrade> = r.deserialize()
        //             .filter_map(|row|{
        //                match row {
        //                    Ok(record) => {
        //                        Some(record)
        //                    },
        //                    Err(_) => { None}
        //                }
        //             })
        //             .collect();
        //         //println!("trans: {} > {:?}", date,trans.len())
        //     }
        //     Err(e) => {
        //         //println!("{} = {:?}",date, e);
        //         continue
        //     }
        // };
        // drop(reader);
        //exit(0);
        //sleep(std::time::Duration::from_secs(2));
        pb_main.inc(1);
    }
}
