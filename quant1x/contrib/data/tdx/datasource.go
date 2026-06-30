package tdx

import (
	_ "unsafe" // for go:linkname

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/data"
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

func (p *tdxProvider) GetBars(instrument string, startDate, endDate string, frequency string, adjust ...data.AdjustmentType) ([]schema.Bar, error) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetBarFilename(instrument, true)
	bars := []schema.Bar{}
	err := encoding.CsvToSlices(cacheFilename, &bars)
	if err == nil && len(bars) > 0 {
		return bars, nil
	}
	// 2. 尝试更新缓存
	sc := data.DetectSymbol(instrument)
	tdxUpdateBar(sc, data.NowTimestamp())
	// 3. 重新读取缓存文件
	bars = []schema.Bar{}
	err = encoding.CsvToSlices(cacheFilename, &bars)
	if err == nil && len(bars) > 0 {
		return bars, nil
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
	sc := data.DetectSymbol(instrument)
	ts, err := data.NewTimestampFromString(date)
	if err != nil {
		return nil, err
	}
	list := CheckoutTransactionData(sc, ts, true)
	return list, nil
}
