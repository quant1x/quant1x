fn main() {
    println!("Starting level1 client test...");
    // Acquire a pooled client using the public re-export
    match quant1x::client() {
        Ok(mut pooled) => {
            println!("Acquired pooled connection.");
            // Construct a heartbeat request and send via process_request to test the round-trip
            let mut req = quant1x::HeartbeatRequest::new();
            let buf = req.serialize();
            match quant1x::process_request(pooled.stream(), &buf) {
                Ok(body) => println!("process_request returned {} bytes", body.len()),
                Err(e) => eprintln!("process_request failed: {}", e),
            }
        }
        Err(e) => {
            eprintln!("Failed to acquire client: {}", e);
        }
    }
}
