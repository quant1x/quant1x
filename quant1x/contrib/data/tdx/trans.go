package tdx

import (
	"encoding/csv"
	"fmt"
	"os"
	"path/filepath"
	"sync"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/level1/std"
	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data/schema"
	"github.com/quant1x/quant1x/quant1x/encoding"
	logger "github.com/quant1x/quant1x/quant1x/log"
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
	historicalTradingDataBegin data.Timestamp
)

func initHistoricalTradingData() {
	ts, err := data.ParseTimestamp(defaultTrainsBeginDate)
	if err != nil {
		historicalTradingDataBegin = data.ZeroTimestamp()
		return
	}
	historicalTradingDataBegin = ts
}

// getBeginDateOfHistoricalTradingData 返回配置的历史逐笔数据的最早日期.
// 该值为惰性初始化, 来源于 defaultTrainsBeginDate.
func getBeginDateOfHistoricalTradingData() data.Timestamp {
	historicalTradingDataOnce.Do(initHistoricalTradingData)
	return historicalTradingDataBegin
}

// updateBeginDateOfHistoricalTradingData 以线程安全的方式更新起始日期.
func updateBeginDateOfHistoricalTradingData(date string) {
	// ensure initialized
	_ = getBeginDateOfHistoricalTradingData()
	historicalTradingDataMutex.Lock()
	defer historicalTradingDataMutex.Unlock()
	if ts, err := data.ParseTimestamp(date); err == nil {
		historicalTradingDataBegin = ts
	}
}

// restoreBeginDateOfHistoricalTradingData 将起始日期恢复为默认值.
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

