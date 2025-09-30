// ringbuffer module - minimal wrapper
// Only export the Vyukov bounded MPMC queue implementation.
// The actual implementation lives in `vyukov.rs`.
pub mod vyukov;
pub use vyukov::*;
