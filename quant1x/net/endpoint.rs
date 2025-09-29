use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::Mutex;

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
}

/// Simple thread-safe endpoint manager, similar to the C++ EndpointManager.
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
        for ep in list.iter() {
            if let Some(d) = data.get_mut(ep) {
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
        data.iter()
            .map(|(_, d)| {
                if d.active_connections < d.max_connections {
                    d.max_connections - d.active_connections
                } else {
                    0
                }
            })
            .sum()
    }
}