// loadTransactionDataFromCache 从 CSV 缓存读取逐笔数据并返回数据列表及起始时间字符串.
func loadTransactionDataFromCache(instrument data.InstrumentInfo, featureDate data.Timestamp, ignorePreviousData bool) ([]schema.Transaction, string) {
	list := make([]schema.Transaction, 0)

	if ignorePreviousData {
		startDate := getBeginDateOfHistoricalTradingData()
		if featureDate.YYYYMMDD() < startDate.YYYYMMDD() {
			// 无数据
			return list, HistoricalTransactionDataFirstTime
		}
	}

	startTime := HistoricalTransactionDataFirstTime
	correctedCode := instrument.Symbol()
	logger.Debugf("loading transaction data from cache for %s on %s", correctedCode, featureDate.OnlyDate())
	filename := config.GetHistoricalTradeFilename(correctedCode, featureDate.OnlyDate())

	err := encoding.CsvToSlices(filename, &list)
	if err != nil {
		return list, startTime
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

	return list, startTime
}

// updateTransactionData 从 level1 拉取逐笔数据并写入合并后的 CSV 缓存.
func updateTransactionData(instrument data.InstrumentInfo, featureDate data.Timestamp, startTime string) {
	tradeDate := featureDate.YYYYMMDD()
	todayIsLastTradingDate := featureDate.IsSameDate(data.NowTimestamp())
	offset := int(std.TickTransactionPerRequestMax)
	start := 0
	history := make([]schema.Transaction, 0)
	hs := make([][]schema.Transaction, 0)
	u32Date := uint32(tradeDate)
	conn, release, err := GetStdConnection()
	if err != nil {
		logger.Errorf("level1 client acquire failed: %v", err)
		return
	}
	if release != nil {
		defer release()
	}
	if conn == nil || conn.Conn() == nil {
		logger.Errorf("nil connection from level1 client")
		return
	}
	for {
		var reply *std.TransactionReply
		if todayIsLastTradingDate {
			msg := std.NewTransactionContext(instrument, start, offset)
			if err := tdxproto.TransactMessageSync(conn, msg); err != nil {
				logger.Errorf("[tdx::trans] code=%s, tradeDate=%d, error=%v", instrument.Symbol(), tradeDate, err)
				break
			}
			if msg.Reply.Count == 0 || len(msg.Reply.List) == 0 {
				break
			}
			reply = &msg.Reply
		} else {
			msg := std.NewHistoryTransactionContext(instrument, u32Date, start, offset)
			if err := tdxproto.TransactMessageSync(conn, msg); err != nil {
				logger.Errorf("[tdx::trans] code=%s, tradeDate=%d, error=%v", instrument.Symbol(), tradeDate, err)
				break
			}
			if msg.Reply.Count == 0 || len(msg.Reply.List) == 0 {
				break
			}
			reply = &msg.Reply
		}
		var incremental []schema.Transaction // 临时存储本次请求的数据
		var incrementalCount int             // 记录增量数据数量
		tmpList := std.Reverse(reply.List)
		for _, td := range tmpList {
			if td.Time >= startTime {
				incrementalCount += 1
				incremental = append(incremental, schema.Transaction{
					Time:      td.Time,
					Price:     td.Price,
					Volume:    td.Vol,
					Num:       td.Num,
					Amount:    td.Amount,
					Direction: td.Direction,
				})

			}
		}
		incremental = std.Reverse(incremental)
		hs = append(hs, incremental)

		if len(incremental) < offset {
			break
		}
		start += offset
	}

	// 将分段数据反转并展开(服务器返回最新到最旧)
	for i := len(hs) - 1; i >= 0; i-- {
		history = append(history, hs[i]...)
	}

	if len(history) == 0 {
		return
	}
	correctedCode := instrument.Symbol()
	// 与现有缓存合并
	existingList, _ := loadTransactionDataFromCache(instrument, featureDate, false)
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
			fmt.Sprintf("%d", rec.Volume),
			fmt.Sprintf("%d", rec.Num),
			fmt.Sprintf("%g", rec.Amount),
			fmt.Sprintf("%d", rec.Direction),
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

func ensureTransactionDataUpdated(instrument data.InstrumentInfo, featureDate data.Timestamp, ignorePreviousData bool) {
	list, startTime := loadTransactionDataFromCache(instrument, featureDate, ignorePreviousData)
	needsUpdate := len(list) == 0 || (list[len(list)-1].Time != HistoricalTransactionDataLastTime)
	if needsUpdate {
		updateTransactionData(instrument, featureDate, startTime)
	}
}

// CheckoutTransactionData 导出: 检出指定日期的逐笔成交数据
func CheckoutTransactionData(instrument data.InstrumentInfo, featureDate data.Timestamp, ignorePreviousData bool) []data.Transaction {
	ensureTransactionDataUpdated(instrument, featureDate, ignorePreviousData)
	list, _ := loadTransactionDataFromCache(instrument, featureDate, ignorePreviousData)
	return list
}

// CountInflow 计算成交额/成交量汇总, 行为与 C++ 实现相似.
func CountInflow(list []std.TickTransaction, securityCode string, featureDate data.Timestamp) TurnoverDataSummary {
	summary := TurnoverDataSummary{}
	if len(list) == 0 {
		return summary
	}
	//correctedCode := data.CorrectSecurityCode(securityCode)
	var lastPrice float64
	for _, v := range list {
		tm := v.Time
		direction := v.Direction
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

	// F10 尚未在此模块移植到 Go, 保留 TurnZ 为零.
	//_ = correctedCode
	_ = featureDate
	return summary
}

// DataTrans 实现了缓存适配器风格的更新器
type DataTrans struct{}

func (d *DataTrans) Kind() data.Kind { return data.BaseTransaction }
func (d *DataTrans) Owner() string   { return data.DefaultDataProvider }
func (d *DataTrans) Key() string     { return "trans" }
func (d *DataTrans) Name() string    { return "逐笔成交" }
func (d *DataTrans) Usage() string   { return "" }

func (d *DataTrans) Print(instrument data.InstrumentInfo, dates ...data.Timestamp) {
	_ = instrument
	_ = dates
}

func (d *DataTrans) Update(instrument data.InstrumentInfo, date data.Timestamp) {
	ensureTransactionDataUpdated(instrument, date, false)
}

func init() {
	// 注册到 data 插件中心, 容错处理重复注册
	_ = data.Register(&DataTrans{})
}
