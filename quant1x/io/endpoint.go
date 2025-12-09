package io

import (
	"errors"
	"fmt"
	stdnet "net"
	"strconv"
	"sync"

	"gitee.com/quant1x/quant1x/quant1x/logger"
)

// EndpointData 保存单个端点的连接统计
type EndpointData struct {
	MaxConnections    int
	ActiveConnections int
}

// EndpointManager 管理一组 TCP 端点（ip:port），并维护每个端点的最大连接数与当前活跃连接数。
//
// 实现说明：
// - 使用字符串形式 "host:port" 作为 map 的 key（因为 net.TCPAddr 包含切片，不能直接作为 map key）。
// - 所有访问共享状态的操作都使用 mutex 保护，保证线程安全。
type EndpointManager struct {
	mutex         sync.Mutex
	endpointsList []string // 按插入顺序保存的 endpoint 字符串 (host:port)
	endpointsData map[string]*EndpointData
}

// NewEndpointManager 创建一个新的 EndpointManager
func NewEndpointManager() *EndpointManager {
	return &EndpointManager{
		endpointsData: make(map[string]*EndpointData),
	}
}

// AddEndpoint 接受 ip 字符串与端口并验证，然后添加到管理器中
func (m *EndpointManager) AddEndpoint(ip string, port uint16, maxConnections int) bool {
	// 验证端口
	if port == 0 || port == 65535 {
		return false
	}

	// 验证IP
	if stdnet.ParseIP(ip) == nil {
		logger.Errorf("[endpoint] invalid ip: %s", ip)
		return false
	}

	addr := stdnet.JoinHostPort(ip, strconv.Itoa(int(port)))
	return m.addEndpointByString(addr, maxConnections)
}

// AddEndpointAddr 直接使用 net.TCPAddr 添加端点
func (m *EndpointManager) AddEndpointAddr(addr *stdnet.TCPAddr, maxConnections int) bool {
	if addr == nil {
		return false
	}
	key := addr.String()
	return m.addEndpointByString(key, maxConnections)
}

// addEndpointByString 为内部实现，key 为 "host:port" 格式
func (m *EndpointManager) addEndpointByString(key string, maxConnections int) bool {
	m.mutex.Lock()
	defer m.mutex.Unlock()

	if _, ok := m.endpointsData[key]; ok {
		return false
	}

	m.endpointsList = append(m.endpointsList, key)
	m.endpointsData[key] = &EndpointData{MaxConnections: maxConnections}
	return true
}

// RemoveEndpoint 移除指定端点（使用 net.TCPAddr）
func (m *EndpointManager) RemoveEndpoint(addr *stdnet.TCPAddr) {
	if addr == nil {
		return
	}
	key := addr.String()
	m.removeEndpointByString(key)
}

// removeEndpointByString 内部移除函数
func (m *EndpointManager) removeEndpointByString(key string) {
	m.mutex.Lock()
	defer m.mutex.Unlock()

	delete(m.endpointsData, key)

	// 从 slice 中删除
	for i, v := range m.endpointsList {
		if v == key {
			m.endpointsList = append(m.endpointsList[:i], m.endpointsList[i+1:]...)
			break
		}
	}
}

// AcquireEndpoint 返回第一个仍有可用连接配额的端点，并同时增加其活跃连接数
// 返回值为解析后的 *net.TCPAddr 与 ok 标志；若无可用端点则返回 (nil, false)
func (m *EndpointManager) AcquireEndpoint() (*stdnet.TCPAddr, bool) {
	m.mutex.Lock()
	defer m.mutex.Unlock()

	for _, key := range m.endpointsList {
		data := m.endpointsData[key]
		if data == nil {
			continue
		}
		if data.ActiveConnections < data.MaxConnections {
			data.ActiveConnections++
			// 解析地址为 net.TCPAddr，理论上解析不会失败，因为添加时已验证
			tcpAddr, err := stdnet.ResolveTCPAddr("tcp", key)
			if err != nil {
				// 解析失败时回退计数并继续
				data.ActiveConnections--
				logger.Errorf("[endpoint] resolve tcp addr failed: %v", err)
				continue
			}
			logger.Debugf("acquire endpoint: %s", key)
			return tcpAddr, true
		}
	}

	return nil, false
}

// ReleaseEndpoint 释放指定端点的活跃连接计数（若存在且大于0）
func (m *EndpointManager) ReleaseEndpoint(addr *stdnet.TCPAddr) {
	if addr == nil {
		return
	}
	key := addr.String()

	m.mutex.Lock()
	defer m.mutex.Unlock()
	if data, ok := m.endpointsData[key]; ok && data.ActiveConnections > 0 {
		data.ActiveConnections--
	}
	logger.Debugf("release endpoint: %s", key)
}

// GetEndpointStats 返回 (maxConnections, activeConnections, error)
func (m *EndpointManager) GetEndpointStats(addr *stdnet.TCPAddr) (int, int, error) {
	if addr == nil {
		return 0, 0, errors.New("nil addr")
	}
	key := addr.String()

	m.mutex.Lock()
	defer m.mutex.Unlock()

	if data, ok := m.endpointsData[key]; ok {
		return data.MaxConnections, data.ActiveConnections, nil
	}

	return 0, 0, fmt.Errorf("endpoint not found: %s", key)
}

// GetAllEndpoints 返回当前所有的端点（解析为 []*net.TCPAddr）
func (m *EndpointManager) GetAllEndpoints() ([]*stdnet.TCPAddr, error) {
	m.mutex.Lock()
	defer m.mutex.Unlock()

	out := make([]*stdnet.TCPAddr, 0, len(m.endpointsList))
	for _, key := range m.endpointsList {
		tcpAddr, err := stdnet.ResolveTCPAddr("tcp", key)
		if err != nil {
			return nil, err
		}
		out = append(out, tcpAddr)
	}
	return out, nil
}

// GetAvailableResources 计算所有端点剩余可用连接数之和
func (m *EndpointManager) GetAvailableResources() int {
	m.mutex.Lock()
	defer m.mutex.Unlock()

	available := 0
	for _, data := range m.endpointsData {
		if data.ActiveConnections < data.MaxConnections {
			available += data.MaxConnections - data.ActiveConnections
		}
	}
	return available
}
