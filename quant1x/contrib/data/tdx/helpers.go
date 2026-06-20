// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package tdx

import (
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
)

// Re-export helper functions from tdxproto for backward compatibility.

var ExchangeToMarketId = tdxproto.ExchangeToMarketId
var MarketIdToExchange = tdxproto.MarketIdToExchange
var GetDatetimeFromUint32 = tdxproto.GetDatetimeFromUint32
var VarintEncode = tdxproto.VarintEncode
var VarintDecode = tdxproto.VarintDecode
var FormatTimestampFromI64 = tdxproto.FormatTimestampFromI64
var Float64IsNaN = tdxproto.Float64IsNaN
var DefaultBaseUnit = tdxproto.DefaultBaseUnit
var InstrumentsToString = tdxproto.InstrumentsToString
var instrumentsToString = tdxproto.InstrumentsToString
