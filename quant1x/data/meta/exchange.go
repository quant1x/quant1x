// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package meta

import (
	"fmt"
	"strings"
)

// Exchange 交易所
type Exchange string

const (
	// 中国市场
	SSE  Exchange = "SSE"  // 上海证券交易所
	XSSC Exchange = "XSSC" // XSSC: 上海证券交易所 - 沪股通
	SZSE Exchange = "SZSE" // 深圳证券交易所
	XSEC Exchange = "XSEC" // XSEC: 深证证券交易所 - 深股通
	BSE  Exchange = "BSE"  // 北京证券交易所

	// 期货交易所
	SHFE  Exchange = "SHFE"  // 上海期货交易所, 主要品种: 金属(铜, 黄金), 能源(原油), 化工(橡胶), 钢材(螺纹钢)等; 国际化品种: 上海国际能源交易中心INE, 隶属SHFE, 负责原油等国际化品种交易
	XINE  Exchange = "XINE"  // 上海国际能源交易中心, 主要品种: 国际化品种(原油, 天然气, 铜, 铝, 锌, 黄金, 白银, 石油, 天然气)
	CZCE  Exchange = "CZCE"  // 郑州商品交易所, 主要品种: 农产品(棉花, 苹果), 化工(PTA), 期权等
	DCE   Exchange = "DCE"   // 大连商品交易所, 主要品种: 农产品(大豆, 玉米), 黑色系(铁矿石), 化工(塑料)等
	CFFEX Exchange = "CFFEX" // 中国金融期货交易所, 主要品种: 股指期货(沪深300指数IF), 国债期货等
	GFEX  Exchange = "GFEX"  // 广州期货交易所, 主要品种: 绿色金融(碳排放权, 工业硅)
	SGE   Exchange = "SGE"   // 上海黄金交易所, 主要品种: 黄金T+D, 白银T+D等

	// 香港
	HKEX Exchange = "HKEX" // 香港交易所(现货股票)
	HKSC Exchange = "HKSC" // 香港交易所-港股通, 虚拟MIC
	HKFE Exchange = "HKFE" // 香港期货交易所(香港指数市场, 指数期货, 商品期货)

	// 指数
	CSI Exchange = "CSI" // 中证指数, China Securities Index, 中证指数有限公司, 虚拟MIC
	CNI Exchange = "CNI" // 国证指数, CNI Index, 深证证券交易所指数机构, 虚拟MIC

	// 扩展
	EXTENDED Exchange = "EXTENDED" // 扩展市场, Extended, 虚拟MIC

	// 离岸/在岸
	OFFSHORE Exchange = "OFFSHORE" // 国际, 其它离岸市场, 虚拟MIC
	ONSHORE  Exchange = "ONSHORE"  // 国内, 其它在岸市场, 虚拟MIC
	OTC      Exchange = "OTC"      // 国内, 场外, 虚拟MIC
	OFFEX    Exchange = "OFFEX"    // 场外申赎市场, Off-exchange Subscription/Redemption, 虚拟MIC

	// 宏观
	MACRO Exchange = "MACRO" // 宏观经济市场, Macro-economic, 虚拟MIC

	// 美国
	USA    Exchange = "USA"    // 美国证券市场(泛指), 虚拟MIC
	NYSE   Exchange = "NYSE"   // 纽约证券交易所
	NASDAQ Exchange = "NASDAQ" // 纳斯达克

	// 英国
	LSE Exchange = "LSE" // 伦敦证券交易所
	GBR Exchange = "GBR" // 英国证券市场(泛指), 虚拟MIC

	// 新加坡
	SGX Exchange = "SGX" // 新加坡交易所

	// 其它
	MIRROR Exchange = "MIRROR" // 镜像市场, Mirror, 虚拟MIC
	TEMP   Exchange = "TEMP"   // 临时市场, Temporary, 虚拟MIC

	UNKNOWN Exchange = "UNKNOWN" // 未知交易所, 虚拟MIC
)

// ExchangeInfo 交易所信息
type ExchangeInfo struct {
	MIC        string // MIC: Market Identifier Code, used for exchanges and market identification
	Identifier string // 标识: 交易所的小写缩写, 如 sh/sz/bj/hk, 与系统缓存的证券代码列表对应
	Region     Region // 市场
	Label      string // 交易所名称
}

