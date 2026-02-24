// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package data

import (
	"fmt"
)

const (
	// PreMarketHour 盘前小时 (9点)
	PreMarketHour = 9
	// PreMarketMinute 盘前分钟 (0分)
	PreMarketMinute = 0
	// PreMarketSecond 盘前秒 (0秒)
	PreMarketSecond = 0
)

// CnCronExprDailyInit 每日初始化 cron 表达式
// 格式: 秒 分 时 日 月 周
// 对应 Python: cn_cron_expr_daily_init = f"0 {PRE_MARKET_HOUR} {PRE_MARKET_MINUTE} * * *"
var CnCronExprDailyInit = fmt.Sprintf("0 %d %d * * *", PreMarketMinute, PreMarketHour)
