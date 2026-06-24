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

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/level1/std"
	"github.com/quant1x/quant1x/quant1x/encoding"
	"golang.org/x/text/encoding/simplifiedchinese"
)

func TestBuildRequest(t *testing.T) {
	atomic.StoreUint32(&seqId, 0)
	ctx := NewHeartbeatContext()
	payload := SerializeRequest(ctx)

	wantLen := RequestHeaderLength
	if len(payload) != wantLen {
		t.Fatalf("unexpected request length: got %d want %d", len(payload), wantLen)
	}
	if payload[0] != FlagUncompressed {
		t.Fatalf("expected zip flag 0x%02X got 0x%02X", FlagUncompressed, payload[0])
	}
	if seq := binary.LittleEndian.Uint32(payload[1:5]); seq != 1 {
		t.Fatalf("expected seq 1 got %d", seq)
	}
	if pkt := payload[5]; pkt != PacketCtrlHeartbeat {
		t.Fatalf("expected packet type 0x02 got 0x%02X", pkt)
	}
	pkgLen := uint16(2)
	if got := binary.LittleEndian.Uint16(payload[6:8]); got != pkgLen {
		t.Fatalf("unexpected pkgLen1: got %d want %d", got, pkgLen)
	}
	if got := binary.LittleEndian.Uint16(payload[8:10]); got != pkgLen {
		t.Fatalf("unexpected pkgLen2: got %d want %d", got, pkgLen)
	}
	if method := binary.LittleEndian.Uint16(payload[10:12]); method != uint16(StdCommandHeartbeat) {
		t.Fatalf("unexpected method: got 0x%04X", method)
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
	if hdr.MagicNumber != 0xAABBCCDD || hdr.FrameType != FlagUncompressed || hdr.SeqId != 42 || hdr.PacketCtrl != 0x01 || hdr.Method != 128 || hdr.BodyWireLen != 256 || hdr.BodyRawLen != 512 {
		t.Fatalf("unexpected header %+v", hdr)
	}
}

func TestHeartbeatRequest(t *testing.T) {
	atomic.StoreUint32(&seqId, 0)
	ctx := std.NewHeartbeatContext()
	payload := SerializeRequest(ctx)
	if len(payload) != RequestHeaderLength {
		t.Fatalf("unexpected heartbeat request length: %d", len(payload))
	}
	if payload[5] != PacketCtrlHeartbeat {
		t.Fatalf("unexpected packet type: 0x%02X", payload[5])
	}
	method := binary.LittleEndian.Uint16(payload[10:12])
	if method != uint16(StdCommandHeartbeat) {
		t.Fatalf("unexpected method: 0x%04X", method)
	}
}

func TestHeartbeatResponseDeserialize(t *testing.T) {
	payload := make([]byte, std.HeartbeatInfoLength)
	copy(payload, []byte("ALIVE"))
	ctx := std.NewHeartbeatContext()
	if err := ctx.DeserializeResponseBody(payload); err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if ctx.Info != "ALIVE" {
		t.Fatalf("unexpected info: %q", ctx.Info)
	}
	if err := ctx.DeserializeResponseBody([]byte("short")); err == nil {
		t.Fatalf("expected error for short payload")
	}
}

func TestGBKToUTF8(t *testing.T) {
	input := "测试"
	encoded, err := simplifiedchinese.GBK.NewEncoder().Bytes([]byte(input))
	if err != nil {
		t.Fatalf("gbk encode failed: %v", err)
	}
	out, err := encoding.GBKToUTF8(encoded)
	if err != nil {
		t.Fatalf("GBKToUTF8 failed: %v", err)
	}
	if out != input {
		t.Fatalf("unexpected conversion result: got %q want %q", out, input)
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
