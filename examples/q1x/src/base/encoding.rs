
#[cfg(test)]
mod tests {
    use encoding::{DecoderTrap, Encoding};
    use encoding::all::GBK;

    #[test]
    fn test_demo(){
        let code:Vec<u8>=vec![116, 109, 112, 47, 208, 194, 189, 168, 206, 196, 188, 254, 188, 208, 47];//这个是从zip文件中读的原始字节

        println!("{}",GBK.decode(&code, DecoderTrap::Strict).unwrap())//GBK解码
    }
}