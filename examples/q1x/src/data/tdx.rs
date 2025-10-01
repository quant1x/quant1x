mod raw {
    use std::any::type_name_of_val;
    use std::fmt::Formatter;

    /// 请求消息头
    #[allow(non_camel_case_types)]
    #[allow(dead_code)]
    struct request_header {
        /// Zip Flag
        zip_flag: u8,
        /// 请求编号
        seq_id: u32,
        /// 包类型
        packet_type: u8,
        /// 消息体长度1
        pkg_len1: u16,
        /// 消息体长度2
        pkg_len2: u16,
        /// method 请求方法
        method: u16,
    }

    /// 响应消息头
    #[derive(Debug)]
    #[allow(non_camel_case_types)]
    pub(crate) struct response_header {
        /// 未知字段1
        unknown_field1: u32,
        /// zip flag
        zip_flag: u8,
        /// 请求编号
        seq_id: u32,
        /// 未知字段2
        unknown_field2: u8,
        /// method
        method: u16,
        /// 压缩长度
        zip_size: u16,
        /// 未压缩长度
        unzip_size: u16,
    }

    impl std::fmt::Display for response_header {
        fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
            let mut buf :Vec<String>= vec![];
            buf.push(format!("unknown_field1({})={}", type_name_of_val(&self.unknown_field1), self.unknown_field1));
            buf.push(format!("zip_flag({})={}", type_name_of_val(&self.zip_flag), self.zip_flag));
            buf.push(format!("seq_id({})={}",type_name_of_val(&self.seq_id), self.seq_id));
            buf.push(format!("unknown_field2({})={}",type_name_of_val(&self.unknown_field2), self.unknown_field2));
            buf.push(format!("method({})={}",type_name_of_val(&self.method), self.method));
            buf.push(format!("zip_size({})={}",type_name_of_val(&self.zip_size), self.zip_size));
            buf.push(format!("unzip_size({})={}", type_name_of_val(&self.unzip_size), self.unzip_size));
            write!(f, "response_header[{}]", buf.join(", "))
        }
    }

    #[allow(dead_code)]
    impl response_header {
        /// 解码
        pub(crate) fn v1_unpack(data: &[u8]) -> response_header {
            let mut bytes = [0; 4];
            let tmp = unsafe { data.get_unchecked(0..4) };
            bytes.copy_from_slice(tmp);
            let unknown_field1 = u32::from_le_bytes(bytes);
            response_header {
                unknown_field1,
                zip_flag: 0,
                seq_id: 0,
                unknown_field2: 0,
                method: 0,
                zip_size: 0,
                unzip_size: 0,
            }
        }

        /// 解码
        pub(crate) fn unpack(data: &mut &[u8]) -> response_header {
            let unknown_field1 = util::read_le_u32(data);
            let zip_flag = util::read_le_u8(data);
            let seq_id = util::read_le_u32(data);
            let unknown_field2 =util::read_le_u8(data);
            let method = util::read_le_u16(data);
            let zip_size = util::read_le_u16(data);
            let unzip_size = util::read_le_u16(data);
            response_header {
                unknown_field1: unknown_field1,
                zip_flag: zip_flag,
                seq_id: seq_id,
                unknown_field2: unknown_field2,
                method: method,
                zip_size: zip_size,
                unzip_size: unzip_size,
            }
        }
    }

    /// 股票K线
    #[allow(non_camel_case_types)]
    #[allow(dead_code)]
    struct kline_stock {
        /// 日期时间
        pub datetime: String,
        /// 证券代码
        pub code: String,
        /// 开盘价
        pub open: f64,
        /// 收盘价
        pub close: f64,
        /// 最高价
        pub high: f64,
        /// 最低价
        pub low: f64,
        /// 成交量
        pub volume: f64,
        /// 成交金额
        pub amount: f64,
    }

    /// 指数类K线
    #[allow(non_camel_case_types)]
    #[allow(dead_code)]
    struct kline_index {
        /// 日期时间
        pub datetime: String,
        /// 证券代码
        pub code: String,
        /// 开盘价
        pub open: f64,
        /// 收盘价
        pub close: f64,
        /// 最高价
        pub high: f64,
        /// 最低价
        pub low: f64,
        /// 成交量
        pub volume: f64,
        /// 成交金额
        pub amount: f64,
        /// 上涨家数
        pub up: i32,
        /// 下跌家数
        pub down: i32,
    }

    /// 杂项
    pub mod util {
        /// 解析u8
        pub fn read_le_u8(input: &mut &[u8]) -> u8 {
            let (int_bytes, rest) = input.split_at(std::mem::size_of::<u8>());
            *input = rest;
            u8::from_le_bytes(int_bytes.try_into().unwrap())
        }

        pub fn read_le_u16(input: &mut &[u8]) -> u16 {
            let (int_bytes, rest) = input.split_at(std::mem::size_of::<u16>());
            *input = rest;
            u16::from_le_bytes(int_bytes.try_into().unwrap())
        }

        pub fn read_le_u32(input: &mut &[u8]) -> u32 {
            let (int_bytes, rest) = input.split_at(std::mem::size_of::<u32>());
            *input = rest;
            u32::from_le_bytes(int_bytes.try_into().unwrap())
        }

        // /// 不能工作
        // pub fn read_le<T:std::marker::Sized>(input: &mut &[u8]) -> T {
        //     let (int_bytes, rest) = input. split_at(std::mem::size_of::<T>());
        //     *input = rest;
        //     let value = T::from_le_bytes(int_bytes. try_into().unwrap());
        //     value
        // }
    }
}

#[cfg(test)]
mod tests_tdx {
    use std::slice;

    use super::*;

    #[test]
    fn test_decode_header() {
        let text = "b1cb74001c01000000000d006100bd00";
        let hexobj = hex::decode(text);
        let vec = hexobj.unwrap();
        let ptr: *const u8 = vec.as_ptr();
        let len: usize = vec.len();
        let mut data: &[u8] = unsafe { slice::from_raw_parts(ptr, len) };
        let rh = raw::response_header::unpack(&mut data);
        println!("{}", rh);
    }

    #[test]
    fn test_decode_header_v2() -> Result<(), hex::FromHexError> {
        let text = "b1Cb74001c01000000000d006100bd00";
        let hexobj = hex::decode(text)?;
        let mut data :&[u8] = &hexobj;
        let rh = raw::response_header::unpack(&mut data);
        println!("{}", rh);
        Ok(())
    }
}