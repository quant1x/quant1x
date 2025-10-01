mod database;
mod cache;

pub use database::DatabaseAdapter;
pub use cache::CacheAdapter;

#[ctor::ctor]
fn register_adapters() {
    use super::adapter::register_adapter;

    register_adapter::<DatabaseAdapter>();
    register_adapter::<CacheAdapter>();
}