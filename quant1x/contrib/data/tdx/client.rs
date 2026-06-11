// -*- coding: utf-8 -*-
// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// TDX client — 连接管理

use std::net::SocketAddr;
use std::str::FromStr;
use std::sync::{Arc, OnceLock};

use crate::io::TcpConnectionPool;

use super::protocol::{ExtensionProtocolHandler, StandardProtocolHandler};

// ============================================================
// get_std_conn — 获取标准行情连接
// ============================================================

static STD_POOL: OnceLock<Arc<TcpConnectionPool<StandardProtocolHandler>>> = OnceLock::new();

fn build_std_pool() -> Arc<TcpConnectionPool<StandardProtocolHandler>> {
    let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());

    let mut servers: Vec<super::config::ServerInfo> = Vec::new();
    if let Some(cached) = Some(super::config::read_cache("standard")) {
        if !cached.is_empty() {
            log::debug!("[tdx/client] loaded {} cached std servers", cached.len());
            servers = cached;
        }
    }

    if servers.is_empty() {
        log::debug!("[tdx/client] no cached std servers, running detect()");
        let detected_map = super::config::detect(
            super::config::MAX_ELAPSED_TIME_MS,
            super::config::MAX_CONNECTIONS,
            super::config::DEFAULT_CONNECT_TIMEOUT_MS,
        );
        let detected = detected_map.get("standard").cloned().unwrap_or_default();
        log::debug!("[tdx/client] detect() returned {} std servers", detected.len());
        if !detected.is_empty() {
            let mut cache_map = std::collections::BTreeMap::new();
            cache_map.insert("standard".to_string(), detected.clone());
            super::config::write_cache(&cache_map);
        }
        servers = detected;
    } else {
        log::debug!("[tdx/client] using cached std servers for pool seeding");
    }

    if servers.is_empty() {
        log::warn!("[tdx/client] std detection produced no servers, falling back to standard list");
        servers = super::config::standard_server_list();
    }

    for s in &servers {
        match SocketAddr::from_str(&s.addr()) {
            Ok(addr) => {
                let _ = endpoint_manager.add_endpoint(addr, 1);
            }
            Err(e) => log::warn!("[tdx/client] invalid std server addr {}: {}", s.addr(), e),
        }
    }

    let server_count = servers.len();
    if server_count == 0 {
        log::error!("[tdx/client] no std servers available");
    }

    let max_conn = std::cmp::min(
        super::config::MAX_CONNECTIONS,
        server_count.max(1),
    );

    log::debug!(
        "[tdx/client] building std pool with max_connections={}, endpoints={}",
        max_conn,
        server_count
    );

    let handler = Arc::new(StandardProtocolHandler {});
    TcpConnectionPool::new(1, max_conn, handler, endpoint_manager)
}

pub fn get_std_conn() -> std::io::Result<crate::io::PooledConnection<StandardProtocolHandler>> {
    let pool = STD_POOL.get_or_init(|| build_std_pool());
    pool.acquire()
}

/// 获取标准行情连接池的最大连接数
pub fn pool_max_connections() -> Option<usize> {
    STD_POOL.get().map(|p| p.max_connections())
}

// ============================================================
// get_ext_conn — 获取扩展行情连接
// ============================================================

static EXT_POOL: OnceLock<Arc<TcpConnectionPool<ExtensionProtocolHandler>>> = OnceLock::new();

fn build_ext_pool() -> Arc<TcpConnectionPool<ExtensionProtocolHandler>> {
    let endpoint_manager = Arc::new(crate::io::endpoint::EndpointManager::new());

    let mut discovered: Vec<SocketAddr> = Vec::new();
    let cached = super::config::read_cache("extension");
    if !cached.is_empty() {
        for s in &cached {
            match SocketAddr::from_str(&s.addr()) {
                Ok(addr) => discovered.push(addr),
                Err(e) => log::warn!("[tdx/client] invalid ext server addr {}: {}", s.addr(), e),
            }
        }
    }

    if discovered.is_empty() {
        let ext_servers = super::config::extension_server_list();
        for s in &ext_servers {
            match SocketAddr::from_str(&s.addr()) {
                Ok(addr) => discovered.push(addr),
                Err(e) => log::warn!("[tdx/client] invalid ext server addr {}: {}", s.addr(), e),
            }
        }
    }

    if discovered.is_empty() {
        log::error!("[tdx/client] no extension servers available");
    }

    let max_conn = std::cmp::min(
        super::config::MAX_CONNECTIONS,
        std::cmp::max(1, discovered.len()),
    );

    for addr in &discovered {
        let _ = endpoint_manager.add_endpoint(*addr, 1);
    }

    log::debug!(
        "[tdx/client] building ext pool with max_connections={}, endpoints={}",
        max_conn,
        discovered.len()
    );

    let handler = Arc::new(ExtensionProtocolHandler);
    TcpConnectionPool::new(1, max_conn, handler, endpoint_manager)
}

pub fn get_ext_conn() -> std::io::Result<crate::io::PooledConnection<ExtensionProtocolHandler>> {
    let pool = EXT_POOL.get_or_init(|| build_ext_pool());
    pool.acquire()
}
