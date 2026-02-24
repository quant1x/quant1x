// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package meta

import (
	"fmt"
	"strings"
)

// Subtype 资产子类型(高4位), 语义由主类型(InstrumentType)决定
type Subtype uint8

const (
	SubtypeDefault        Subtype = 0x00 // 默认/无特殊子类(如A股、普通指数), 默认市场
	SubtypeChiNext        Subtype = 0x10 // 深交所, 创业板, ChiNext
	SubtypeStar           Subtype = 0x20 // 上交所, 科创板, STAR(The Science and Technology Innovation Board)
	SubtypeB              Subtype = 0x30 // B股(STOCK)/ 认购(OPTION预留), B股市场
	SubtypeH              Subtype = 0x40 // H股(STOCK)/ 认沽(OPTION预留), H股市场
	SubtypeGem            Subtype = 0x50 // 港交所创业板, 成长型企业市场(Growth_Enterprises_Market), 港交所创业板市场
	SubtypeExchangeTraded Subtype = 0x60 // 交易型开放式
	SubtypeListed         Subtype = 0x70 // 上市型开放式
	SubtypeOpenEnded      Subtype = 0x80 // 开放式
	SubtypeMutual         Subtype = 0xB0 // 公募市场
	SubtypePrivate        Subtype = 0xC0 // 私募市场
	SubtypeMoney          Subtype = 0xD0 // 货币(FOREX), 货币市场
	SubtypeSpecial        Subtype = 0xE0 // 特殊变体：IPO(STOCK)、板块(INDEX)等
	SubtypeTemporary      Subtype = 0xF0 // 临时市场：临时合约(FUTURE)等
)

// InstrumentType 合约类型(低4位=资产大类, 高4位=子类型扩展)
type InstrumentType uint8

const (
	InstrumentTypeUnknown   InstrumentType = 0x00 // 未知类型
	InstrumentTypeIndex     InstrumentType = 0x01 // 指数(含普通指数、板块等)
	InstrumentTypeStock     InstrumentType = 0x02 // 股票(默认A股)
	InstrumentTypeFund      InstrumentType = 0x03 // 基金
	InstrumentTypeBond      InstrumentType = 0x04 // 债券
	InstrumentTypeForex     InstrumentType = 0x05 // 外汇
	InstrumentTypeCommodity InstrumentType = 0x06 // 商品现货
	InstrumentTypeFuture    InstrumentType = 0x07 // 期货
	InstrumentTypeOption    InstrumentType = 0x08 // 期权
	InstrumentTypeWarrant   InstrumentType = 0x09 // 权证
	// 0x0B-0x0E 预留基础类型扩展
	InstrumentTypeMacro InstrumentType = 0x0F // 宏观指标

	// === 组合类型(命名空间化, 便于使用)===

	// 股票子类
	InstrumentTypeBStock InstrumentType = InstrumentType(SubtypeB) | InstrumentTypeStock       // B股
	InstrumentTypeHStock InstrumentType = InstrumentType(SubtypeH) | InstrumentTypeStock       // H股
	InstrumentTypeIPO    InstrumentType = InstrumentType(SubtypeSpecial) | InstrumentTypeStock // IPO

	InstrumentTypeChiNextMarket  InstrumentType = InstrumentType(SubtypeChiNext) | InstrumentTypeStock   // 深交所, 创业板
	InstrumentTypeStarMarket     InstrumentType = InstrumentType(SubtypeStar) | InstrumentTypeStock      // 上交所, 科创板
	InstrumentTypeGemMarket      InstrumentType = InstrumentType(SubtypeGem) | InstrumentTypeStock       // 港交所, 创业板
	InstrumentTypeTemporaryStock InstrumentType = InstrumentType(SubtypeTemporary) | InstrumentTypeStock // 港交所, 临时柜台

	// 基金子类
	InstrumentTypeETF           InstrumentType = InstrumentType(SubtypeExchangeTraded) | InstrumentTypeFund // ETF基金
	InstrumentTypeLOF           InstrumentType = InstrumentType(SubtypeListed) | InstrumentTypeFund         // LOF基金(上市型开放式基金, 是中国特色的交易所交易基金品种)
	InstrumentTypeOpenEndedFund InstrumentType = InstrumentType(SubtypeOpenEnded) | InstrumentTypeFund      // 开放式基金
	InstrumentTypeMoneyFund     InstrumentType = InstrumentType(SubtypeMoney) | InstrumentTypeFund          // 货币基金

	// 指数子类(板块作为指数的特殊变体)
	InstrumentTypeSector InstrumentType = InstrumentType(SubtypeSpecial) | InstrumentTypeIndex // 板块

	InstrumentTypeNEEQ  InstrumentType = 0xFE // 新三板/股转系统
	InstrumentTypeOther InstrumentType = 0xFF // 其他未分类
)

