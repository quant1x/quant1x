use std::any::Any;
use std::fmt::Debug;

// DefaultValue 默认值接口
pub trait DefaultValue {
    fn default_value(&self) -> Box<dyn Any>;
}

// ApplyDefaults 给结构体零值字段填充默认值. 
//
// 在Rust中, 由于serde和Default trait, 我们使用Default::default(). 
// 这里提供一个兼容的函数. 
pub fn apply_defaults<T: Default>(target: &mut T) {
    // Since T implements Default, and serde handles defaults, this is a no-op.
    // In Go, it uses reflection to set tagged defaults.
    // In Rust, we rely on #[derive(Default)] and #[serde(default)].
    let _ = target;
}

// For compatibility, a function to apply defaults to a struct.
pub fn apply_defaults_to_struct<T: Default>(target: &mut T) {
    *target = T::default();
}
