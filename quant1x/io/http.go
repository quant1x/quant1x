package io

import (
	"bytes"
	"compress/flate"
	"compress/gzip"
	"fmt"
	"io"
	"maps"
	"math"
	"net"
	stdNetHttp "net/http"
	stdNetUrl "net/url"
	"strings"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/log"
	"gitee.com/quant1x/quant1x/quant1x/std"
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

type Response struct {
	StatusCode    int
	ContentLength int
	LastModified  time.Time
	Body          []byte
}

const (
	MethodGet     = stdNetHttp.MethodGet
	MethodPost    = stdNetHttp.MethodPost
	MethodHead    = stdNetHttp.MethodHead
	MethodPut     = stdNetHttp.MethodPut
	MethodPatch   = stdNetHttp.MethodPatch // RFC 5789
	MethodDelete  = stdNetHttp.MethodDelete
	MethodConnect = stdNetHttp.MethodConnect
	MethodOptions = stdNetHttp.MethodOptions
	MethodTrace   = stdNetHttp.MethodTrace

	ContentEncoding = "Content-Encoding"
	ContextType     = "Content-Type"
	LastModified    = "Last-Modified"
	IfModifiedSince = "If-Modified-Since"
	charsetUtf8     = "charset=UTF-8"
	ApplicationJson = "application/json" + ";" + charsetUtf8
	ApplicationForm = "application/x-www-form-urlencoded" + ";" + charsetUtf8
)

var (
	TimeZero = time.Unix(0, 0)
	NotFound = std.NewException(stdNetHttp.StatusNotFound, "%s", stdNetHttp.StatusText(stdNetHttp.StatusNotFound))
)

var (
	defaultHeaders = map[string]string{
		"Accept":                    "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
		"Accept-Encoding":           "gzip, deflate",
		"Accept-Language":           "zh-CN,zh;q=0.9,en;q=0.8",
		"Cache-Control":             "no-cache",
		"Connection":                "keep-alive",
		"Pragma":                    "no-cache",
		"Upgrade-Insecure-Requests": "1",
		"User-Agent":                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36 Edg/113.0.1774.35",
	}
)

// HttpRequest HTTP 请求
func HttpRequest(url string, method string, header ...map[string]any) ([]byte, error) {
	data, lastModified, err := Request(url, method, "", header...)
	_ = lastModified
	return data, err
}

// Get HTTP协议GET请求
func Get(url string, header ...map[string]any) ([]byte, error) {
	data, _, err := Request(url, MethodGet, "", header...)
	return data, err
}

// Post HTTP协议POST请求
func Post(url string, content string, header ...map[string]any) (data []byte, err error) {
	var requestHeader map[string]any
	if len(header) == 0 {
		requestHeader = make(map[string]any, 0)
	} else {
		requestHeader = header[0]
	}
	requestHeader[ContextType] = ApplicationForm
	content = strings.TrimSpace(content)
	length := len(content)
	if length >= 2 {
		// json 最短长度为2
		start := content[0]
		end := content[length-1]
		if (start == '{' && end == '}') || (start == '[' && end == ']') {
			// 这是json
			requestHeader[ContextType] = ApplicationJson
		}
	}
	data, _, err = Request(url, MethodPost, content, requestHeader)
	return data, err
}

// Request http request, 支持传入header
func Request(url string, method string, content string, header ...map[string]any) (data []byte, lastModified time.Time, err error) {
	u, err := stdNetUrl.Parse(url)
	if err != nil {
		return nil, TimeZero, err
	}
	reqHeader := maps.Clone(defaultHeaders)
	reqHeader["Host"] = u.Host
	if len(header) > 0 {
		mapHeader := header[0]
		for k, v := range mapHeader {
			switch val := v.(type) {
			case time.Time:
				val = val.UTC()
				reqHeader[k] = val.Format(time.RFC1123)
			case float32, float64:
				reqHeader[k] = fmt.Sprintf("%f", val)
			case int8, int16, int32, int64:
				reqHeader[k] = fmt.Sprintf("%d", val)
			case uint8, uint16, uint32, uint64:
				reqHeader[k] = fmt.Sprintf("%d", val)
			case string:
				reqHeader[k] = val
			default:
				reqHeader[k] = fmt.Sprintf("%v", val)
			}
		}
	}

	client := defaultClient()
	var requestBody io.Reader = nil
	if len(content) > 0 {
		requestBody = strings.NewReader(content)
	}
	request, err := stdNetHttp.NewRequest(strings.ToUpper(method), url, requestBody)
	if err != nil {
		return nil, TimeZero, err
	}
	for key, v := range reqHeader {
		request.Header.Add(key, v)
	}

	response, err := client.Do(request)
	if err != nil {
		return nil, TimeZero, err
	}
	if response.StatusCode == stdNetHttp.StatusNotFound {
		return nil, TimeZero, NotFound
	}
	lm := response.Header.Get(LastModified)
	if response.StatusCode == stdNetHttp.StatusNotModified && !std.IsEmpty(lm) {
		return nil, TimeZero, nil
	}
	lastModified, err = time.Parse(time.RFC1123, lm)
	defer std.CloseQuietly(response.Body)
	body, err := io.ReadAll(response.Body)
	if err != nil {
		return nil, TimeZero, err
	}
	contentEncoding := response.Header.Get(ContentEncoding)
	var reader io.ReadCloser = nil
	if len(contentEncoding) > 0 {
		contentEncoding = strings.ToLower(contentEncoding)
		switch contentEncoding {
		case "gzip":
			reader, err = gzip.NewReader(bytes.NewBuffer(body))
			if err != nil {
				log.Error(err)
				reader = nil
			}
		case "deflate":
			reader = flate.NewReader(bytes.NewReader(body))
		}
	}
	if reader != nil {
		defer std.CloseQuietly(reader)
		body, err = io.ReadAll(reader)
		if err != nil {
			return nil, TimeZero, err
		}
	}
	return body, lastModified, nil
}