// BaseType 提取基础资产类型(低4位)
func (it InstrumentType) BaseType() InstrumentType {
	return it & 0x0F
}

// SubtypeValue 提取子类型扩展位(高4位)
func (it InstrumentType) SubtypeValue() Subtype {
	return Subtype(it & 0xF0)
}

// IsStock 判断是否为股票类型
func (it InstrumentType) IsStock() bool {
	return it.BaseType() == InstrumentTypeStock
}

// IsIndex 判断是否为指数类(含普通指数、板块等)
func (it InstrumentType) IsIndex() bool {
	return it.BaseType() == InstrumentTypeIndex
}

// String 实现 Stringer 接口
func (it InstrumentType) String() string {
	typeNames := map[InstrumentType]string{
		InstrumentTypeUnknown:        "unknown",
		InstrumentTypeIndex:          "index",
		InstrumentTypeStock:          "stock",
		InstrumentTypeFund:           "fund",
		InstrumentTypeBond:           "bond",
		InstrumentTypeForex:          "forex",
		InstrumentTypeCommodity:      "commodity",
		InstrumentTypeFuture:         "future",
		InstrumentTypeOption:         "option",
		InstrumentTypeWarrant:        "warrant",
		InstrumentTypeMacro:          "macro",
		InstrumentTypeBStock:         "bstock",
		InstrumentTypeHStock:         "hstock",
		InstrumentTypeIPO:            "ipo",
		InstrumentTypeChiNextMarket:  "chinext_market",
		InstrumentTypeStarMarket:     "star_market",
		InstrumentTypeGemMarket:      "gem_market",
		InstrumentTypeTemporaryStock: "temporary_stock",
		InstrumentTypeETF:            "etf",
		InstrumentTypeLOF:            "lof",
		InstrumentTypeOpenEndedFund:  "open_ended_fund",
		InstrumentTypeMoneyFund:      "money_fund",
		InstrumentTypeSector:         "sector",
		InstrumentTypeNEEQ:           "neeq",
		InstrumentTypeOther:          "other",
	}
	if name, ok := typeNames[it]; ok {
		return name
	}
	return "unknown"
}

// FromString 从字符串解析 InstrumentType
func InstrumentTypeFromString(s string) InstrumentType {
	key := strings.ToLower(strings.TrimSpace(s))
	// 构建映射
	strToType := map[string]InstrumentType{
		"unknown":         InstrumentTypeUnknown,
		"index":           InstrumentTypeIndex,
		"stock":           InstrumentTypeStock,
		"fund":            InstrumentTypeFund,
		"bond":            InstrumentTypeBond,
		"forex":           InstrumentTypeForex,
		"commodity":       InstrumentTypeCommodity,
		"future":          InstrumentTypeFuture,
		"option":          InstrumentTypeOption,
		"warrant":         InstrumentTypeWarrant,
		"macro":           InstrumentTypeMacro,
		"bstock":          InstrumentTypeBStock,
		"hstock":          InstrumentTypeHStock,
		"ipo":             InstrumentTypeIPO,
		"chinext_market":  InstrumentTypeChiNextMarket,
		"star_market":     InstrumentTypeStarMarket,
		"gem_market":      InstrumentTypeGemMarket,
		"temporary_stock": InstrumentTypeTemporaryStock,
		"etf":             InstrumentTypeETF,
		"lof":             InstrumentTypeLOF,
		"open_ended_fund": InstrumentTypeOpenEndedFund,
		"money_fund":      InstrumentTypeMoneyFund,
		"sector":          InstrumentTypeSector,
		"neeq":            InstrumentTypeNEEQ,
		"other":           InstrumentTypeOther,
	}
	if typ, ok := strToType[key]; ok {
		return typ
	}
	return InstrumentTypeUnknown
}

