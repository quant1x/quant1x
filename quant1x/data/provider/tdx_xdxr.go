package provider

import (
	"fmt"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/data"
	"gitee.com/quant1x/quant1x/quant1x/encoding"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
)

// UpdateXdxr 通过真实的 Level1 客户端从服务器获取除权除息数据，
// 并通过 config 包确定的文件名保存到本地缓存，然后返回加载的切片。
// 调用者无需注册 resolver；该函数会使用 config.GetXdxrFilename 生成路径。
// 该操作会发起真实的网络请求。
func tdxFetchXdxrList(sc exchange.InstrumentInfo) ([]data.XdxrInfo, error) {
	if sc.Type == exchange.SecurityTypeUnknown {
		return nil, fmt.Errorf("unknown security type for code %s", sc.String())
	}
	code := sc.String()
	// Compute filename via config package
	if len(code) != 8 {
		return nil, fmt.Errorf("invalid security code length: %s", code)
	}
	conn, release, err := level1.GetStdConnection()
	if err != nil {
		return nil, fmt.Errorf("level1 client acquire failed: %w", err)
	}
	if release != nil {
		defer release()
	}
	if conn == nil || conn.Conn() == nil {
		return nil, fmt.Errorf("nil connection from level1 client")
	}

	req := level1.XdxrInfoRequest{Code: sc}
	resp := level1.NewXdxrInfoResponse()
	if err := level1.Process(conn, req, resp); err != nil {
		return nil, fmt.Errorf("xdxr request failed: %w", err)
	}

	out := make([]data.XdxrInfo, 0, len(resp.List))
	for _, it := range resp.List {
		// level1.XdxrInfo -> datasets.XdxrInfo mapping
		xi := data.XdxrInfo{
			Date:          it.Date,
			Category:      int(it.Category),
			Name:          it.Name,
			FenHong:       it.FenHong,
			PeiGuJia:      it.PeiGuJia,
			SongZhuanGu:   it.SongZhuanGu,
			PeiGu:         it.PeiGu,
			SuoGu:         it.SuoGu,
			QianLiuTong:   it.QianLiuTong,
			HouLiuTong:    it.HouLiuTong,
			QianZongGuBen: it.QianZongGuBen,
			HouZongGuBen:  it.HouZongGuBen,
			FenShu:        it.FenShu,
			XingQuanJia:   it.XingQuanJia,
		}
		out = append(out, xi)
	}
	fname := config.GetXdxrFilename(code)
	err = encoding.SlicesToCsv(fname, out)
	return out, err
}

func tdxGetXdxrList(sc exchange.InstrumentInfo) ([]data.XdxrInfo, error) {
	if sc.Type == exchange.SecurityTypeUnknown {
		return nil, fmt.Errorf("unknown security type for code %s", sc.String())
	}
	// 1. 确定缓存文件并读取本地缓存
	cacheFilename := config.GetXdxrFilename(sc.String())
	var xdxr_list []data.XdxrInfo
	err := encoding.CsvToSlices(cacheFilename, &xdxr_list)
	if err == nil && len(xdxr_list) > 0 {
		return xdxr_list, nil
	}

	// 2. 尝试通过网络获取数据并更新缓存
	xdxr_list, err = tdxFetchXdxrList(sc)
	if err != nil {
		return nil, err
	}

	if len(xdxr_list) > 0 {
		return xdxr_list, nil
	}

	// 3. 缓存不存在则返回无数据错误
	return nil, data.ErrNoData
}

// DataXdxr 实现了 data.DataAdapter，用于 XDXR 数据，并在包初始化时注册到缓存插件中心。
type DataXdxr struct{}

func (d *DataXdxr) Kind() data.Kind { return data.BaseXdxr }
func (d *DataXdxr) Owner() string   { return data.DefaultDataProvider }
func (d *DataXdxr) Key() string     { return "xdxr" }
func (d *DataXdxr) Name() string    { return "除权除息" }
func (d *DataXdxr) Usage() string   { return "" }

func (d *DataXdxr) Print(code exchange.InstrumentInfo, dates ...exchange.Timestamp) {
	// No-op for now; could be extended to pretty-print loaded XDXR rows.
}

func (d *DataXdxr) Update(code exchange.InstrumentInfo, date exchange.Timestamp) {
	// Delegate to UpdateXdxr which fetches and writes CSV data.
	_, _ = tdxFetchXdxrList(code)
}

func init() {
	// Best-effort registration; ignore error if already registered.
	_ = data.Register(&DataXdxr{})
}
