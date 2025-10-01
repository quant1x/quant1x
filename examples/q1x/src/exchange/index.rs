/// A股指数
pub static A_SHARE_INDEX_LIST: [&str; 12] = [
    "sh000001", // 上证综合指数
    "sh000002", // 上证A股指数
    "sh000300", // 沪深300指数
    "sh000688", // 科创50指数
    "sh000905", // 中证500指数
    "sz399001", // 深证成份指数
    "sz399006", // 创业板指
    "sz399107", // 深证A指
    "sh880005", // 通达信板块-涨跌家数
    "sh510050", // 上证50ETF
    "sh510300", // 沪深300ETF
    "sh510900", // H股ETF
];

pub fn index_list() -> &'static [&'static str] {
    &A_SHARE_INDEX_LIST
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_index_list() {
        let indices = index_list();

        // 验证列表长度
        assert_eq!(indices.len(), 12);

        // 验证关键指数存在
        assert!(indices.contains(&"sh000001"));
        assert!(indices.contains(&"sz399001"));
        assert!(indices.contains(&"sh510050"));

        // 验证顺序正确
        assert_eq!(indices[0], "sh000001");
        assert_eq!(indices[5], "sz399001");
        assert_eq!(indices[11], "sh510900");

        // 验证特殊代码
        assert!(indices.iter().any(|&s| s.starts_with("sh880")));
        assert!(indices.iter().any(|&s| s.starts_with("sz399")));
    }

    #[test]
    fn test_static_lifetime() {
        let indices = index_list();
        let first_index = indices[0];
        // 确保返回的引用具有 'static 生命周期
        let _: &'static str = first_index;
    }
}
