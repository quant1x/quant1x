package config

import (
	"fmt"
)

const (
	Cn_pre_market_hour   = 9 ///< 盘前9点
	Cn_pre_market_minute = 0 ///< 盘点9点0分
	Cn_pre_market_second = 0 ///< 盘点9点0分0秒
)

var (
	// 每天9点整
	CronExprDaily9am = fmt.Sprintf("0 %d %d * * *", Cn_pre_market_minute, Cn_pre_market_hour)
)
