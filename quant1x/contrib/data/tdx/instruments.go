// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.
//
// tdx/instruments - TDX 证券列表缓存加载与初始化
// 对齐 Python contrib/data/tdx/instruments.py 与 Rust instruments.rs / C++ instruments.cpp

package tdx

import (
	"encoding/csv"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"sync"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/level1/std"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data/meta"
	"github.com/quant1x/quant1x/quant1x/runtime"
	logger "github.com/quant1x/quant1x/quant1x/log"
)

// SecurityListPerRequestMax 单次请求最大证券数量, 与 Python/C++/Rust 对齐
const SecurityListPerRequestMax = 1600

// securityMap 内存缓存: symbol -> Instrument
var (
	securityOnce  = runtime.RollingOnceFromSpec(config.GLOBAL_CRON_EXPR_DAILY_INIT)
	securityMutex sync.RWMutex
	securityMap   = make(map[string]meta.Instrument)
)

// loadSecurities 从 CSV 加载到内存 (调用方负责通过 RollingOnce 控制时机)
func loadSecurities() bool {
	fname := config.GetSecurityFilename()
	logger.Debugf("[tdx/instruments] Loading securities from %s", fname)

	securityMutex.Lock()
	defer securityMutex.Unlock()

	securityMap = make(map[string]meta.Instrument)

	file, err := os.Open(fname)
	if err != nil {
		return false
	}
	defer file.Close()

	reader := csv.NewReader(file)
	headers, err := reader.Read()
	if err != nil {
		return false
	}
	_ = headers

	count := 0
	for {
		record, err := reader.Read()
		if err != nil {
			break
		}
		if len(record) < 8 {
			continue
		}
		exchangeStr := record[0]
		if exchangeStr == "" {
			exchangeStr = "unknown"
		}
		exchange, err := meta.ParseExchange(exchangeStr)
		if err != nil {
			exchange = meta.UNKNOWN
		}
		typeStr := record[1]
		if typeStr == "" {
			typeStr = "unknown"
		}
		instType := meta.InstrumentTypeFromString(typeStr)
		ticker := strings.ToLower(record[2])
		name := record[3]
		lotSize := parseInt(record[4], 100)
		pricePrecision := parseInt(record[5], 2)
		extMarket := parseInt(record[6], 0)
		extCategory := parseInt(record[7], 0)

		inst := meta.Instrument{
			Exchange:       exchange,
			Type:           instType,
			Ticker:         ticker,
			Name:           name,
			LotSize:        lotSize,
			PricePrecision: pricePrecision,
			ExtMarket:      extMarket,
			ExtCategory:    extCategory,
		}
		symbol := inst.Symbol()
		securityMap[symbol] = inst
		count++
	}
	logger.Infof("[tdx/instruments] loaded %d instruments from %s", count, fname)
	return count > 0
}

// parseInt 解析整数字符串, 失败时返回默认值
func parseInt(s string, def int) int {
	s = strings.TrimSpace(s)
	if s == "" {
		return def
	}
	v, err := strconv.Atoi(s)
	if err != nil {
		return def
	}
	return v
}

// fetchSecurityList 从 TDX 标准行情服务器获取一页证券
// 对齐 Python instruments.fetch_security_list() (标准行情部分)
func fetchSecurityList(exchange meta.Exchange, start, count int) []meta.Instrument {
	var result []meta.Instrument
	if tdxproto.ExchangeToMarketId(exchange) < 0 {
		logger.Errorf("[tdx/instruments] unsupported exchange for std market: %s", exchange)
		return result
	}

	conn, release, err := GetStdConnection()
	if err != nil {
		logger.Errorf("[tdx/instruments] get std connection failed: %v", err)
		return result
	}
	if release != nil {
		defer release()
	}
	if conn == nil || conn.Conn() == nil {
		logger.Errorf("[tdx/instruments] nil connection from level1 client")
		return result
	}

	msg := std.NewSecurityListContext(exchange, start, count)
	if err := tdxproto.TransactMessageSync(conn, msg); err != nil {
		logger.Errorf("[tdx/instruments] SecurityListContext fetch failed: %s %v", exchange, err)
		return result
	}

	logger.Debugf("[tdx/instruments] SecurityListContext %s start=%d count=%d got %d records",
		exchange, start, count, len(msg.List))

	for _, sec := range msg.List {
		inst := meta.Instrument{
			Exchange:       exchange,
			Ticker:         strings.ToLower(sec.Code),
			Name:           sec.Name,
			LotSize:        int(sec.VolUnit),
			PricePrecision: int(sec.DecimalPoint),
			ExtMarket:      tdxproto.ExchangeToMarketId(exchange), // 与 Python 对齐: ext_market = exchange_to_market(exchange)
		}
		inst.Type = data.DetectInstrumentTypeByRule(exchange, inst.Ticker)
		inst.ExtCategory = int(inst.Type)
		result = append(result, inst)
	}
	return result
}

