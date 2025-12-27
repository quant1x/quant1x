package factors

import (
	"fmt"
	"sort"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/datasets"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

const DateLayout = "2006-01-02"

type CumulativeAdjustment struct {
	Timestamp            exchange.Timestamp
	M                    float64
	A                    float64
	MonetaryAdjustment   float64
	ShareAdjustmentRatio float64
	No                   int
}

func (c CumulativeAdjustment) ToString() string {
	return fmt.Sprintf("{no=%d,timestamp=%s,m=%.6f,a=%.6f,monetaryAdjustment=%.6f,shareAdjRatio=%.6f}",
		c.No, c.Timestamp.OnlyDate(), c.M, c.A, c.MonetaryAdjustment, c.ShareAdjustmentRatio)
}

func (c CumulativeAdjustment) Apply(price float64) float64 {
	return price*c.M + c.A
}

func (c CumulativeAdjustment) Inverse(adjustedPrice float64) float64 {
	return (adjustedPrice - c.A) / c.M
}

// CheckKlineOffset 检查K线数据中指定日期的偏移量
// 参数:
//
//	klines: K线数据数组
//	date: 要查找的目标日期字符串
//
// 返回值:
//
//	如果找到目标日期，返回其在数组中的偏移量(从末尾开始计数)
//	如果目标日期不存在或比所有K线日期都早，返回-1
func CheckKlineOffset(klines []datasets.KLineRaw, date string) int {
	rows := len(klines)
	offset := 0
	for i := 0; i < rows; i++ {
		klineDate := klines[rows-1-i].Date
		if klineDate < date {
			return -1
		} else if klineDate == date {
			break
		} else {
			offset++
		}
	}
	if offset+1 >= rows {
		return -1
	}
	return offset
}

func IpoDateFromXdxrs(xdxrList []datasets.XdxrInfo) *string {
	for _, v := range xdxrList {
		if v.Category != 5 {
			continue
		}
		if v.QianLiuTong == 0 && v.QianZongGuBen == 0 && v.HouLiuTong > 0 && v.HouZongGuBen > 0 {
			return &v.Date
		}
	}
	return nil
}

func CombineAdjustmentsInPeriod(xdxrList []datasets.XdxrInfo, startDate, endDate exchange.Timestamp) []CumulativeAdjustment {
	result := []CumulativeAdjustment{}

	for _, info := range xdxrList {
		if info.Category == 5 {
			continue
		}

		eventDate, _ := time.Parse(DateLayout, info.Date)
		eventTs := exchange.PreMarketTimestamp(eventDate.Year(), int(eventDate.Month()), eventDate.Day())
		if eventTs.Less(startDate) || eventTs.Greater(endDate) {
			continue
		}

		m, a := info.AdjustFactor()
		eventMonetaryAdjustment := info.ComputeMonetaryAdjustment()
		eventShareAdjustmentRatio := info.ComputeShareAdjustmentRatio()

		for i := range result {
			factor := &result[i]
			factor.M *= m
			factor.A = m*factor.A + a
			factor.No += 1

			oldMonetaryAdjustment := factor.MonetaryAdjustment
			oldShareAdjustmentRatio := factor.ShareAdjustmentRatio

			newShareAdjustmentRatio := oldShareAdjustmentRatio + eventShareAdjustmentRatio + oldShareAdjustmentRatio*eventShareAdjustmentRatio
			newMonetaryAdjustment := oldMonetaryAdjustment + eventMonetaryAdjustment*(1.0+oldShareAdjustmentRatio)

			factor.MonetaryAdjustment = newMonetaryAdjustment
			factor.ShareAdjustmentRatio = newShareAdjustmentRatio
		}

		entry := CumulativeAdjustment{
			Timestamp:            eventTs,
			M:                    m,
			A:                    a,
			MonetaryAdjustment:   eventMonetaryAdjustment,
			ShareAdjustmentRatio: eventShareAdjustmentRatio,
			No:                   1,
		}
		result = append(result, entry)
	}

	return result
}

func ApplyForwardAdjustmentIncrementally(klines []*datasets.KLine, xdxrList []datasets.XdxrInfo, lastAdjustedDate, asOfDate exchange.Timestamp, truncateToAsOfDate bool) {
	if len(klines) == 0 {
		return
	}

	tsStart := lastAdjustedDate
	tsEnd := asOfDate
	factors := CombineAdjustmentsInPeriod(xdxrList, tsStart, tsEnd)

	if len(factors) == 0 {
		return
	}

	factorsCount := len(factors)
	i := 0
	rows := 0
	klinesCount := len(klines)

	for idx := 0; idx < klinesCount; idx++ {
		kline := klines[idx]
		currentDateDt, _ := time.Parse(DateLayout, kline.Date)
		currentDate := exchange.PreMarketTimestamp(currentDateDt.Year(), int(currentDateDt.Month()), currentDateDt.Day())

		if i < factorsCount {
			factor := factors[i]

			if currentDate.Greater(tsEnd) {
				break
			}

			for i+1 < factorsCount && currentDate.GreaterOrEqual(factor.Timestamp) {
				i++
				factor = factors[i]
			}

			if currentDate.Less(factor.Timestamp) {
				adj := datasets.CumulativeAdjustment{
					M:                    factor.M,
					A:                    factor.A,
					ShareAdjustmentRatio: factor.ShareAdjustmentRatio,
					No:                   factor.No,
				}
				kline.Adjust(adj)
			} else if !truncateToAsOfDate {
				break
			}
		}

		rows++
	}

	if truncateToAsOfDate {
		klines = klines[:rows]
	}
}

func CalculatePreAdjust(klines []*datasets.KLine, xdxrList []datasets.XdxrInfo) {
	if len(klines) == 0 {
		return
	}

	startDate, _ := time.Parse(DateLayout, klines[0].Date)
	endDate, _ := time.Parse(DateLayout, klines[len(klines)-1].Date)
	startTs := exchange.PreMarketTimestamp(startDate.Year(), int(startDate.Month()), startDate.Day())
	endTs := exchange.PreMarketTimestamp(endDate.Year(), int(endDate.Month()), endDate.Day())
	ApplyForwardAdjustmentIncrementally(klines, xdxrList, startTs, endTs, true)
}

func GetCrossSectionForwardAdjustedKlines(securityCode, asOfDate string) []*datasets.KLine {
	correctedCode := exchange.CorrectSecurityCode(securityCode)
	ts, _ := exchange.ParseTimestamp(asOfDate)
	fixedDate := ts.OnlyDate()

	rawKlines, err := datasets.LoadKlineRaw(correctedCode)
	if err != nil || len(rawKlines) == 0 {
		return []*datasets.KLine{}
	}

	lastKline := rawKlines[len(rawKlines)-1]
	if lastKline.Date < fixedDate {
		rawKlines, err = datasets.LoadKlineRaw(correctedCode)
		if err != nil {
			return []*datasets.KLine{}
		}
	}

	offset := CheckKlineOffset(rawKlines, fixedDate)
	if offset < 0 {
		return []*datasets.KLine{}
	}

	fixedCount := len(rawKlines) - offset
	filteredKlines := rawKlines[:fixedCount]

	if len(filteredKlines) == 0 {
		return []*datasets.KLine{}
	}

	klines := []*datasets.KLine{}
	for _, rawKline := range filteredKlines {
		kline := &datasets.KLine{
			Date:            rawKline.Date,
			Open:            rawKline.Open,
			Close:           rawKline.Close,
			High:            rawKline.High,
			Low:             rawKline.Low,
			Volume:          rawKline.Volume,
			Amount:          rawKline.Amount,
			Up:              rawKline.Up,
			Down:            rawKline.Down,
			Datetime:        rawKline.Datetime,
			AdjustmentCount: 0,
		}
		klines = append(klines, kline)
	}

	xdxrList, err := datasets.LoadXdxr(correctedCode)
	if err != nil {
		return klines // return unadjusted if no xdxr
	}

	sort.Slice(xdxrList, func(i, j int) bool {
		dateI, _ := time.Parse(DateLayout, xdxrList[i].Date)
		dateJ, _ := time.Parse(DateLayout, xdxrList[j].Date)
		return dateI.Before(dateJ)
	})

	startDate, _ := time.Parse(DateLayout, klines[0].Date)
	endDate, _ := time.Parse(DateLayout, klines[len(klines)-1].Date)
	startTs := exchange.PreMarketTimestamp(startDate.Year(), int(startDate.Month()), startDate.Day())
	endTs := exchange.PreMarketTimestamp(endDate.Year(), int(endDate.Month()), endDate.Day())

	ApplyForwardAdjustmentIncrementally(klines, xdxrList, startTs, endTs, true)

	return klines
}