// exchangeDataMap 交易所数据映射
var exchangeDataMap = map[Exchange]ExchangeInfo{
	SSE:      {"XSHG", "sh", RegionCN, "上海证券交易所"},
	XSSC:     {"XSSC", "sh", RegionCN, "上海证券交易所"},
	SZSE:     {"XSHE", "sz", RegionCN, "深圳证券交易所"},
	XSEC:     {"XSEC", "sz", RegionCN, "深圳证券交易所"},
	BSE:      {"BJSE", "bj", RegionCN, "北京证券交易所"},
	SHFE:     {"XSGE", "shfe", RegionCN, "上海期货交易所"},
	XINE:     {"XINE", "ine", RegionCN, "上海国际能源交易中心"},
	CZCE:     {"XZCE", "zce", RegionCN, "郑州商品交易所"},
	DCE:      {"XDCE", "dce", RegionCN, "大连商品交易所"},
	CFFEX:    {"CCFX", "cff", RegionCN, "中国金融期货交易所"},
	GFEX:     {"GFEX", "gfex", RegionCN, "广州期货交易所"},
	SGE:      {"SGEX", "sge", RegionCN, "上海黄金交易所"},
	HKEX:     {"XHKG", "hk", RegionHK, "香港交易所(现货股票)"},
	HKSC:     {"XHKG", "hksc", RegionHK, "香港交易所-港股通"},
	HKFE:     {"XHKF", "hkf", RegionHK, "香港期货交易所(香港指数市场, 指数期货, 商品期货)"},
	CSI:      {"CSI", "csi", RegionCN, "中证指数, China Securities Index, 中证指数有限公司"},
	CNI:      {"CNI", "cni", RegionCN, "国证指数, CNI Index, 深证证券交易所指数机构"},
	EXTENDED: {"EXTENDED", "ext", RegionGLB, "扩展市场, Extended"},
	OFFSHORE: {"OFFSHORE", "os", RegionOFFSHORE, "国际, 其它离岸市场"},
	ONSHORE:  {"ONSHORE", "on", RegionONSHORE, "国内, 其它在岸市场"},
	OTC:      {"OTC", "otc", RegionONSHORE, "国内, 场外"},
	OFFEX:    {"OFFEX", "offex", RegionONSHORE, "场外申赎市场, Off-exchange Subscription/Redemption"},
	MACRO:    {"MACRO", "macro", RegionGLB, "宏观经济市场, Macro-economic"},
	USA:      {"USA", "us", RegionUS, "美国证券市场(泛指)"},
	NYSE:     {"XNYS", "us", RegionUS, "纽约证券交易所"},
	NASDAQ:   {"XNAS", "us", RegionUS, "纳斯达克"},
	LSE:      {"XLON", "uk", RegionUK, "伦敦证券交易所"},
	GBR:      {"GBR", "uk", RegionUK, "英国证券市场(泛指)"},
	SGX:      {"XSES", "sg", RegionSG, "新加坡交易所"},
	MIRROR:   {"MIRROR", "mirror", RegionGLB, "镜像市场, Mirror"},
	TEMP:     {"TEMP", "temp", RegionGLB, "临时市场, Temporary"},
	UNKNOWN:  {"UNKNOWN", "unknown", RegionUNKNOWN, "未知交易所"},
}

// Parse 智能解析字符串为 Exchange 实例
func (e *Exchange) Parse(s string) error {
	if s == "" {
		return fmt.Errorf("empty string cannot be parsed to Exchange")
	}

	name := strings.TrimSpace(strings.ToUpper(s))

	// 1. By code (enum name)
	if _, ok := exchangeDataMap[Exchange(name)]; ok {
		*e = Exchange(name)
		return nil
	}

	// 2. By identifier
	identifier := strings.ToLower(name)
	for ex, info := range exchangeDataMap {
		if info.Identifier == identifier {
			*e = ex
			return nil
		}
	}

	// 3. By MIC
	for ex, info := range exchangeDataMap {
		if info.MIC == name {
			*e = ex
			return nil
		}
	}

	return fmt.Errorf("cannot parse exchange from: '%s'", s)
}

// ParseExchange 智能解析字符串为 Exchange 实例(非方法版本)
func ParseExchange(s string) (Exchange, error) {
	var ex Exchange
	err := ex.Parse(s)
	return ex, err
}

// Code 返回交易所代码
func (e Exchange) Code() string {
	return string(e)
}

// String 返回交易所名称
func (e Exchange) String() string {
	return string(e)
}

// Info 返回交易所详细信息
func (e Exchange) Info() ExchangeInfo {
	if info, ok := exchangeDataMap[e]; ok {
		return info
	}
	return exchangeDataMap[UNKNOWN]
}

// MIC 返回 MIC 代码
func (e Exchange) MIC() string {
	return e.Info().MIC
}

// Identifier 返回标识符(小写缩写)
func (e Exchange) Identifier() string {
	return e.Info().Identifier
}

// Region 返回所属区域
func (e Exchange) Region() Region {
	return e.Info().Region
}

// Label 返回交易所名称
func (e Exchange) Label() string {
	return e.Info().Label
}

// ToString 格式化输出交易所信息
func (e Exchange) ToString() string {
	info := e.Info()
	regionCode := "None"
	if info.Region != "" {
		regionCode = string(info.Region)
	}
	return fmt.Sprintf("<Exchange.%s: %s (%s) - %s>", e.Code(), info.Identifier, regionCode, info.Label)
}

// FromCode 根据代码创建 Exchange
func FromCode(code string) (Exchange, error) {
	name := strings.TrimSpace(strings.ToUpper(code))
	ex := Exchange(name)
	if _, ok := exchangeDataMap[ex]; ok {
		return ex, nil
	}
	return "", fmt.Errorf("unknown exchange code: %s", code)
}

// FromAbbr 根据缩写创建 Exchange
func FromAbbr(abbr string) (Exchange, error) {
	identifier := strings.ToLower(strings.TrimSpace(abbr))
	for ex, info := range exchangeDataMap {
		if info.Identifier == identifier {
			return ex, nil
		}
	}
	return "", fmt.Errorf("unknown exchange abbreviation: %s", abbr)
}

// FromMIC 根据 MIC 创建 Exchange
func FromMIC(mic string) (Exchange, error) {
	name := strings.ToUpper(strings.TrimSpace(mic))
	for ex, info := range exchangeDataMap {
		if info.MIC == name {
			return ex, nil
		}
	}
	return "", fmt.Errorf("unknown MIC: %s", mic)
}
