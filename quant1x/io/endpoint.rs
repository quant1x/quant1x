use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::Mutex;
use std::time::{Duration, Instant};

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct Endpoint {
    pub addr: SocketAddr,
}

impl From<SocketAddr> for Endpoint {
    fn from(a: SocketAddr) -> Self {
        Endpoint { addr: a }
    }
}

#[derive(Clone)]
struct EndpointData {
    max_connections: usize,
    active_connections: usize,
    // If set, this endpoint is considered temporarily failed/unavailable until this Instant
    failed_until: Option<Instant>,
}

/// 简单的线程安全端点管理器, 类似于 C++ 的 EndpointManager. 
pub struct EndpointManager {
    list: Mutex<Vec<Endpoint>>,
    data: Mutex<HashMap<Endpoint, EndpointData>>,
}

impl EndpointManager {
    pub fn new() -> Self {
        Self {
            list: Mutex::new(Vec::new()),
            data: Mutex::new(HashMap::new()),
        }
    }

    pub fn add_endpoint(&self, addr: SocketAddr, max_connections: usize) -> bool {
        if max_connections == 0 {
            return false;
        }
        let ep = Endpoint::from(addr);
        let mut list = self.list.lock().unwrap();
        let mut data = self.data.lock().unwrap();
        if data.contains_key(&ep) {
            return false;
        }
        list.push(ep.clone());
        data.insert(
            ep,
            EndpointData {
                max_connections,
                active_connections: 0,
                failed_until: None,
            },
        );
        true
    }

    pub fn remove_endpoint(&self, addr: SocketAddr) {
        let ep = Endpoint::from(addr);
        let mut list = self.list.lock().unwrap();
        let mut data = self.data.lock().unwrap();
        data.remove(&ep);
        list.retain(|e| e != &ep);
    }

    pub fn acquire_endpoint(&self) -> Option<SocketAddr> {
        let list = self.list.lock().unwrap();
        let mut data = self.data.lock().unwrap();
        let now = Instant::now();
        for ep in list.iter() {
            if let Some(d) = data.get_mut(ep) {
                // skip endpoints currently marked as failed
                if let Some(t) = d.failed_until {
                    if t > now {
                        continue;
                    } else {
                        // failure cooldown expired
                        d.failed_until = None;
                    }
                }
                if d.active_connections < d.max_connections {
                    d.active_connections += 1;
                    return Some(ep.addr);
                }
            }
        }
        None
    }

    pub fn release_endpoint(&self, addr: SocketAddr) {
        let ep = Endpoint::from(addr);
        let mut data = self.data.lock().unwrap();
        if let Some(d) = data.get_mut(&ep) {
            if d.active_connections > 0 {
                d.active_connections -= 1;
            }
        }
    }

    /// Mark an endpoint as failed/unavailable for the given cooldown duration.
    /// This prevents acquire_endpoint from selecting it until the cooldown expires.
    pub fn mark_failed(&self, addr: SocketAddr, cooldown: Duration) {
        let ep = Endpoint::from(addr);
        let mut data = self.data.lock().unwrap();
        if let Some(d) = data.get_mut(&ep) {
            d.failed_until = Some(Instant::now() + cooldown);
        }
    }

    pub fn get_endpoint_stats(&self, addr: SocketAddr) -> Option<(usize, usize)> {
        let ep = Endpoint::from(addr);
        let data = self.data.lock().unwrap();
        data.get(&ep)
            .map(|d| (d.max_connections, d.active_connections))
    }

    pub fn get_all_endpoints(&self) -> Vec<SocketAddr> {
        let list = self.list.lock().unwrap();
        list.iter().map(|e| e.addr).collect()
    }

    pub fn get_available_resources(&self) -> usize {
        let data = self.data.lock().unwrap();
        let now = Instant::now();
        data.iter()
            .map(|(_, d)| {
                if let Some(t) = d.failed_until {
                    if t > now {
                        return 0;
                    }
                }
                if d.active_connections < d.max_connections {
                    d.max_connections - d.active_connections
                } else {
                    0
                }
            })
            .sum()
    }
}
