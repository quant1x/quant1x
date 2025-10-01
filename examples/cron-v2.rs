use q1x::base::runtime;
//#[tokio::main]
fn main() {
    runtime::register_shutdown_hook(|| { {
        println!("test close function...");
    }});
    runtime::add_task("*/5 * * * * *", || {
        println!("test open function-5s...");
    });

    let x = q1x::data::cache::get_stock_name("600600");
    println!("x: {}", x);
    runtime::wait_for_exit();
}