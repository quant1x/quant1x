#![feature(portable_simd)]
#![feature(slice_range)]

pub mod data;
pub mod base;
pub mod exchange;

pub fn add(left: u64, right: u64) -> u64 {
    left + right
}



#[cfg(test)]
mod tests {
    use crate::base::time;
    use crate::exchange::calendar::{lastday, lastday_by};
    use super::*;

    #[test]
    fn it_works() {
        let mut lastday = lastday();
        println!("{}", lastday);
        lastday = lastday_by("2024-07-13".to_string());
        println!("{}", lastday);
        //load().expect("TODO: panic message");
        //static MANAGER: patterns::SingletonManager<i32> = patterns::SingletonManager::new();
        let result = add(2, 2);
        assert_eq!(result, 4);
    }

    #[test]
    fn test_timestamp() {
        let ts = time::now();
        println!("{}", ts)
    }
}
