// Copyright (c) Quant1X <wangfengxy@sina.cn>.
// Licensed under the MIT License.

package meta

import (
	"fmt"
	"testing"
	"time"
)

// TestSessionMain 对应 Python 代码的 __main__ 部分
func TestSessionMain(t *testing.T) {
	// 打印当前日期时间
	dt := time.Now().Format("2006-01-02 15:04:05")
	fmt.Println(dt)

	// 获取美股交易时段
	session := LatestSessionByExchange(USA)
	fmt.Printf("Earliest: %v, Latest: %v, Closing: %v\n",
		session.EarliestStart.OnlyTime(),
		session.LatestEnd.OnlyTime(),
		session.ClosingTime.OnlyTime())
	fmt.Printf("Trading minutes: %d\n", session.GetTradingMinutes())

	// 测试时间列表
	testTimes := []string{"09:00:00", "09:16:00", "09:22:00", "09:28:00", "09:35:00", "12:00:00", "13:30:00", "14:58:00", "15:01:00"}
	for _, tStr := range testTimes {
		status := session.CheckStatus(tStr)
		ts, err := ParseTimeOnly(tStr)
		if err != nil {
			t.Errorf("ParseTimeOnly(%s) error: %v", tStr, err)
			continue
		}
		fmt.Printf("%s -> %v -> %s\n", tStr, ts.OnlyTime(), ts.OnlyTime())
		fmt.Printf("elapsed: %s -> %d, trading: %v\n", tStr, session.Minutes(ts), session.IsTrading(ts))
		fmt.Printf("Time: %s, Status: %d, Active: %v, Trading: %v\n",
			tStr, status, status.IsMarketActive(), status.IsContinuousTrading())
	}
}
