package data

import (
	"encoding/csv"
	"fmt"
	"log"
	"os"
	"strconv"

	"gitee.com/quant1x/quant1x/quant1x/config"
)

// KLineRaw 对应 C++ 中的 data::KLineRaw，用于表示原始日线数据。
type KLineRaw struct {
	Date     string  `name:"日期" csv:"date"`        // 日期 YYYY-MM-DD
	Open     float64 `name:"开盘价" csv:"open"`       // 开盘价
	Close    float64 `name:"收盘价" csv:"close"`      // 收盘价
	High     float64 `name:"最高价" csv:"high"`       // 最高价
	Low      float64 `name:"最低价" csv:"low"`        // 最低价
	Volume   float64 `name:"成交量(股)" csv:"volume"`  // 成交量(股)
	Amount   float64 `name:"成交额(元)" csv:"amount"`  // 成交额(元)
	Up       int     `name:"上涨家数 / 外盘" csv:"up"`   // 上涨家数 / 外盘
	Down     int     `name:"下跌家数 / 内盘" csv:"down"` // 下跌家数 / 内盘
	Datetime string  `name:"时间" csv:"datetime"`    // 时间
}

//const maxKlineRawLookbackDays = 1

// SaveKlineRaw 将原始 K 线写入 CSV 文件（包含表头）。
func SaveKlineRaw(filename string, values []KLineRaw) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()

	header := []string{"date", "open", "close", "high", "low", "volume", "amount", "up", "down", "datetime"}
	if err := w.Write(header); err != nil {
		return err
	}
	for _, r := range values {
		row := []string{
			r.Date,
			fmt.Sprintf("%g", r.Open),
			fmt.Sprintf("%g", r.Close),
			fmt.Sprintf("%g", r.High),
			fmt.Sprintf("%g", r.Low),
			fmt.Sprintf("%g", r.Volume),
			fmt.Sprintf("%g", r.Amount),
			fmt.Sprintf("%d", r.Up),
			fmt.Sprintf("%d", r.Down),
			r.Datetime,
		}
		if err := w.Write(row); err != nil {
			return err
		}
	}
	return nil
}

// ReadKlineRawFromCSV 从 CSV 文件中读取 KLineRaw 条目；解析错误会返回错误。
func ReadKlineRawFromCSV(filename string) ([]KLineRaw, error) {
	f, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	r := csv.NewReader(f)
	rows, err := r.ReadAll()
	if err != nil {
		return nil, err
	}
	if len(rows) == 0 {
		return nil, nil
	}
	// 期望第一行为表头
	out := make([]KLineRaw, 0, len(rows)-1)
	for i := 1; i < len(rows); i++ {
		rec := rows[i]
		if len(rec) < 10 {
			continue
		}
		open, _ := strconv.ParseFloat(rec[1], 64)
		closev, _ := strconv.ParseFloat(rec[2], 64)
		high, _ := strconv.ParseFloat(rec[3], 64)
		low, _ := strconv.ParseFloat(rec[4], 64)
		vol, _ := strconv.ParseFloat(rec[5], 64)
		amt, _ := strconv.ParseFloat(rec[6], 64)
		up, _ := strconv.Atoi(rec[7])
		down, _ := strconv.Atoi(rec[8])
		datetime := rec[9]
		k := KLineRaw{
			Date:     rec[0],
			Open:     open,
			Close:    closev,
			High:     high,
			Low:      low,
			Volume:   vol,
			Amount:   amt,
			Up:       up,
			Down:     down,
			Datetime: datetime,
		}
		out = append(out, k)
	}
	return out, nil
}

// LoadKlineRaw 从缓存文件加载原始K线数据。
func LoadKlineRaw(code string) ([]KLineRaw, error) {
	filename := config.GetKlineFilename(code, false)
	log.Printf("[data::KLineRaw] kline file: %s", filename)
	return ReadKlineRawFromCSV(filename)
}
