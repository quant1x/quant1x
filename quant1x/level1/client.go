package level1

import (
	"errors"
	"io"
	"math"
	stdnet "net"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/logging"
	qnet "gitee.com/quant1x/quant1x/quant1x/net"
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
	poolInstance *qnet.TcpConnectionPool
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
	Servers []serverInfo `yaml:"servers"`
}

// StandardProtocolHandler 与 C++ 版本保持一致，实现握手与心跳逻辑。
type StandardProtocolHandler struct {
	timeout       time.Duration
	checkInterval time.Duration
}

// NewStandardProtocolHandler 构造协议处理器。
func NewStandardProtocolHandler(timeout, interval time.Duration) qnet.NetworkOperationHandler {
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

func (h *StandardProtocolHandler) processRequest(conn *stdnet.TCPConn, req []byte) ([]byte, *ResponseHeader, error) {
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
	if _, err := io.ReadFull(conn, body); err != nil {
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

func (h *StandardProtocolHandler) Handshake(conn *stdnet.TCPConn) (bool, error) {
	req1 := Hello1Request{}
	body1, _, err := h.processRequest(conn, req1.Bytes())
	if err != nil {
		logging.Errorf("level1 handshake Hello1 failed: %v", err)
		return false, err
	}
	if len(body1) == 0 {
		return false, errors.New("level1 handshake Hello1 empty body")
	}
	var resp1 Hello1Response
	if err := resp1.Deserialize(body1); err != nil {
		logging.Errorf("level1 handshake Hello1 validation failed: %v", err)
		return false, err
	}

	req2 := Hello2Request{}
	body2, _, err := h.processRequest(conn, req2.Bytes())
	if err != nil {
		logging.Errorf("level1 handshake Hello2 failed: %v", err)
		return false, err
	}
	if len(body2) == 0 {
		return false, errors.New("level1 handshake Hello2 empty body")
	}
	var resp2 Hello2Response
	if err := resp2.Deserialize(body2); err != nil {
		logging.Errorf("level1 handshake Hello2 validation failed: %v", err)
		return false, err
	}
	return true, nil
}

func (h *StandardProtocolHandler) Keepalive(conn *stdnet.TCPConn) (bool, error) {
	body, _, err := h.processRequest(conn, HeartbeatRequest{}.Bytes())
	if err != nil {
		return false, err
	}
	if len(body) == 0 {
		return false, errors.New("level1 keepalive empty body")
	}
	var resp HeartbeatResponse
	if err := resp.Deserialize(body); err != nil {
		logging.Errorf("level1 keepalive response invalid: %v", err)
		return false, err
	}
	return true, nil
}

// Client acquires a pooled Level1 connection with a release callback.
func Client() (*qnet.Connection, func(), error) {
	pool, err := getConnectionPool()
	if err != nil {
		return nil, nil, err
	}
	return pool.Acquire()
}

func getConnectionPool() (*qnet.TcpConnectionPool, error) {
	poolOnce.Do(func() {
		poolInstance, poolErr = initConnectionPool()
	})
	return poolInstance, poolErr
}

func initConnectionPool() (*qnet.TcpConnectionPool, error) {
	handler := NewStandardProtocolHandler(0, 0).(*StandardProtocolHandler)
	cachePath, err := ensureServerCachePath()
	if err != nil {
		return nil, err
	}

	servers, info, err := loadCachedServers(cachePath)
	needDetect := false
	if err != nil {
		logging.Debugf("level1: cached server list missing or invalid: %v", err)
		needDetect = true
	} else if shouldRefreshCache(info) {
		needDetect = true
	}

	if needDetect {
		logging.Infof("level1: refreshing server cache via detection")
		detected := detectServers(handler, latencyThreshold, maxConnections, defaultConnectTimeout)
		if len(detected) > 0 {
			servers = detected
			if err := saveCachedServers(cachePath, servers); err != nil {
				logging.Errorf("level1: failed to save detected servers: %v", err)
			}
		} else if len(servers) == 0 {
			logging.Infof("level1: detection returned no servers, falling back to standard list")
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
	pool, err := qnet.NewTcpConnectionPool(1, poolSize, handler)
	if err != nil {
		return nil, err
	}

	for i, srv := range servers {
		if i >= poolSize {
			break
		}
		if added := pool.AddEndpoint(srv.Host, int(srv.Port), 0); !added {
			logging.Debugf("level1: endpoint %s:%d already registered", srv.Host, srv.Port)
		}
	}

	return pool, nil
}

func detectServers(handler *StandardProtocolHandler, threshold time.Duration, limit int, connectTimeout time.Duration) []serverInfo {
	candidates := standardServerList()
	if len(candidates) == 0 {
		return nil
	}

	workerLimit := runtime.GOMAXPROCS(0)
	if workerLimit <= 0 {
		workerLimit = 4
	}
	if workerLimit > len(candidates) {
		workerLimit = len(candidates)
	}

	sem := make(chan struct{}, workerLimit)
	var wg sync.WaitGroup
	var mu sync.Mutex
	results := make([]serverInfo, 0, limit)

	for _, srv := range candidates {
		srv := srv
		sem <- struct{}{}
		wg.Add(1)
		go func() {
			defer wg.Done()
			defer func() { <-sem }()

			addr := stdnet.JoinHostPort(srv.Host, strconv.Itoa(int(srv.Port)))
			conn, err := stdnet.DialTimeout("tcp", addr, connectTimeout)
			if err != nil {
				logging.Debugf("level1 detect: dial %s failed: %v", addr, err)
				return
			}
			tcpConn, ok := conn.(*stdnet.TCPConn)
			if !ok {
				_ = conn.Close()
				return
			}
			defer tcpConn.Close()

			_ = tcpConn.SetNoDelay(true)
			start := time.Now()
			if ok, err := handler.Handshake(tcpConn); err != nil || !ok {
				logging.Debugf("level1 detect: handshake %s failed: %v", addr, err)
				return
			}
			latency := time.Since(start)
			if latency >= threshold {
				logging.Debugf("level1 detect: latency %s = %v exceeds threshold", addr, latency)
				return
			}
			srv.LatencyMS = latency.Milliseconds()

			mu.Lock()
			results = append(results, srv)
			mu.Unlock()
		}()
	}

	wg.Wait()
	sort.Slice(results, func(i, j int) bool { return results[i].LatencyMS < results[j].LatencyMS })
	if len(results) > limit {
		results = results[:limit]
	}
	return results
}

func ensureServerCachePath() (string, error) {
	home := defaultHomePath()
	meta := filepath.Join(home, "meta")
	if err := os.MkdirAll(meta, 0o755); err != nil {
		return "", err
	}
	return filepath.Join(meta, serverCacheFileName), nil
}

func defaultHomePath() string {
	candidates := []string{
		strings.TrimSpace(os.Getenv("QUANT1X_HOME")),
		strings.TrimSpace(os.Getenv("QUANT1X_DATA_HOME")),
		"~/.q1x",
	}
	for _, c := range candidates {
		if c == "" {
			continue
		}
		expanded := expandHome(c)
		if expanded != "" {
			return filepath.Clean(expanded)
		}
	}
	if home, err := os.UserHomeDir(); err == nil {
		return filepath.Join(home, ".q1x")
	}
	return ".q1x"
}

func expandHome(path string) string {
	if path == "~" {
		if h, err := os.UserHomeDir(); err == nil {
			return h
		}
		return ""
	}
	if strings.HasPrefix(path, "~/") {
		if h, err := os.UserHomeDir(); err == nil {
			return filepath.Join(h, path[2:])
		}
		return ""
	}
	return path
}

func loadCachedServers(path string) ([]serverInfo, os.FileInfo, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, nil, err
	}
	info, err := os.Stat(path)
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
	wrapper := serverListWrapper{Servers: servers}
	data, err := yaml.Marshal(&wrapper)
	if err != nil {
		return err
	}
	return os.WriteFile(path, data, 0o644)
}

func decodeServerList(data []byte) ([]serverInfo, error) {
	var wrapper serverListWrapper
	if err := yaml.Unmarshal(data, &wrapper); err == nil && len(wrapper.Servers) > 0 {
		return wrapper.Servers, nil
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

func standardServerList() []serverInfo {
	return []serverInfo{
		{Source: "通达信", Name: "深圳双线主站1", Host: "110.41.147.114", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站2", Host: "110.41.2.72", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站3", Host: "110.41.4.4", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站4", Host: "47.113.94.204", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站5", Host: "8.129.174.169", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站6", Host: "110.41.154.219", Port: 7709},
		{Source: "通达信", Name: "上海双线主站1", Host: "124.70.176.52", Port: 7709},
		{Source: "通达信", Name: "上海双线主站2", Host: "47.100.236.28", Port: 7709},
		{Source: "通达信", Name: "上海双线主站3", Host: "123.60.186.45", Port: 7709},
		{Source: "通达信", Name: "上海双线主站4", Host: "123.60.164.122", Port: 7709},
		{Source: "通达信", Name: "上海双线主站5", Host: "47.116.105.28", Port: 7709},
		{Source: "通达信", Name: "上海双线主站6", Host: "124.70.199.56", Port: 7709},
		{Source: "通达信", Name: "北京双线主站1", Host: "121.36.54.217", Port: 7709},
		{Source: "通达信", Name: "北京双线主站2", Host: "121.36.81.195", Port: 7709},
		{Source: "通达信", Name: "北京双线主站3", Host: "123.249.15.60", Port: 7709},
		{Source: "通达信", Name: "广州双线主站1", Host: "124.71.85.110", Port: 7709},
		{Source: "通达信", Name: "广州双线主站2", Host: "139.9.51.18", Port: 7709},
		{Source: "通达信", Name: "广州双线主站3", Host: "139.159.239.163", Port: 7709},
		{Source: "通达信", Name: "上海双线主站7", Host: "106.14.201.131", Port: 7709},
		{Source: "通达信", Name: "上海双线主站8", Host: "106.14.190.242", Port: 7709},
		{Source: "通达信", Name: "上海双线主站9", Host: "121.36.225.169", Port: 7709},
		{Source: "通达信", Name: "上海双线主站10", Host: "123.60.70.228", Port: 7709},
		{Source: "通达信", Name: "上海双线主站11", Host: "123.60.73.44", Port: 7709},
		{Source: "通达信", Name: "上海双线主站12", Host: "124.70.133.119", Port: 7709},
		{Source: "通达信", Name: "上海双线主站13", Host: "124.71.187.72", Port: 7709},
		{Source: "通达信", Name: "上海双线主站14", Host: "124.71.187.122", Port: 7709},
		{Source: "通达信", Name: "武汉电信主站1", Host: "119.97.185.59", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站7", Host: "47.107.64.168", Port: 7709},
		{Source: "通达信", Name: "北京双线主站4", Host: "124.70.75.113", Port: 7709},
		{Source: "通达信", Name: "广州双线主站4", Host: "124.71.9.153", Port: 7709},
		{Source: "通达信", Name: "上海双线主站15", Host: "123.60.84.66", Port: 7709},
		{Source: "通达信", Name: "深圳双线主站8", Host: "47.107.228.47", Port: 7719},
		{Source: "通达信", Name: "北京双线主站5", Host: "120.46.186.223", Port: 7709},
		{Source: "通达信", Name: "北京双线主站6", Host: "124.70.22.210", Port: 7709},
		{Source: "通达信", Name: "北京双线主站7", Host: "139.9.133.247", Port: 7709},
		{Source: "通达信", Name: "广州双线主站5", Host: "116.205.163.254", Port: 7709},
		{Source: "通达信", Name: "广州双线主站6", Host: "116.205.171.132", Port: 7709},
		{Source: "通达信", Name: "广州双线主站7", Host: "116.205.183.150", Port: 7709},
		{Source: "中信证券", Name: "上海电信主站Z1", Host: "180.153.18.170", Port: 7709},
		{Source: "中信证券", Name: "上海电信主站Z2", Host: "180.153.18.171", Port: 7709},
		{Source: "中信证券", Name: "北京联通主站Z1", Host: "202.108.253.130", Port: 7709},
		{Source: "中信证券", Name: "北京联通主站Z2", Host: "202.108.253.131", Port: 7709},
		{Source: "中信证券", Name: "杭州电信主站J1", Host: "60.191.117.167", Port: 7709},
		{Source: "中信证券", Name: "杭州电信主站J2", Host: "115.238.56.198", Port: 7709},
		{Source: "中信证券", Name: "杭州电信主站J3", Host: "218.75.126.9", Port: 7709},
		{Source: "中信证券", Name: "杭州电信主站J4", Host: "115.238.90.165", Port: 7709},
		{Source: "中信证券", Name: "杭州联通主站J1", Host: "124.160.88.183", Port: 7709},
		{Source: "中信证券", Name: "杭州联通主站J2", Host: "60.12.136.250", Port: 7709},
		{Source: "中信证券", Name: "杭州华数主站J1", Host: "218.108.98.244", Port: 7709},
		{Source: "中信证券", Name: "杭州华数主站J2", Host: "218.108.47.69", Port: 7709},
		{Source: "中信证券", Name: "济南联通主站W1", Host: "27.221.115.131", Port: 7709},
		{Source: "中信证券", Name: "青岛电信主站W1", Host: "58.56.180.60", Port: 7709},
		{Source: "中信证券", Name: "深圳电信主站Z1", Host: "14.17.75.71", Port: 7709},
		{Source: "中信证券", Name: "云行情上海电信Z1", Host: "114.80.63.12", Port: 7709},
		{Source: "中信证券", Name: "云行情上海电信Z2", Host: "114.80.63.35", Port: 7709},
		{Source: "中信证券", Name: "上海电信主站Z3", Host: "180.153.39.51", Port: 7709},
		{Source: "中信证券", Name: "云行情北京联通Z1", Host: "123.125.108.23", Port: 7709},
		{Source: "中信证券", Name: "云行情北京联通Z2", Host: "123.125.108.24", Port: 7709},
		{Source: "中信证券", Name: "云行情广州电信Z1", Host: "121.201.83.106", Port: 7709},
		{Source: "中信证券", Name: "云行情成都电信Z1", Host: "218.6.170.55", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(南京电信一)", Host: "180.101.48.170", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(南京电信二)", Host: "180.101.48.171", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(南京移动一)", Host: "120.195.71.155", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(南京移动二)", Host: "120.195.71.156", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(南京联通一)", Host: "122.96.107.242", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(南京联通二)", Host: "122.96.107.243", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(亚马逊一)", Host: "52.83.39.241", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(亚马逊二)", Host: "52.83.199.101", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(华南阿里云一)", Host: "8.135.57.58", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(华南阿里云二)", Host: "8.135.62.177", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(华东华为云一)", Host: "124.70.183.173", Port: 7709},
		{Source: "华泰证券", Name: "华泰证券(华东华为云二)", Host: "124.71.163.106", Port: 7709},
		{Source: "国泰君安", Name: "郑州网通行情一", Host: "182.118.47.141", Port: 7709},
		{Source: "国泰君安", Name: "郑州网通行情二", Host: "182.118.47.168", Port: 7709},
		{Source: "国泰君安", Name: "郑州网通行情三", Host: "182.118.47.169", Port: 7709},
		{Source: "国泰君安", Name: "武汉电信行情一", Host: "119.97.164.184", Port: 7709},
		{Source: "国泰君安", Name: "武汉电信行情二", Host: "119.97.164.189", Port: 7709},
		{Source: "国泰君安", Name: "武汉电信行情三", Host: "116.211.121.102", Port: 7709},
		{Source: "国泰君安", Name: "武汉电信行情四", Host: "116.211.121.108", Port: 7709},
		{Source: "国泰君安", Name: "武汉电信行情五", Host: "116.211.121.31", Port: 7709},
		{Source: "国泰君安", Name: "新疆电信云行情一", Host: "202.100.166.117", Port: 7709},
		{Source: "国泰君安", Name: "新疆电信云行情二", Host: "202.100.166.118", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情八", Host: "222.73.139.166", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情九", Host: "222.73.139.167", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情十", Host: "222.73.139.168", Port: 7709},
		{Source: "国泰君安", Name: "上海BGP行情一", Host: "103.251.85.90", Port: 7709},
		{Source: "国泰君安", Name: "北京联通行情一", Host: "123.125.108.213", Port: 7709},
		{Source: "国泰君安", Name: "北京联通行情二", Host: "123.125.108.214", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情六", Host: "222.73.139.151", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情七", Host: "222.73.139.152", Port: 7709},
		{Source: "国泰君安", Name: "成都BGP行情一", Host: "148.70.110.41", Port: 7709},
		{Source: "国泰君安", Name: "成都BGP行情二", Host: "148.70.93.117", Port: 7709},
		{Source: "国泰君安", Name: "成都BGP行情三", Host: "148.70.31.16", Port: 7709},
		{Source: "国泰君安", Name: "成都BGP行情四", Host: "148.70.111.63", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情一", Host: "139.159.143.228", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情二", Host: "139.159.183.76", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情三", Host: "139.159.193.118", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情四", Host: "139.159.195.177", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情五", Host: "139.159.202.253", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情六", Host: "139.159.214.78", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情七", Host: "139.9.38.206", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情八", Host: "139.9.43.104", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情九", Host: "139.9.43.31", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情十", Host: "139.9.50.246", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情十一", Host: "139.9.52.158", Port: 7709},
		{Source: "国泰君安", Name: "广州BGP行情十二", Host: "139.9.90.169", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情十一", Host: "101.226.180.73", Port: 7709},
		{Source: "国泰君安", Name: "上海电信行情十二", Host: "101.226.180.74", Port: 7709},
		{Source: "国泰君安", Name: "上海BGP行情六", Host: "103.251.85.200", Port: 7709},
		{Source: "国泰君安", Name: "上海BGP行情七", Host: "103.251.85.201", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情一", Host: "103.221.142.65", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情二", Host: "103.221.142.66", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情三", Host: "103.221.142.67", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情四", Host: "103.221.142.68", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情五", Host: "103.221.142.69", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情六", Host: "103.221.142.70", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情七", Host: "103.221.142.71", Port: 7709},
		{Source: "国泰君安", Name: "南京电信行情八", Host: "103.221.142.72", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情一", Host: "117.34.114.13", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情二", Host: "117.34.114.14", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情三", Host: "117.34.114.15", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情四", Host: "117.34.114.16", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情五", Host: "117.34.114.17", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情六", Host: "117.34.114.18", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情七", Host: "117.34.114.20", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情八", Host: "117.34.114.27", Port: 7709},
		{Source: "国泰君安", Name: "西安电信行情九", Host: "117.34.114.30", Port: 7709},
		{Source: "国泰君安", Name: "上海BGP行情八", Host: "103.251.85.202", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情一", Host: "183.60.224.142", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情二", Host: "183.60.224.143", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情三", Host: "183.60.224.144", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情四", Host: "183.60.224.145", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情五", Host: "183.60.224.146", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情六", Host: "183.60.224.147", Port: 7709},
		{Source: "国泰君安", Name: "东莞电信行情七", Host: "183.60.224.148", Port: 7709},
	}
}