// writeSecuritiesCsv 将证券列表写入 CSV
func writeSecuritiesCsv(fname string, instruments []meta.Instrument) {
	// Ensure parent directory exists
	if dir := filepath.Dir(fname); dir != "" {
		_ = os.MkdirAll(dir, 0o755)
	}

	file, err := os.Create(fname)
	if err != nil {
		logger.Errorf("[tdx/instruments] cannot create %s: %v", fname, err)
		return
	}
	defer file.Close()

	writer := csv.NewWriter(file)
	defer writer.Flush()

	var zeroInstrument meta.Instrument
	headers := zeroInstrument.Headers()
	if err := writer.Write(headers); err != nil {
		logger.Errorf("[tdx/instruments] cannot write header: %v", err)
		return
	}

	for i := range instruments {
		row := instruments[i].ToSlice()
		strRow := make([]string, len(row))
		for j, cell := range row {
			strRow[j] = cellToString(cell)
		}
		if err := writer.Write(strRow); err != nil {
			logger.Errorf("[tdx/instruments] cannot write row: %v", err)
			return
		}
	}
	writer.Flush()
	if err := writer.Error(); err != nil {
		logger.Errorf("[tdx/instruments] cannot flush %s: %v", fname, err)
		return
	}
	logger.Infof("[tdx/instruments] wrote %d instruments to %s", len(instruments), fname)
}

// cellToString 将 CSV 单元格值转为字符串
func cellToString(v any) string {
	switch t := v.(type) {
	case string:
		return t
	case meta.InstrumentType:
		return t.String()
	case int:
		return strconv.Itoa(t)
	default:
		return ""
	}
}

// doInitSecurities 实际初始化逻辑, 由 RollingOnce::Do 调用
func doInitSecurities() {
	fname := config.GetSecurityFilename()

	// Step 1: 检查是否需要更新
	createOrUpdate := meta.ShouldInitializeFile(fname)
	if !createOrUpdate {
		// CSV 存在且是今天的, 尝试加载
		createOrUpdate = !loadSecurities()
	}
	logger.Debugf("[tdx/instruments] init_securities create_or_update=%v", createOrUpdate)

	if !createOrUpdate {
		return // 已加载, 无需更新
	}

	// Step 2: 从 TDX 服务器拉取
	var instruments []meta.Instrument

	// 2a. 标准行情: A 股 (SSE/SZSE/BSE)
	// 对齐 Python: markets = [Exchange.SSE, Exchange.SZSE, Exchange.BSE]
	stdMarkets := []meta.Exchange{meta.SSE, meta.SZSE, meta.BSE}
	for _, m := range stdMarkets {
		start := 0
		var rows []meta.Instrument
		for {
			page := fetchSecurityList(m, start, SecurityListPerRequestMax)
			if len(page) == 0 {
				break
			}
			pageSize := len(page)
			rows = append(rows, page...)
			if pageSize < SecurityListPerRequestMax {
				break
			}
			start += SecurityListPerRequestMax
		}

		// 相同市场按代码排序 (对齐 Python: rows.sort(key=lambda x: x.ticker))
		sort.Slice(rows, func(i, j int) bool {
			return rows[i].Ticker < rows[j].Ticker
		})

		logger.Infof("[tdx/instruments] fetched %d instruments from %s", len(rows), m)
		instruments = append(instruments, rows...)
	}

	// 2b. TODO: 扩展行情 (HKEX 等), 与 C++ 对齐, 等 ext 协议基础设施完成后接入
	logger.Infof("[tdx/instruments] ext market (HKEX) not yet implemented - requires ext protocol handler")

	// Step 3: 写入 CSV
	if len(instruments) > 0 {
		writeSecuritiesCsv(fname, instruments)
	} else {
		logger.Warnf("[tdx/instruments] no instruments fetched, CSV not written")
	}

	// Step 4: 加载到内存
	if !loadSecurities() {
		logger.Errorf("[tdx/instruments] failed to load securities after initialization")
	}
}

// InitSecurities 通过 RollingOnce 保证每日首次调用时初始化
// 对齐 Python init_securities() / Rust init_securities()
func InitSecurities() {
	securityOnce.Do(doInitSecurities)
}

// GetCodeList 返回所有 symbol 字符串
// 对齐 C++ get_code_list()
func GetCodeList() []string {
	InitSecurities() // RollingOnce 保证每天只执行一次
	securityMutex.RLock()
	defer securityMutex.RUnlock()

	codes := make([]string, 0, len(securityMap))
	for symbol := range securityMap {
		codes = append(codes, symbol)
	}
	return codes
}

// GetInstrumentInfo 查找单个证券
// 对齐 C++ get_instrument_info()
func GetInstrumentInfo(symbol string) *meta.Instrument {
	securityCode := data.CorrectSecurityCode(symbol)
	logger.Debugf("[tdx/instruments] get_instrument_info: symbol=%s, security_code=%s", symbol, securityCode)

	InitSecurities() // RollingOnce 保证每天只执行一次
	securityMutex.RLock()
	defer securityMutex.RUnlock()

	if inst, ok := securityMap[securityCode]; ok {
		return &inst
	}
	return nil
}

// EnsureSecuritiesInitialized 供外部在策略启动时调用
// 对齐 C++ ensure_securities_initialized()
func EnsureSecuritiesInitialized() {
	InitSecurities()
}
