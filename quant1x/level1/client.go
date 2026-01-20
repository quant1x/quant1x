package level1

import (
	"errors"
	stdio "io"
	"math"
	"net"
	"os"
	"path/filepath"
	"sync"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/core"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	qio "gitee.com/quant1x/quant1x/quant1x/io"
	"gitee.com/quant1x/quant1x/quant1x/logger"
	"gitee.com/quant1x/quant1x/quant1x/std"
	"gopkg.in/yaml.v3"
)

const (
	maxConnections        = 10
	latencyThreshold      = 100 * time.Millisecond
	defaultConnectTimeout = time.Second
	serverCacheFileName   = "server.bin"
	cacheRefreshInterval  = 6 * time.Hour
)

var (
	poolOnce     sync.Once
	poolInstance *qio.TcpConnectionPool
	poolErr      error
)

type serverInfo struct {
	Source    string `yaml:"source" json:"source"`
	Name      string `yaml:"name" json:"name"`
	Host      string `yaml:"host" json:"host"`
	Port      uint16 `yaml:"port" json:"port"`
	LatencyMS int64  `yaml:"latency_ms" json:"latency_ms"`
}

type serverListWrapper struct {
	Standard  []serverInfo `yaml:"standard"`  // 标准服务器列表
	Extension []serverInfo `yaml:"extension"` // 扩展服务器列表
}

// StandardProtocolHandler 与 C++ 版本保持一致，实现握手与心跳逻辑。
type StandardProtocolHandler struct {
	timeout       time.Duration
	checkInterval time.Duration
}

// NewStandardProtocolHandler 构造协议处理器。
func NewStandardProtocolHandler(timeout, interval time.Duration) qio.NetworkOperationHandler {
	if timeout <= 0 {
		timeout = 10 * time.Second
	}
	if interval <= 0 {
		interval = 5 * time.Second
	}
	return &StandardProtocolHandler{timeout: timeout, checkInterval: interval}
}

func (h *StandardProtocolHandler) Timeout() time.Duration       { return h.timeout }
func (h *StandardProtocolHandler) CheckInterval() time.Duration { return h.checkInterval }

func (h *StandardProtocolHandler) processRequest(conn *net.TCPConn, req []byte) ([]byte, *ResponseHeader, error) {
	if conn == nil {
		return nil, nil, errors.New("nil conn")
	}
	if err := conn.SetDeadline(time.Now().Add(h.timeout)); err != nil {
		return nil, nil, err
	}
	defer conn.SetDeadline(time.Time{})
	if _, err := conn.Write(req); err != nil {
		return nil, nil, err
	}
	hdr, err := readResponseHeader(conn)
	if err != nil {
		return nil, nil, err
	}
	if hdr.ZipSize == 0 {
		return nil, hdr, nil
	}
	body := make([]byte, hdr.ZipSize)
	if _, err := stdio.ReadFull(conn, body); err != nil {
		return nil, hdr, err
	}
	if hdr.ZipSize != hdr.UnZipSize {
		un, err := unzipZlib(body)
		if err != nil {
			return nil, hdr, err
		}
		return un, hdr, nil
	}
	return body, hdr, nil
}

func (h *StandardProtocolHandler) Handshake(conn *net.TCPConn) (bool, error) {
	req1 := Hello1Request{}
	body1, _, err := h.processRequest(conn, req1.Bytes())
	if err != nil {
		logger.Errorf("level1 handshake Hello1 failed: %v", err)
		return false, err
	}
	if len(body1) == 0 {
		return false, errors.New("level1 handshake Hello1 empty body")
	}
	var resp1 Hello1Response
	if err := resp1.Deserialize(body1); err != nil {
		logger.Errorf("level1 handshake Hello1 validation failed: %v", err)
		return false, err
	}

	req2 := Hello2Request{}
	body2, _, err := h.processRequest(conn, req2.Bytes())
	if err != nil {
		logger.Errorf("level1 handshake Hello2 failed: %v", err)
		return false, err
	}
	if len(body2) == 0 {
		return false, errors.New("level1 handshake Hello2 empty body")
	}
	var resp2 Hello2Response
	if err := resp2.Deserialize(body2); err != nil {
		logger.Errorf("level1 handshake Hello2 validation failed: %v", err)
		return false, err
	}
	return true, nil
}

func (h *StandardProtocolHandler) Keepalive(conn *net.TCPConn) (bool, error) {
	body, _, err := h.processRequest(conn, HeartbeatRequest{}.Bytes())
	if err != nil {
		return false, err
	}
	if len(body) == 0 {
		return false, errors.New("level1 keepalive empty body")
	}
	var resp HeartbeatResponse
	if err := resp.Deserialize(body); err != nil {
		logger.Errorf("level1 keepalive response invalid: %v", err)
		return false, err
	}
	return true, nil
}

