package datasets

import (
	"encoding/csv"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"sync"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
	"gitee.com/quant1x/quant1x/quant1x/std"
)

const (
	HistoricalTransactionDataFirstTime        = "09:25"
	HistoricalTransactionDataStartTime        = "09:30"
	HistoricalTransactionDataFinalBiddingTime = "14:57"
	HistoricalTransactionDataLastTime         = "15:00"
)

const defaultTrainsBeginDate = "2024-10-01"

var (
	historicalTradingDataOnce  sync.Once
	historicalTradingDataMutex sync.Mutex
	historicalTradingDataBegin exchange.Timestamp
)

func initHistoricalTradingData() {
	ts, err := exchange.ParseTimestamp(defaultTrainsBeginDate)
	if err != nil {
		historicalTradingDataBegin = exchange.ZeroTimestamp()
		return
	}
	historicalTradingDataBegin = ts
}

// getBeginDateOfHistoricalTradingData 返回配置的历史逐笔数据的最早日期。
// 该值为惰性初始化，来源于 defaultTrainsBeginDate。
func getBeginDateOfHistoricalTradingData() exchange.Timestamp {
	historicalTradingDataOnce.Do(initHistoricalTradingData)
	return historicalTradingDataBegin
}

// updateBeginDateOfHistoricalTradingData 以线程安全的方式更新起始日期。
func updateBeginDateOfHistoricalTradingData(date string) {
	// ensure initialized
	_ = getBeginDateOfHistoricalTradingData()
	historicalTradingDataMutex.Lock()
	defer historicalTradingDataMutex.Unlock()
	if ts, err := exchange.ParseTimestamp(date); err == nil {
		historicalTradingDataBegin = ts
	}
}

// restoreBeginDateOfHistoricalTradingData 将起始日期恢复为默认值。
func restoreBeginDateOfHistoricalTradingData() {
	updateBeginDateOfHistoricalTradingData(defaultTrainsBeginDate)
}

// TurnoverDataSummary 对应 C++ 中的 datasets::TurnoverDataSummary
type TurnoverDataSummary struct {
	OuterVolume int64   `csv:"outer_volume"`
	OuterAmount float64 `csv:"outer_amount"`
	InnerVolume int64   `csv:"inner_volume"`
	InnerAmount float64 `csv:"inner_amount"`
	OpenVolume  int64   `csv:"open_volume"`
	OpenTurnZ   float64 `csv:"open_turn_z"`
	CloseVolume int64   `csv:"close_volume"`
	CloseTurnZ  float64 `csv:"close_turn_z"`
}

// loadTransactionDataFromCache 从 CSV 缓存读取逐笔数据并返回数据列表及起始时间字符串。
func loadTransactionDataFromCache(correctedCode string, featureDate exchange.Timestamp, ignorePreviousData bool) ([]level1.TickTransaction, string) {
	list := make([]level1.TickTransaction, 0)
	//tradeDate := featureDate.YYYYMMDD()

	if ignorePreviousData {
		startDate := getBeginDateOfHistoricalTradingData()
		if featureDate.YYYYMMDD() < startDate.YYYYMMDD() {
			// 无数据
			return list, HistoricalTransactionDataFirstTime
		}
	}

	startTime := HistoricalTransactionDataFirstTime
	filename := config.GetHistoricalTradeFilename(correctedCode, featureDate.OnlyDate())

	if _, err := os.Stat(filename); err == nil {
		f, err := os.Open(filename)
		if err == nil {
			defer f.Close()
			r := csv.NewReader(f)
			rows, err := r.ReadAll()
			if err == nil && len(rows) > 0 {
				// 期望第一行为表头
				for i := 1; i < len(rows); i++ {
					rec := rows[i]
					// 确保至少 6 列
					for len(rec) < 6 {
						rec = append(rec, "")
					}
					price, _ := strconv.ParseFloat(rec[1], 64)
					vol, _ := strconv.ParseInt(rec[2], 10, 64)
					num, _ := strconv.ParseInt(rec[3], 10, 64)
					amount, _ := strconv.ParseFloat(rec[4], 64)
					buyOrSell, _ := strconv.ParseInt(rec[5], 10, 64)
					list = append(list, level1.TickTransaction{
						Time:      rec[0],
						Price:     price,
						Vol:       vol,
						Num:       num,
						Amount:    amount,
						BuyOrSell: buyOrSell,
					})
				}

				if len(list) > 0 {
					lastTime := list[len(list)-1].Time
					if lastTime == HistoricalTransactionDataLastTime {
						return list, startTime
					}

					// 从尾部扫描以确定 startTime 并截取已缓存的尾部重复部分
					cacheLength := len(list)
					firstTime := ""
					skipCount := 0
					for i := 1; i <= cacheLength; i++ {
						tm := list[cacheLength-i].Time
						if firstTime == "" {
							firstTime = tm
							startTime = firstTime
							skipCount++
							continue
						}
						if tm < firstTime {
							startTime = firstTime
							break
						} else {
							skipCount++
						}
					}
					if skipCount > 0 {
						list = list[:cacheLength-skipCount]
					}
				}
			}
		}
	}

	return list, startTime
}