// Instrument 证券信息结构体
type Instrument struct {
	Exchange       Exchange       // 交易所代码(如 SH, SZ, NASDAQ)
	Type           InstrumentType // 证券类型(股票, 债券, 期货等)
	Ticker         string         // 交易所分配的证券代码(ticker)
	Name           string         // 证券名称
	LotSize        int            // 每手股数
	PricePrecision int            // 价格小数位数
	ExtMarket      int            // 扩展市场代码(如 US, HK)
	ExtCategory    int            // 扩展类别代码(如 STK, FUT, OPT, ...)
	Desc           string         // 证券描述
}

// NewInstrument 创建新的 Instrument 实例
func NewInstrument(exchange Exchange, instType InstrumentType, ticker, name string) *Instrument {
	return &Instrument{
		Exchange:       exchange,
		Type:           instType,
		Ticker:         ticker,
		Name:           name,
		LotSize:        100,
		PricePrecision: 2,
		ExtMarket:      0,
		ExtCategory:    0,
		Desc:           "",
	}
}

// String 实现 Stringer 接口, 返回 symbol() 方法的结果
func (i *Instrument) String() string {
	return i.Symbol()
}

// Symbol 构建交易符号字符串
func (i *Instrument) Symbol() string {
	// normalize
	if i.Exchange.Region() == RegionCN {
		return fmt.Sprintf("%s%s", i.Exchange.Identifier(), i.Ticker)
	}
	return fmt.Sprintf("%s.%s", i.Ticker, i.Exchange.Identifier())
}

// CacheDir 获取缓存目录路径，用于存储交易所相关数据文件
func (i *Instrument) CacheDir() string {
	return strings.ToLower(i.Exchange.String())
}

// ToString 格式化输出证券信息
func (i *Instrument) ToString() string {
	return fmt.Sprintf("Instrument(exchange=%s, type=%s, ticker=%s, name=%s, lot_size=%d, price_precision=%d, ext_market=%d, ext_category=%d)",
		i.Exchange, i.Type, i.Ticker, i.Name, i.LotSize, i.PricePrecision, i.ExtMarket, i.ExtCategory)
}

// Headers 返回表头列表
func (i *Instrument) Headers() []string {
	return []string{"exchange", "type", "code", "name", "lot_size", "price_precision", "ext_market", "ext_category"}
}

// ToDict 将证券对象转换为字典(map)格式
func (i *Instrument) ToDict() map[string]any {
	return map[string]any{
		"exchange":        i.Exchange.Identifier(),
		"type":            i.Type,
		"code":            i.Ticker,
		"name":            i.Name,
		"lot_size":        i.LotSize,
		"price_precision": i.PricePrecision,
		"ext_market":      i.ExtMarket,
		"ext_category":    i.ExtCategory,
	}
}

// ToSlice 将证券对象转换为切片
func (i *Instrument) ToSlice() []any {
	return []any{
		i.Exchange.Identifier(),
		i.Type,
		i.Ticker,
		i.Name,
		i.LotSize,
		i.PricePrecision,
		i.ExtMarket,
		i.ExtCategory,
	}
}

// CanConstructSymbol 检查当前对象是否可以构造有效的交易符号
func (i *Instrument) CanConstructSymbol() bool {
	return i.Exchange != UNKNOWN && i.Type != InstrumentTypeUnknown
}

// IsValid 检查当前证券对象是否有效
func (i *Instrument) IsValid() bool {
	return i.Exchange != UNKNOWN && i.Type != InstrumentTypeUnknown && i.LotSize > 0 && i.PricePrecision > 0
}
