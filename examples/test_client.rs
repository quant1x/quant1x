fn main() {
    println!("Starting level1 client test...");
    // Acquire a pooled client using the public re-export
    match quant1x::get_std_conn() {
        Ok(mut pooled) => {
            println!("Acquired pooled connection.");
            // Construct a heartbeat request and send via process_request to test the round-trip
            let mut req = quant1x::HeartbeatRequest::new();
            let mut resp = quant1x::HeartbeatResponse::new();
            match quant1x::process(pooled.stream(), &mut req, &mut resp) {
                Ok(_) => println!("process returned info: {}", resp.info),
                Err(e) => eprintln!("process failed: {}", e),
            }
        }
        Err(e) => {
            eprintln!("Failed to acquire client: {}", e);
        }
    }
}
