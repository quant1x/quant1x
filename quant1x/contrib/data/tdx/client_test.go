package tdx

import (
	"bytes"
	"encoding/binary"
	"os"
	"path/filepath"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"golang.org/x/text/encoding/simplifiedchinese"
)

func TestBuildRequest(t *testing.T) {
	atomic.StoreUint32(&seqId, 0)
	payload := []byte{0xAA, 0xBB, 0xCC}
	req := buildRequest(StdCommandSecurityList, packetTypeHeartbeat, payload)

	wantLen := 1 + 4 + 1 + 2 + 2 + 2 + len(payload)
	if len(req) != wantLen {
		t.Fatalf("unexpected request length: got %d want %d", len(req), wantLen)
	}
	if req[0] != FlagUncompressed {
		t.Fatalf("expected zip flag 0x%02X got 0x%02X", FlagUncompressed, req[0])
	}
	if seq := binary.LittleEndian.Uint32(req[1:5]); seq != 1 {
		t.Fatalf("expected seq 1 got %d", seq)
	}
	if pkt := req[5]; pkt != 0x02 {
		t.Fatalf("expected packet type 0x02 got 0x%02X", pkt)
	}
	pkgLen := uint16(2 + len(payload))
	if got := binary.LittleEndian.Uint16(req[6:8]); got != pkgLen {
		t.Fatalf("unexpected pkgLen1: got %d want %d", got, pkgLen)
	}
	if got := binary.LittleEndian.Uint16(req[8:10]); got != pkgLen {
		t.Fatalf("unexpected pkgLen2: got %d want %d", got, pkgLen)
	}
	if method := binary.LittleEndian.Uint16(req[10:12]); method != uint16(StdCommandSecurityList) {
		t.Fatalf("unexpected method: got 0x%04X", method)
	}
	if !bytes.Equal(req[12:], payload) {
		t.Fatalf("payload mismatch")
	}
}

func TestReadResponseHeader(t *testing.T) {
	buf := &bytes.Buffer{}
	write := func(v any) {
		if err := binary.Write(buf, binary.LittleEndian, v); err != nil {
			t.Fatalf("write failed: %v", err)
		}
	}
	write(uint32(0xAABBCCDD))
	buf.WriteByte(0x0C)
	write(uint32(42))
	buf.WriteByte(0x01)
	write(uint16(128))
	write(uint16(256))
	write(uint16(512))

	hdr, err := readResponseHeader(buf)
	if err != nil {
		t.Fatalf("readResponseHeader failed: %v", err)
	}
	if hdr.I1 != 0xAABBCCDD || hdr.FrameType != FlagUncompressed || hdr.SeqId != 42 || hdr.I2 != 0x01 || hdr.Method != 128 || hdr.BodyWireLen != 256 || hdr.BodyRawLen != 512 {
		t.Fatalf("unexpected header %+v", hdr)
	}
}

func TestHeartbeatRequestBytes(t *testing.T) {
	atomic.StoreUint32(&seqId, 0)
	req := HeartbeatRequest{}.Bytes()
	if len(req) != 12 {
		t.Fatalf("unexpected heartbeat request length: %d", len(req))
	}
	if req[5] != packetTypeHeartbeat {
		t.Fatalf("unexpected packet type: 0x%02X", req[5])
	}
	method := binary.LittleEndian.Uint16(req[10:12])
	if method != uint16(StdCommandHeartbeat) {
		t.Fatalf("unexpected method: 0x%04X", method)
	}
}

func TestHeartbeatResponseDeserialize(t *testing.T) {
	payload := make([]byte, heartbeatInfoLength)
	copy(payload, []byte("ALIVE"))
	var resp HeartbeatResponse
	if err := resp.Deserialize(payload); err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if resp.Info != "ALIVE" {
		t.Fatalf("unexpected info: %q", resp.Info)
	}
	if err := resp.Deserialize([]byte("short")); err == nil {
		t.Fatalf("expected error for short payload")
	}
}

func TestGBKToUTF8(t *testing.T) {
	input := "测试"
	encoded, err := simplifiedchinese.GBK.NewEncoder().Bytes([]byte(input))
	if err != nil {
		t.Fatalf("gbk encode failed: %v", err)
	}
	out, err := gbkToUTF8(encoded)
	if err != nil {
		t.Fatalf("gbkToUTF8 failed: %v", err)
	}
	if out != input {
		t.Fatalf("unexpected conversion result: got %q want %q", out, input)
	}
}

func TestDecodeHelloInfo(t *testing.T) {
	message := "欢迎"
	encoded, err := simplifiedchinese.GBK.NewEncoder().Bytes([]byte(message))
	if err != nil {
		t.Fatalf("gbk encode failed: %v", err)
	}
	body := append(make([]byte, 8), encoded...)
	out, err := decodeHelloInfo(body, 8)
	if err != nil {
		t.Fatalf("decodeHelloInfo failed: %v", err)
	}
	if out != message {
		t.Fatalf("unexpected message: got %q want %q", out, message)
	}
	if _, err := decodeHelloInfo([]byte{0x00}, 10); err == nil {
		t.Fatalf("expected error for short body")
	}
	empty, _ := simplifiedchinese.GBK.NewEncoder().Bytes([]byte("   "))
	body = append(make([]byte, 2), empty...)
	if _, err := decodeHelloInfo(body, 2); err == nil {
		t.Fatalf("expected error for blank message")
	}
}