// updateTransactionData 从 level1 拉取逐笔数据并写入合并后的 CSV 缓存。
func updateTransactionData(correctedCode string, featureDate exchange.Timestamp, startTime string) {
	tradeDate := featureDate.YYYYMMDD()
	todayIsLastTradingDate := featureDate.IsSameDate(exchange.NowTimestamp())
	offset := int(level1.TickTransactionPerRequestMax)
	start := 0
	history := make([]level1.TickTransaction, 0)
	hs := make([][]level1.TickTransaction, 0)
	marketId, _, pureCode, _ := exchange.DetectMarket(correctedCode)

	if todayIsLastTradingDate {
		for {
			req := level1.NewTransactionRequest(correctedCode, start, offset)
			resp := level1.NewTransactionResponse(int(marketId), pureCode)
			conn, release, err := level1.GetStdConnection()
			if err != nil {
				fmt.Printf("level1 client acquire failed: %v\n", err)
				break
			}
			if release != nil {
				defer release()
			}
			if conn == nil || conn.Conn() == nil {
				fmt.Printf("nil connection from level1 client\n")
				break
			}
			if err := level1.Process(conn, req, resp); err != nil {
				fmt.Printf("[dataset::trans] code=%s, tradeDate=%d, error=%v\n", correctedCode, tradeDate, err)
				break
			}
			if resp.Count == 0 || len(resp.List) == 0 {
				break
			}
			var tmp level1.TransactionResponse
			tmpList := std.Reverse(resp.List)
			for _, td := range tmpList {
				if td.Time >= startTime {
					tmp.Count += 1
					tmp.List = append(tmp.List, td)
				}
			}
			tmp.List = std.Reverse(tmp.List)
			hs = append(hs, tmp.List)
			if len(tmp.List) < offset {
				break
			}
			start += offset
		}
	} else {
		u32Date := uint32(tradeDate)
		for {
			req := level1.NewHistoryTransactionRequest(correctedCode, u32Date, start, offset)
			resp := level1.NewHistoryTransactionResponse(int(marketId), pureCode)
			conn, release, err := level1.GetStdConnection()
			if err != nil {
				fmt.Printf("level1 client acquire failed: %v\n", err)
				break
			}
			if release != nil {
				defer release()
			}
			if conn == nil || conn.Conn() == nil {
				fmt.Printf("nil connection from level1 client\n")
				break
			}
			if err := level1.Process(conn, req, resp); err != nil {
				fmt.Printf("[dataset::trans] code=%s, tradeDate=%d, error=%v\n", correctedCode, tradeDate, err)
				break
			}
			if resp.Count == 0 || len(resp.List) == 0 {
				break
			}
			var tmp level1.TransactionResponse
			tmpList := std.Reverse(resp.List)
			for _, td := range tmpList {
				if td.Time >= startTime {
					tmp.Count += 1
					tmp.List = append(tmp.List, td)
				}
			}
			tmp.List = std.Reverse(tmp.List)
			hs = append(hs, tmp.List)
			if len(tmp.List) < offset {
				break
			}
			start += offset
		}
	}

	// 将分段数据反转并展开（服务器返回最新到最旧）
	for i := len(hs) - 1; i >= 0; i-- {
		history = append(history, hs[i]...)
	}

	if len(history) == 0 {
		return
	}

	// 与现有缓存合并
	existingList, _ := loadTransactionDataFromCache(correctedCode, featureDate, false)
	existingList = append(existingList, history...)

	filename := config.GetHistoricalTradeFilename(correctedCode, featureDate.OnlyDate())
	tmp := filename + ".tmp"
	if err := os.MkdirAll(filepath.Dir(tmp), 0o755); err != nil {
		// 忽略创建目录错误
	}
	f, err := os.Create(tmp)
	if err != nil {
		fmt.Printf("[dataset::trans] create tmp failed: %v\n", err)
		return
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()
	_ = w.Write([]string{"time", "price", "volume", "number", "amount", "buy_or_sell"})
	for _, rec := range existingList {
		_ = w.Write([]string{
			rec.Time,
			fmt.Sprintf("%g", rec.Price),
			fmt.Sprintf("%d", rec.Vol),
			fmt.Sprintf("%d", rec.Num),
			fmt.Sprintf("%g", rec.Amount),
			fmt.Sprintf("%d", rec.BuyOrSell),
		})
	}
	w.Flush()
	if err := w.Error(); err != nil {
		fmt.Printf("[dataset::trans] csv write failed: %v\n", err)
		return
	}
	f.Close()
	if err := os.Rename(tmp, filename); err != nil {
		fmt.Printf("[dataset::trans] rename failed: %v\n", err)
		_ = os.Remove(tmp)
	}
}

func ensureTransactionDataUpdated(correctedCode string, featureDate exchange.Timestamp, ignorePreviousData bool) {
	list, startTime := loadTransactionDataFromCache(correctedCode, featureDate, ignorePreviousData)
	needsUpdate := len(list) == 0 || (list[len(list)-1].Time != HistoricalTransactionDataLastTime)
	if needsUpdate {
		updateTransactionData(correctedCode, featureDate, startTime)
	}
}

// CheckoutTransactionData 导出：检出指定日期的逐笔成交数据
func CheckoutTransactionData(securityCode string, featureDate exchange.Timestamp, ignorePreviousData bool) []level1.TickTransaction {
	correctedCode := exchange.CorrectSecurityCode(securityCode)
	ensureTransactionDataUpdated(correctedCode, featureDate, ignorePreviousData)
	list, _ := loadTransactionDataFromCache(correctedCode, featureDate, ignorePreviousData)
	return list
}

// CountInflow 计算成交额/成交量汇总，行为与 C++ 实现相似。
func CountInflow(list []level1.TickTransaction, securityCode string, featureDate exchange.Timestamp) TurnoverDataSummary {
	summary := TurnoverDataSummary{}
	if len(list) == 0 {
		return summary
	}
	correctedCode := exchange.CorrectSecurityCode(securityCode)
	var lastPrice float64
	for _, v := range list {
		tm := v.Time
		direction := v.BuyOrSell
		price := v.Price
		if lastPrice == 0 {
			lastPrice = price
		}
		vol := v.Vol
		if direction != 0 && direction != 1 { // 未知类型
			if price > lastPrice {
				direction = 0
			} else if price < lastPrice {
				direction = 1
			}
		}
		if direction == 0 {
			summary.OuterVolume += vol
			summary.OuterAmount += float64(vol) * price
		} else if direction == 1 {
			summary.InnerVolume += vol
			summary.InnerAmount += float64(vol) * price
		} else {
			vn := vol
			buyOffset := vn / 2
			sellOffset := vn - buyOffset
			summary.OuterVolume += buyOffset
			summary.OuterAmount += float64(buyOffset) * price
			summary.InnerVolume += sellOffset
			summary.InnerAmount += float64(sellOffset) * price
		}

		if tm >= HistoricalTransactionDataFirstTime && tm < HistoricalTransactionDataStartTime {
			summary.OpenVolume += vol
		}
		if tm > HistoricalTransactionDataFinalBiddingTime && tm <= HistoricalTransactionDataLastTime {
			summary.CloseVolume += vol
		}
		lastPrice = price
	}

	// F10 尚未在此模块移植到 Go，保留 TurnZ 为零。
	_ = correctedCode
	_ = featureDate
	return summary
}

// DataTrans 实现了缓存适配器风格的更新器
type DataTrans struct{}

func (d *DataTrans) Kind() data.Kind { return BaseTransaction }
func (d *DataTrans) Owner() string   { return data.DefaultDataProvider }
func (d *DataTrans) Key() string     { return "trans" }
func (d *DataTrans) Name() string    { return "逐笔成交" }
func (d *DataTrans) Usage() string   { return "" }

func (d *DataTrans) Print(code string, dates ...exchange.Timestamp) {
	_ = code
	_ = dates
}

func (d *DataTrans) Update(code string, date exchange.Timestamp) {
	correctedCode := exchange.CorrectSecurityCode(code)
	ensureTransactionDataUpdated(correctedCode, date, false)
}

func init() {
	// 注册到 data 插件中心，容错处理重复注册
	_ = data.Register(&DataTrans{})
}
