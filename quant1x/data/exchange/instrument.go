package exchange

import (
	"fmt"
	"strings"
)

func BuildInstrument(exchange Exchange, ticker string) string {
	if exchange == ExchangeUS {
		return fmt.Sprintf("%s.%s", exchange, ticker)
	}
	return fmt.Sprintf("%s%s", exchange, ticker)
}

// SecurityType 表示证券类型（枚举，基于 uint8）
type SecurityType uint8

const (
	SecurityTypeUnknown   SecurityType = iota // 未知类型
	SecurityTypeStock                         // A股/普通股票
	SecurityTypeETF                           // 交易所交易基金
	SecurityTypeFund                          // 各类基金（如 LOF、封闭式基金等）
	SecurityTypeBond                          // 债券（公司债、可转债、可交换债等）
	SecurityTypeStockB                        // B股
	SecurityTypeIPO                           // 新股申购/新股
	SecurityTypeIndex                         // 指数（如上证指数、深证指数等）
	SecurityTypeBlock                         // 板块/板块指数
	SecurityTypeOption                        // 期权
	SecurityTypeFuture                        // 期货
	SecurityTypeWarrant                       // 权证
	SecurityTypeForex                         // 外汇
	SecurityTypeCommodity                     // 商品
	SecurityTypeOther     = 255               // 其他类型
)

// IsEquity 判断证券类型是否为股票类型（包括A股和B股）
func (st SecurityType) IsEquity() bool {
	return st == SecurityTypeStock || st == SecurityTypeStockB
}

// IsIndex 判断证券类型是否为指数类型或板块类型
func (st SecurityType) IsIndex() bool {
	return st == SecurityTypeIndex || st == SecurityTypeBlock
}

func (st SecurityType) IsETF() bool {
	return st == SecurityTypeETF
}

func (st SecurityType) String() string {
	switch st {
	case SecurityTypeStock:
		return "stock"
	case SecurityTypeETF:
		return "etf"
	case SecurityTypeFund:
		return "fund"
	case SecurityTypeBond:
		return "bond"
	case SecurityTypeStockB:
		return "stockb"
	case SecurityTypeIPO:
		return "ipo"
	case SecurityTypeIndex:
		return "index"
	case SecurityTypeBlock:
		return "block"
	case SecurityTypeOption:
		return "option"
	case SecurityTypeFuture:
		return "future"
	case SecurityTypeWarrant:
		return "warrant"
	case SecurityTypeForex:
		return "forex"
	case SecurityTypeCommodity:
		return "commodity"
	case SecurityTypeOther:
		return "other"
	default:
		return "unknown"
	}
}

func (st SecurityType) MarshalCSV() (string, error) {
	return st.String(), nil
}
func (st *SecurityType) UnmarshalCSV(text string) error {
	val := strings.TrimSpace(text) // 兼容前后空格
	val = strings.ToLower(val)     // 兼容大小写
	switch val {
	case "stock":
		*st = SecurityTypeStock
	case "etf":
		*st = SecurityTypeETF
	case "fund":
		*st = SecurityTypeFund
	case "bond":
		*st = SecurityTypeBond
	case "stockb":
		*st = SecurityTypeStockB
	case "ipo":
		*st = SecurityTypeIPO
	case "index":
		*st = SecurityTypeIndex
	case "block":
		*st = SecurityTypeBlock
	case "option":
		*st = SecurityTypeOption
	case "future":
		*st = SecurityTypeFuture
	case "warrant":
		*st = SecurityTypeWarrant
	case "forex":
		*st = SecurityTypeForex
	case "commodity":
		*st = SecurityTypeCommodity
	case "other":
		*st = SecurityTypeOther
	default:
		*st = SecurityTypeUnknown
	}
	return nil
}

// InstrumentInfo 证券信息结构体
type InstrumentInfo struct {
	Exchange       Exchange     `csv:"exchange"`        // 交易所代码（如 SH, SZ, NASDAQ）
	Type           SecurityType `csv:"type"`            // 证券类型（股票、债券、期货等）
	Ticker         string       `csv:"code"`            // 交易所分配的证券代码（ticker）
	Name           string       `csv:"name"`            // 证券名称
	LotSize        int          `csv:"lot_size"`        // 每手股数
	PricePrecision int          `csv:"price_precision"` // 价格小数位数
}

func (info InstrumentInfo) String() string {
	return fmt.Sprintf("InstrumentInfo{Exchange:%s,Type:%s,Ticker:%s,Name:%s,LotSize:%d,PricePrecision:%d}",
		info.Exchange, info.Type.String(), info.Ticker, info.Name, info.LotSize, info.PricePrecision)
}

// Symbol 根据交易所信息返回格式化后的证券代码字符串
//
// 格式说明:
//   - 对于美国交易所(ExchangeUS)，返回格式为"交易所.股票代码", 如 "nasdaq.aapl"
//   - 对于其他交易所，返回格式为"交易所股票代码", 如 "sh600000"
func (info InstrumentInfo) Symbol() string {
	if info.Exchange == ExchangeUS {
		return fmt.Sprintf("%s.%s", info.Exchange, info.Ticker)
	}
	return fmt.Sprintf("%s%s", info.Exchange, info.Ticker)
}

func (info *InstrumentInfo) MarshalCSV() (string, error) {
	if info == nil {
		return "", nil
	}

	return fmt.Sprintf("%s%s", info.Exchange, info.Ticker), nil
}

func (info *InstrumentInfo) UnmarshalCSV(val string) error {
	text := strings.TrimSpace(val)
	if len(text) < 2 {
		return ErrSecurityCodeSymbolEmpty
	}

	marketCode := text[:2]
	exchange := ParseExchangeCode(marketCode)
	if exchange == ExchangeUnknown {
		return fmt.Errorf("invalid exchange code: %s", marketCode)
	}
	symbol := text[2:]

	tmp := DetectWithExchange(exchange, symbol)

	info.Exchange = tmp.Exchange
	info.Ticker = tmp.Ticker
	info.Type = tmp.Type
	return nil
}

// AssertIndexByMarketAndCode 根据交易所ID和代码判断是否为指数代码
//
// 参数:
//
//	m: 交易所ID
//	code: 证券代码
//
// 返回值:
//
//	bool: 如果是该交易所的指数代码返回true，否则返回false
func AssertIndexByMarketAndCode(ex Exchange, code string) bool {
	symbol := strings.TrimSpace(code)
	switch ex {
	case ExchangeSSE:
		return strings.HasPrefix(symbol, "000") || strings.HasPrefix(symbol, "880") || strings.HasPrefix(symbol, "881")
	case ExchangeSZSE:
		return strings.HasPrefix(symbol, "399")
	case ExchangeBSE:
		return strings.HasPrefix(symbol, "899")
	default:
		return false
	}
}
