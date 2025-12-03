package http

import (
	"math"
	"net"
	stdNetHttp "net/http"
	"time"
)

const (
	// DefaultRedirects is the default number of times an Attacker follows
	// redirects.
	DefaultRedirects = 10
	// DefaultTimeout is the default amount of time an Attacker waits for a request
	// before it times out.
	DefaultTimeout = 30 * time.Second
	// DefaultConnections is the default amount of max open idle connections per
	// target host.
	DefaultConnections = 10000
	// DefaultMaxConnections is the default amount of connections per target
	// host.
	DefaultMaxConnections = 0
	// DefaultWorkers is the default initial number of workers used to carry an attack.
	DefaultWorkers = 10
	// DefaultMaxWorkers is the default maximum number of workers used to carry an attack.
	DefaultMaxWorkers = math.MaxUint64
	// DefaultMaxBody is the default max number of bytes to be read from response bodies.
	// Defaults to no limit.
	DefaultMaxBody = int64(-1)
	// NoFollow is the value when redirects are not followed but marked successful
	NoFollow = -1
)

// DefaultRoundTripper is used if no RoundTripper is set in Config.
var DefaultRoundTripper stdNetHttp.RoundTripper = &stdNetHttp.Transport{
	Proxy: stdNetHttp.ProxyFromEnvironment,
	DialContext: (&net.Dialer{
		Timeout:   30 * time.Second, // 限制建立TCP连接的时间
		KeepAlive: 30 * time.Second, // 保持连接的超时时间
	}).DialContext,
	IdleConnTimeout:       30 * time.Second,      // 空闲（keep-alive）连接在关闭之前保持空闲的时长
	TLSHandshakeTimeout:   10 * time.Second,      // 限制 TLS握手的时间
	ResponseHeaderTimeout: 10 * time.Second,      // 限制读取response header的时间,默认 timeout + 5*time.Second
	ExpectContinueTimeout: 1 * time.Second,       // 限制client在发送包含 Expect: 100-continue的header到收到继续发送body的response之间的时间等待。
	MaxIdleConns:          100,                   // 所有host的连接池最大连接数量，默认无穷大
	MaxIdleConnsPerHost:   DefaultConnections,    // 每个host的连接池最大空闲连接数,默认2
	MaxConnsPerHost:       DefaultMaxConnections, // 每个host的最大连接数量
	//ForceAttemptHTTP2:     true,
}

func defaultClient() *stdNetHttp.Client {
	return &stdNetHttp.Client{
		Transport: DefaultRoundTripper,
		Timeout:   DefaultTimeout, //设置超时时间
	}
}
