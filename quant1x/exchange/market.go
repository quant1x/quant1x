package exchange

import "strings"

// Exchange 表示交易所（枚举，基于 string）
type Exchange string

const (
	ExchangeSSE     Exchange = "sh"      // 上海证券交易所
	ExchangeSZSE    Exchange = "sz"      // 深圳证券交易所
	ExchangeBSE     Exchange = "bj"      // 北京证券交易所
	ExchangeHKEX    Exchange = "hk"      // 香港交易所
	ExchangeUS      Exchange = "us"      // 美国证券市场(泛指)
	ExchangeNASDAQ  Exchange = "nasdaq"  // 纳斯达克交易所
	ExchangeNYSE    Exchange = "nyse"    // 纽约证券交易所
	ExchangeAMEX    Exchange = "amex"    // 美国证券交易所
	ExchangeCME     Exchange = "cme"     // 芝加哥商品交易所
	ExchangeICE     Exchange = "ice"     // 洲际交易所
	ExchangeCFFEX   Exchange = "cffex"   // 中国金融期货交易所
	ExchangeDCE     Exchange = "dce"     // 大连商品交易所
	ExchangeCZCE    Exchange = "czce"    // 郑州商品交易所
	ExchangeSHFE    Exchange = "shfe"    // 上海期货交易所
	ExchangeINE     Exchange = "ine"     // 上海国际能源交易中心
	ExchangeUnknown Exchange = "unknown" // 未知交易所
)

func (ex Exchange) String() string {
	return string(ex)
}

func (ex Exchange) MIC() string {
	switch ex {
	case ExchangeSSE:
		return "XSHG"
	case ExchangeSZSE:
		return "XSHE"
	case ExchangeBSE:
		return "XBJSE"
	case ExchangeHKEX:
		return "XHKG"
	case ExchangeUS, ExchangeNASDAQ, ExchangeNYSE, ExchangeAMEX:
		return "XNAS" // 泛指美国市场，使用纳斯达克代码
	case ExchangeCME:
		return "XCME"
	case ExchangeICE:
		return "XICE"
	case ExchangeCFFEX:
		return "XCFF"
	case ExchangeDCE:
		return "XDCE"
	case ExchangeCZCE:
		return "XCZC"
	case ExchangeSHFE:
		return "XSHF"
	case ExchangeINE:
		return "XINE"
	default:
		return ExchangeUnknown.String()
	}
}

// ParseExchangeCode 将交易所代码字符串转换为对应的Exchange枚举值。
//
// 支持的交易所代码包括："sh"(上交所), "sz"(深交所), "bj"(北交所), "hk"(港交所),
// "us"(美国市场), "nasdaq"(纳斯达克), "nyse"(纽交所), "amex"(美交所),
// "cme"(芝加哥商品交易所), "ice"(洲际交易所), "cffex"(中金所),
// "dce"(大商所), "czce"(郑商所), "shfe"(上期所), "ine"(能源中心)等。
// 如果输入无法识别，则返回ExchangeUnknown。
func ParseExchangeCode(text string) Exchange {
	val := strings.ToLower(strings.TrimSpace(text))
	switch val {
	case "sh":
		return ExchangeSSE
	case "sz":
		return ExchangeSZSE
	case "bj":
		return ExchangeBSE
	case "hk":
		return ExchangeHKEX
	case "us":
		return ExchangeUS
	case "nasdaq":
		return ExchangeNASDAQ
	case "nyse":
		return ExchangeNYSE
	case "amex":
		return ExchangeAMEX
	case "cme":
		return ExchangeCME
	case "ice":
		return ExchangeICE
	case "cffex":
		return ExchangeCFFEX
	case "dce":
		return ExchangeDCE
	case "czce":
		return ExchangeCZCE
	case "shfe":
		return ExchangeSHFE
	case "ine":
		return ExchangeINE
	default:
		return ExchangeUnknown
	}
}

// MarketConverter 交易所与市场ID转换接口
type MarketConverter interface {
	// ToMarketId 将交易所转换为市场ID
	ToMarketId(exchange Exchange) int
	// FromMarketId 将市场ID转换为交易所
	FromMarketId(marketID int) Exchange
}

var (
	// AllExchangeCodes 包含所有已知的交易所代码
	AllExchangeCodes = []string{
		ExchangeSSE.String(),
		ExchangeSZSE.String(),
		ExchangeBSE.String(),
		ExchangeHKEX.String(),
		ExchangeUS.String(),
	}
)
