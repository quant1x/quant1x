use quant1x::runtime;
use tokio::signal;

#[tokio::main]
async fn main() {
    // runtime::register_shutdown_hook is not available in the current runtime
    // We can simulate the shutdown hook by handling the signal manually

    // runtime::add_task requires a name and is async
    let _ = runtime::add_task("test_task", "*/5 * * * * *", || {
        println!("test open function-5s...");
    })
    .await;

    let x = quant1x::instruments::get_security_info("600600")
        .map(|i| i.name)
        .unwrap_or_default();
    println!("x: {}", x);

    // runtime::wait_for_exit() is not available, use tokio signal handling
    match signal::ctrl_c().await {
        Ok(()) => {
            println!("test close function...");
        }
        Err(err) => {
            eprintln!("Unable to listen for shutdown signal: {}", err);
        }
    }
}
