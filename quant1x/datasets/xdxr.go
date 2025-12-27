package datasets

import (
	"encoding/csv"
	"fmt"
	"math"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/cache"
	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/level1"
)

// XdxrInfo 表示一条除权除息事件的 CSV 行，字段布局与 C++ 保持一致。
type XdxrInfo struct {
	Date          string  `name:"日期" csv:"date"`                 // 除权除息日期 YYYY-MM-DD
	Category      int     `name:"类别" csv:"category"`             // 事件类别
	Name          string  `name:"名称" csv:"name"`                 // 事件名称
	FenHong       float64 `name:"分红金额" csv:"fen_hong"`           // 分红金额
	PeiGuJia      float64 `name:"配股价格" csv:"pei_gu_jia"`         // 配股价格
	SongZhuanGu   float64 `name:"送转股数" csv:"song_zhuan_gu"`      // 送转股数
	PeiGu         float64 `name:"配股数" csv:"pei_gu"`              // 配股数
	SuoGu         float64 `name:"缩股数" csv:"suo_gu"`              // 缩股数
	QianLiuTong   float64 `name:"除权前流通股本" csv:"qian_liu_tong"`   // 除权前流通股本
	HouLiuTong    float64 `name:"除权后流通股本" csv:"hou_liu_tong"`    // 除权后流通股本
	QianZongGuBen float64 `name:"除权前总股本" csv:"qian_zong_gu_ben"` // 除权前总股本
	HouZongGuBen  float64 `name:"除权后总股本" csv:"hou_zong_gu_ben"`  // 除权后总股本
	FenShu        float64 `name:"份数" csv:"fen_shu"`              // 份数
	XingQuanJia   float64 `name:"行权价格" csv:"xing_quan_jia"`      // 行权价格
}

// ComputeShareAdjustmentRatio 对应 C++ 中的 XdxrInfo::computeShareAdjustmentRatio
func (x *XdxrInfo) ComputeShareAdjustmentRatio() float64 {
	return (x.SongZhuanGu + x.PeiGu - x.SuoGu + x.FenShu) / 10.0
}

// ComputeMonetaryAdjustment 对应 C++ 中的 XdxrInfo::computeMonetaryAdjustment
func (x *XdxrInfo) ComputeMonetaryAdjustment() float64 {
	return (x.PeiGu*x.PeiGuJia - x.FenHong + x.FenShu*x.XingQuanJia) / 10.0
}

// AdjustFactor 对应 C++ 中的 XdxrInfo::adjustFactor，返回 m 和 a
func (x *XdxrInfo) AdjustFactor() (float64, float64) {
	A := x.ComputeMonetaryAdjustment()
	B := x.ComputeShareAdjustmentRatio()
	if math.Abs(1.0+B) > 1e-10 {
		m := 1.0 / (1.0 + B)
		a := A * m
		return m, a
	} else {
		return 1.0, A
	}
}

// LoadXdxr 尝试定位并读取本地的 xdxr CSV 缓存文件。
// 会在若干常见位置查找（用户主目录下 .q1x、.q1x-rust，以及工程目录 ./xdxr）。
// 若未找到或解析失败，返回空切片并带错误。
func LoadXdxr(code string) ([]XdxrInfo, error) {
	if len(code) != 8 {
		return nil, fmt.Errorf("invalid security code length: %s", code)
	}
	fname := config.GetXdxrFilename(code)
	if _, err := os.Stat(fname); err != nil {
		return nil, err
	}
	f, err := os.Open(fname)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	r := csv.NewReader(f)
	rows, err := r.ReadAll()
	if err != nil {
		return nil, err
	}
	// expect header
	out := make([]XdxrInfo, 0, len(rows)-1)
	for i := 1; i < len(rows); i++ {
		rec := rows[i]
		// ensure at least 14 columns
		for len(rec) < 14 {
			rec = append(rec, "")
		}
		var xi XdxrInfo
		xi.Date = rec[0]
		if v, err := strconv.Atoi(rec[1]); err == nil {
			xi.Category = v
		}
		xi.Name = rec[2]
		xi.FenHong, _ = strconv.ParseFloat(rec[3], 64)
		xi.PeiGuJia, _ = strconv.ParseFloat(rec[4], 64)
		xi.SongZhuanGu, _ = strconv.ParseFloat(rec[5], 64)
		xi.PeiGu, _ = strconv.ParseFloat(rec[6], 64)
		xi.SuoGu, _ = strconv.ParseFloat(rec[7], 64)
		xi.QianLiuTong, _ = strconv.ParseFloat(rec[8], 64)
		xi.HouLiuTong, _ = strconv.ParseFloat(rec[9], 64)
		xi.QianZongGuBen, _ = strconv.ParseFloat(rec[10], 64)
		xi.HouZongGuBen, _ = strconv.ParseFloat(rec[11], 64)
		xi.FenShu, _ = strconv.ParseFloat(rec[12], 64)
		xi.XingQuanJia, _ = strconv.ParseFloat(rec[13], 64)
		out = append(out, xi)
	}
	return out, nil
}

// path helpers migrated to `config` package

