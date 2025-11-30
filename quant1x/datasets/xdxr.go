package datasets

import (
	"encoding/csv"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/config"
	"gitee.com/quant1x/quant1x/quant1x/level1"
)

// XdxrInfo represents a single 除权除息 event row (CSV). Fields mirror the C++ layout.
type XdxrInfo struct {
	Date          string
	Category      int
	Name          string
	FenHong       float64
	PeiGuJia      float64
	SongZhuanGu   float64
	PeiGu         float64
	SuoGu         float64
	QianLiuTong   float64
	HouLiuTong    float64
	QianZongGuBen float64
	HouZongGuBen  float64
	FenShu        float64
	XingQuanJia   float64
}

// computeShareAdjustmentRatio mirrors C++ XdxrInfo::computeShareAdjustmentRatio
func (x *XdxrInfo) computeShareAdjustmentRatio() float64 {
	return (x.SongZhuanGu + x.PeiGu - x.SuoGu + x.FenShu) / 10.0
}

// computeMonetaryAdjustment mirrors C++ XdxrInfo::computeMonetaryAdjustment
func (x *XdxrInfo) computeMonetaryAdjustment() float64 {
	return (x.PeiGu*x.PeiGuJia - x.FenHong + x.FenShu*x.XingQuanJia) / 10.0
}

// adjustFactor mirrors C++ XdxrInfo::adjustFactor -> returns m,a
func (x *XdxrInfo) adjustFactor() (float64, float64) {
	A := x.computeMonetaryAdjustment()
	B := x.computeShareAdjustmentRatio()
	if (1.0 + B) == 0 {
		return 1.0, 0.0
	}
	m := 1.0 / (1.0 + B)
	a := A * m
	return m, a
}

// LoadXdxr tries to locate and read the local xdxr CSV cache for `code`.
// It looks under several likely cache locations (user home .q1x, .q1x-rust, and project ./xdxr).
// Returns an empty slice and an error if not found or parse fails.
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

// ApplyForwardAdjustmentForEvent applies forward-adjustment (前复权) to klines using the provided dividends/events.
// eventStartDate is the starting date used to filter out IPO-era events (format YYYY-MM-DD).
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
		m, a := info.adjustFactor()
		shareRatio := info.computeShareAdjustmentRatio()
		for i := range klines {
			if klines[i].Date >= info.Date {
				break
			}
			adj := CumulativeAdjustment{M: m, A: a, ShareAdjustmentRatio: shareRatio, No: klines[i].AdjustmentCount + 1}
			klines[i].Adjust(adj)
		}
	}
}

// UpdateXdxr fetches XDXR data from the Level1 servers using the real
// `level1.Client()`, saves it via the registered filename resolver, and
// returns the loaded slice. Caller must register `SetXdxrFilenameResolver`
// before calling. This performs a real network request.
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
	if err := level1.Process(conn.Conn(), req, resp); err != nil {
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
	header := []string{"Date", "Category", "Name", "FenHong", "PeiGuJia", "SongZhuanGu", "PeiGu", "SuoGu", "QianLiuTong", "HouLiuTong", "QianZongGuBen", "HouZongGuBen", "FenShu", "XingQuanJia"}
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
