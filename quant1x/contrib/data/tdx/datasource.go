package tdx

import (
	_ "unsafe" // for go:linkname

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/data/exchange"
	"github.com/quant1x/quant1x/quant1x/data/schema"
	"github.com/quant1x/quant1x/quant1x/encoding"
)

//go:linkname GetTdxProvider github.com/quant1x/quant1x/quant1x/data.DataHandler
func GetTdxProvider() data.DataSource {
	return new(tdxProvider)
}

// tdxProvider 通达信数据提供者
type tdxProvider struct {
}

func (p *tdxProvider) GetF10(instrument string) (data.F10, error) {
	return data.F10{}, data.ErrNotImplemented
}

func (p *tdxProvider) GetKLines(instrument string, startDate, endDate string, frequency string, adjust ...data.AdjustmentType) ([]data.KLine, error) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetKlineFilename(instrument, true)
	klines := []schema.Bar{}
	err := encoding.CsvToSlices(cacheFilename, &klines)
	if err == nil && len(klines) > 0 {
		return klines, nil
	}
	// 2. 尝试更新缓存
	sc := exchange.DetectSymbol(instrument)
	tdxUpdateKLine(sc, exchange.NowTimestamp())
	// 3. 重新读取缓存文件
	klines = []schema.Bar{}
	err = encoding.CsvToSlices(cacheFilename, &klines)
	if err == nil && len(klines) > 0 {
		return klines, nil
	}

	// 3. 缓存不存在则返回未实现错误
	return nil, data.ErrNotImplemented
}

func (p *tdxProvider) GetTransactions(instrument string, date string) ([]schema.Transaction, error) {
	return p.GetTradeDetails(instrument, date)
}

func (p *tdxProvider) GetTradeTicks(instrument string, date string) ([]schema.Transaction, error) {
	return p.GetTradeDetails(instrument, date)
}

func (p *tdxProvider) GetTradeDetails(instrument string, date string) ([]schema.Transaction, error) {
	sc := exchange.DetectSymbol(instrument)
	ts, err := exchange.NewTimestampFromString(date)
	if err != nil {
		return nil, err
	}
	list := CheckoutTransactionData(sc, ts, true)
	return list, nil
}
