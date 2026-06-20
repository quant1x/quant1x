// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package tdx

import (
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
)

// Re-export protocol types from tdxproto, 对齐 C++/Rust/Python.

type StdCommand = tdxproto.StdCommand

const (
	StdCommandHeartbeat              = tdxproto.StdCommandHeartbeat
	StdCommandLogin1                 = tdxproto.StdCommandLogin1
	StdCommandLogin2                 = tdxproto.StdCommandLogin2
	StdCommandXdxrInfo               = tdxproto.StdCommandXdxrInfo
	StdCommandFinanceInfo            = tdxproto.StdCommandFinanceInfo
	StdCommandPing                   = tdxproto.StdCommandPing
	StdCommandCompanyCategory        = tdxproto.StdCommandCompanyCategory
	StdCommandCompanyContent         = tdxproto.StdCommandCompanyContent
	StdCommandSecurityCount          = tdxproto.StdCommandSecurityCount
	StdCommandSecurityList           = tdxproto.StdCommandSecurityList
	StdCommandOldSecurityList        = tdxproto.StdCommandOldSecurityList
	StdCommandIndexBars              = tdxproto.StdCommandIndexBars
	StdCommandSecurityBars           = tdxproto.StdCommandSecurityBars
	StdCommandSecurityQuotesOld      = tdxproto.StdCommandSecurityQuotesOld
	StdCommandSecurityQuotesNew      = tdxproto.StdCommandSecurityQuotesNew
	StdCommandMinuteTimeData         = tdxproto.StdCommandMinuteTimeData
	StdCommandBlockMeta              = tdxproto.StdCommandBlockMeta
	StdCommandBlockData              = tdxproto.StdCommandBlockData
	StdCommandTransactionData        = tdxproto.StdCommandTransactionData
	StdCommandHistoryMinuteData      = tdxproto.StdCommandHistoryMinuteData
	StdCommandHistoryTransactionData = tdxproto.StdCommandHistoryTransactionData
)

const (
	PacketTypeRequest   = tdxproto.PacketTypeRequest
	PacketCtrlHeartbeat = tdxproto.PacketCtrlHeartbeat
	FlagZip             = tdxproto.FlagZip
	FlagUncompressed    = tdxproto.FlagUncompressed
	FlagZipped          = tdxproto.FlagZipped
)

type RequestHeader = tdxproto.RequestHeader
type ResponseHeader = tdxproto.ResponseHeader
type FrameBase = tdxproto.FrameBase
type BaseFrame = tdxproto.BaseFrame

var NewFrameBase = tdxproto.NewFrameBase
var SerializeRequest = tdxproto.SerializeRequest
var ReadResponseHeader = tdxproto.ReadResponseHeader
var CommandToString = tdxproto.CommandToString
var UnzipZlib = tdxproto.UnzipZlib
var TransactMessageSync = tdxproto.TransactMessageSync

// Keep internal references working
var command_to_string = tdxproto.CommandToString
var readResponseHeader = tdxproto.ReadResponseHeader
var unzipZlib = tdxproto.UnzipZlib