func TestShouldRefreshCache(t *testing.T) {
	if !shouldRefreshCache(nil) {
		t.Fatalf("expected refresh for nil info")
	}

	dir := t.TempDir()
	emptyPath := filepath.Join(dir, "empty.yaml")
	if err := os.WriteFile(emptyPath, nil, 0o644); err != nil {
		t.Fatalf("write failed: %v", err)
	}
	info, err := os.Stat(emptyPath)
	if err != nil {
		t.Fatalf("stat failed: %v", err)
	}
	if !shouldRefreshCache(info) {
		t.Fatalf("expected refresh for zero-size file")
	}

	freshPath := filepath.Join(dir, "fresh.yaml")
	if err := os.WriteFile(freshPath, []byte("data"), 0o644); err != nil {
		t.Fatalf("write failed: %v", err)
	}
	info, err = os.Stat(freshPath)
	if err != nil {
		t.Fatalf("stat failed: %v", err)
	}
	if shouldRefreshCache(info) {
		t.Fatalf("did not expect refresh for fresh file")
	}

	stalePath := filepath.Join(dir, "stale.yaml")
	if err := os.WriteFile(stalePath, []byte("data"), 0o644); err != nil {
		t.Fatalf("write failed: %v", err)
	}
	old := time.Now().Add(-(cacheRefreshInterval + time.Minute))
	if err := os.Chtimes(stalePath, old, old); err != nil {
		t.Fatalf("chtimes failed: %v", err)
	}
	info, err = os.Stat(stalePath)
	if err != nil {
		t.Fatalf("stat failed: %v", err)
	}
	if !shouldRefreshCache(info) {
		t.Fatalf("expected refresh for stale file")
	}
}

func TestSaveAndLoadCachedServers(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "servers.yaml")
	servers := []serverInfo{{Source: "test", Name: "srv", Host: "localhost", Port: 7709, LatencyMS: 5}}

	if err := saveCachedServers(path, servers); err != nil {
		t.Fatalf("saveCachedServers failed: %v", err)
	}
	loaded, info, err := loadCachedServers(path)
	if err != nil {
		t.Fatalf("loadCachedServers failed: %v", err)
	}
	if info == nil {
		t.Fatalf("expected non-nil file info")
	}
	if len(loaded) != 1 || loaded[0] != servers[0] {
		t.Fatalf("unexpected loaded servers: %+v", loaded)
	}
}

func TestDecodeServerListLegacyFormat(t *testing.T) {
	data := []byte("- host: example.com\n  port: 1234\n  latency: 42\n  source: legacy\n  desc: legacy-name\n")
	servers, err := decodeServerList(data)
	if err != nil {
		t.Fatalf("decodeServerList failed: %v", err)
	}
	if len(servers) != 1 {
		t.Fatalf("expected 1 server got %d", len(servers))
	}
	got := servers[0]
	if got.Source != "legacy" || got.Host != "example.com" || got.Port != 1234 || got.LatencyMS != 0 {
		t.Fatalf("unexpected server: %+v", got)
	}
	if got.Name != "" {
		t.Fatalf("expected empty name when only desc provided, got %q", got.Name)
	}
}

func TestEnsureServerCachePathUsesEnv(t *testing.T) {
	dir := t.TempDir()
	t.Setenv("QUANT1X_HOME", dir)

	path, err := ensureServerCachePath()
	if err != nil {
		t.Fatalf("ensureServerCachePath failed: %v", err)
	}
	wantDir := filepath.Join(dir, "meta")
	if filepath.Dir(path) != wantDir {
		t.Fatalf("unexpected cache dir: got %s want %s", filepath.Dir(path), wantDir)
	}
	if filepath.Base(path) != serverCacheFileName {
		t.Fatalf("unexpected cache file name: %s", filepath.Base(path))
	}
	if _, err := os.Stat(wantDir); err != nil {
		t.Fatalf("expected meta directory to exist: %v", err)
	}
}

func TestDetectServersReal(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping real network detection in short mode")
	}

	handler := NewStandardProtocolHandler(3*time.Second, 3*time.Second).(*StandardProtocolHandler)
	start := time.Now()
	servers := detectServers(handler, 1500*time.Millisecond, 3, 3*time.Second)
	elapsed := time.Since(start)

	if len(servers) == 0 {
		t.Fatalf("detectServers returned no reachable endpoints within %v", elapsed)
	}
	for _, srv := range servers {
		if srv.Host == "" || srv.Port == 0 {
			t.Fatalf("invalid server entry: %+v", srv)
		}
	}
}

func TestClientAcquireReal(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping real client acquire in short mode")
	}

	poolOnce = sync.Once{}
	poolInstance = nil
	poolErr = nil

	dir := t.TempDir()
	t.Setenv("QUANT1X_HOME", dir)

	conn, release, err := GetStdConnection()
	if err != nil {
		t.Fatalf("GetStdConnection() returned error: %v", err)
	}
	if conn == nil {
		t.Fatalf("GetStdConnection() returned nil connection")
	}
	if release == nil {
		t.Fatalf("GetStdConnection() returned nil release func")
	}
	release()
}