// ApplyForwardAdjustmentForEvent 使用提供的除权除息事件对 K 线执行前复权处理。
// eventStartDate 是用于过滤 IPO 早期事件的起始日期（格式 YYYY-MM-DD）。
func ApplyForwardAdjustmentForEvent(klines []KLine, eventStartDate string, dividends []XdxrInfo) {
	if len(klines) == 0 {
		return
	}
	lastDay := klines[len(klines)-1].Date
	// compute next day (approximate next trading day)
	d, err := time.Parse("2006-01-02", lastDay)
	if err != nil {
		return
	}
	lastDayNext := d.Add(24 * time.Hour).Format("2006-01-02")

	// filter dividends: only include events where Date <= lastDayNext and Category == 1 (除权除息)
	infos := make([]XdxrInfo, 0, len(dividends))
	for _, v := range dividends {
		if v.Category == 1 && v.Date <= lastDayNext {
			infos = append(infos, v)
		}
	}
	// sort by date ascending (older events first)
	sort.Slice(infos, func(i, j int) bool { return infos[i].Date < infos[j].Date })

	startDate := eventStartDate
	for _, info := range infos {
		if info.Date <= startDate {
			// skip events before or on the start date
			continue
		}
		m, a := info.AdjustFactor()
		shareRatio := info.ComputeShareAdjustmentRatio()
		for i := range klines {
			if klines[i].Date >= info.Date {
				break
			}
			adj := CumulativeAdjustment{M: m, A: a, ShareAdjustmentRatio: shareRatio, No: klines[i].AdjustmentCount + 1}
			klines[i].Adjust(adj)
		}
	}
}

// UpdateXdxr 通过真实的 Level1 客户端从服务器获取除权除息数据，
// 并通过 config 包确定的文件名保存到本地缓存，然后返回加载的切片。
// 调用者无需注册 resolver；该函数会使用 config.GetXdxrFilename 生成路径。
// 该操作会发起真实的网络请求。
func UpdateXdxr(code string) ([]XdxrInfo, error) {
	// Compute filename via config package
	if len(code) != 8 {
		return nil, fmt.Errorf("invalid security code length: %s", code)
	}
	fname := config.GetXdxrFilename(code)
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

	req := level1.NewXdxrInfoRequest(code)
	resp := level1.NewXdxrInfoResponse()
	if err := level1.Process(conn, req, resp); err != nil {
		return nil, fmt.Errorf("xdxr request failed: %w", err)
	}

	// Convert level1.XdxrInfo -> datasets.XdxrInfo CSV rows and save
	if err := os.MkdirAll(filepath.Dir(fname), 0o755); err != nil {
		return nil, err
	}
	f, err := os.Create(fname)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()
	header := []string{"date", "category", "name", "fen_hong", "pei_gu_jia", "song_zhuan_gu", "pei_gu", "suo_gu", "qian_liu_tong", "hou_liu_tong", "qian_zong_gu_ben", "hou_zong_gu_ben", "fen_shu", "xing_quan_jia"}
	if err := w.Write(header); err != nil {
		return nil, err
	}
	out := make([]XdxrInfo, 0, len(resp.List))
	for _, it := range resp.List {
		// level1.XdxrInfo -> datasets.XdxrInfo mapping
		xi := XdxrInfo{
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
		row := []string{
			xi.Date,
			fmt.Sprintf("%d", xi.Category),
			xi.Name,
			fmt.Sprintf("%g", xi.FenHong),
			fmt.Sprintf("%g", xi.PeiGuJia),
			fmt.Sprintf("%g", xi.SongZhuanGu),
			fmt.Sprintf("%g", xi.PeiGu),
			fmt.Sprintf("%g", xi.SuoGu),
			fmt.Sprintf("%g", xi.QianLiuTong),
			fmt.Sprintf("%g", xi.HouLiuTong),
			fmt.Sprintf("%g", xi.QianZongGuBen),
			fmt.Sprintf("%g", xi.HouZongGuBen),
			fmt.Sprintf("%g", xi.FenShu),
			fmt.Sprintf("%g", xi.XingQuanJia),
		}
		if err := w.Write(row); err != nil {
			return nil, err
		}
		out = append(out, xi)
	}
	w.Flush()
	if err := w.Error(); err != nil {
		return nil, err
	}
	return out, nil
}

// DataXdxr 实现了 cache.DataAdapter，用于 XDXR 数据，并在包初始化时注册到缓存插件中心。
type DataXdxr struct{}

func (d *DataXdxr) Kind() cache.Kind { return BaseXdxr }
func (d *DataXdxr) Owner() string    { return cache.DefaultDataProvider }
func (d *DataXdxr) Key() string      { return "xdxr" }
func (d *DataXdxr) Name() string     { return "除权除息" }
func (d *DataXdxr) Usage() string    { return "" }

func (d *DataXdxr) Print(code string, dates ...exchange.Timestamp) {
	// No-op for now; could be extended to pretty-print loaded XDXR rows.
}

func (d *DataXdxr) Update(code string, date exchange.Timestamp) {
	// Delegate to UpdateXdxr which fetches and writes CSV cache.
	_, _ = UpdateXdxr(code)
}

func init() {
	// Best-effort registration; ignore error if already registered.
	_ = cache.Register(&DataXdxr{})
}