// GetStdConnection 获取标准连接池中的连接，返回连接对象、关闭函数和可能的错误
func GetStdConnection() (*qio.Connection, func(), error) {
	pool, err := getStandardConnectionPool()
	if err != nil {
		return nil, nil, err
	}
	return pool.Acquire()
}

func getStandardConnectionPool() (*qio.TcpConnectionPool, error) {
	poolOnce.Do(func() {
		poolInstance, poolErr = initStandardConnectionPool()
	})
	return poolInstance, poolErr
}

func initStandardConnectionPool() (*qio.TcpConnectionPool, error) {
	handler := NewStandardProtocolHandler(0, 0).(*StandardProtocolHandler)
	cachePath, err := ensureServerCachePath()
	if err != nil {
		return nil, err
	}

	servers, info, err := loadCachedServers(cachePath)
	needDetect := false
	if err != nil {
		logger.Debugf("level1: cached server list missing or invalid: %v", err)
		needDetect = true
	} else if shouldRefreshCache(info) {
		needDetect = true
	} else if tp := exchange.NewTimestampFromTime(info.ModTime()); err == nil && exchange.CanInitialize(&tp) {
		needDetect = true
	}

	if needDetect {
		logger.Infof("level1: refreshing server cache via detection")
		detected := detectServers(handler, latencyThreshold, maxConnections, defaultConnectTimeout)
		if len(detected) > 0 {
			servers = detected
			if err := saveCachedServers(cachePath, servers); err != nil {
				logger.Errorf("level1: failed to save detected servers: %v", err)
			}
		} else if len(servers) == 0 {
			logger.Infof("level1: detection returned no servers, falling back to standard list")
			servers = standardServerList()
		}
	}

	if len(servers) == 0 {
		servers = standardServerList()
	}

	if len(servers) == 0 {
		return nil, errors.New("level1: no servers available for pool")
	}

	poolSize := int(math.Min(float64(len(servers)), float64(maxConnections)))
	pool, err := qio.NewTcpConnectionPool(1, poolSize, handler)
	if err != nil {
		return nil, err
	}

	for i, srv := range servers {
		if i >= poolSize {
			break
		}
		if added := pool.AddEndpoint(srv.Host, int(srv.Port), 0); !added {
			logger.Debugf("level1: endpoint %s:%d already registered", srv.Host, srv.Port)
		}
	}

	return pool, nil
}

// ensureServerCachePath 确保服务器缓存路径存在，并返回缓存文件路径
// 返回:
//
//	string - 服务器缓存文件完整路径
//	error - 如果创建目录失败则返回错误
func ensureServerCachePath() (string, error) {
	home := core.GetMetaPath()
	meta := filepath.Join(home, "meta")
	if err := std.MkDirs(meta, true); err != nil {
		return "", err
	}
	return filepath.Join(meta, serverCacheFileName), nil
}

func loadCachedServers(path string) ([]serverInfo, os.FileInfo, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, nil, err
	}
	info, err := os.Lstat(path)
	if err != nil {
		return nil, nil, err
	}
	servers, err := decodeServerList(data)
	if err != nil {
		return nil, nil, err
	}
	if len(servers) == 0 {
		return nil, nil, errors.New("empty server list")
	}
	return servers, info, nil
}

func saveCachedServers(path string, servers []serverInfo) error {
	wrapper := serverListWrapper{Standard: servers}
	f, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, std.CACHE_FILE_PERMS)
	if err != nil {
		return err
	}
	defer f.Close()

	encoder := yaml.NewEncoder(f)
	encoder.SetIndent(2)
	defer encoder.Close()

	return encoder.Encode(&wrapper)
}

func decodeServerList(data []byte) ([]serverInfo, error) {
	var wrapper serverListWrapper
	if err := yaml.Unmarshal(data, &wrapper); err == nil && len(wrapper.Standard) > 0 {
		return wrapper.Standard, nil
	}
	var servers []serverInfo
	if err := yaml.Unmarshal(data, &servers); err == nil && len(servers) > 0 {
		return servers, nil
	}
	return nil, errors.New("unsupported cache format")
}

func shouldRefreshCache(info os.FileInfo) bool {
	if info == nil {
		return true
	}
	if info.Size() == 0 {
		return true
	}
	return time.Since(info.ModTime()) > cacheRefreshInterval
}
