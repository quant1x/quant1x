package provider

import (
	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/encoding"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
)

func GetTdxProvider() data.DataSource {
	return new(tdxProvider)
}

// tdxProvider 通达信数据提供者
type tdxProvider struct {
}

func (p *tdxProvider) GetF10(code string) (data.F10, error) {
	return data.F10{}, data.ErrNotImplemented
}

func (p *tdxProvider) GetKLines(code string, startDate, endDate, period string, adjust data.AdjustmentType) ([]data.KLine, error) {
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetKlineFilename(code, true)
	klines := []data.KLine{}
	err := encoding.CsvToSlices(cacheFilename, &klines)
	if err == nil && len(klines) > 0 {
		return klines, nil
	}
	// 2. 尝试更新缓存
	tdxUpdateKLine(code, exchange.NowTimestamp())
	// 3. 重新读取缓存文件
	klines = []data.KLine{}
	err = encoding.CsvToSlices(cacheFilename, &klines)
	if err == nil && len(klines) > 0 {
		return klines, nil
	}

	// 3. 缓存不存在则返回未实现错误
	return nil, data.ErrNotImplemented
}

func (p *tdxProvider) GetLatestKLines(code string, date string, count int, period string, adjust data.AdjustmentType) ([]data.KLine, error) {
	return nil, data.ErrNotImplemented
}

func (p *tdxProvider) GetTransactions(code string, date string) ([]data.Transaction, error) {
	return nil, data.ErrNotImplemented
}

func (p *tdxProvider) GetTradeTicks(code string, date string) ([]data.Transaction, error) {
	return nil, data.ErrNotImplemented
}

func (p *tdxProvider) GetTradeDetails(code string, date string) ([]data.Transaction, error) {
	return nil, data.ErrNotImplemented
}
