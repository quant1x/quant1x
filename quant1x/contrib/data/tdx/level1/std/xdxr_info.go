package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"math"

	"github.com/quant1x/quant1x/quant1x/data/exchange"
)

// XdxrCategory 除权除息类型枚举
type XdxrCategory int

const (
	ExDividend                     XdxrCategory = 1  // 除权除息
	BonusSharesListing             XdxrCategory = 2  // 送股上市(无偿)
	RestrictedSharesListing        XdxrCategory = 3  // 非流通股上市(受限股解禁)
	UnspecifiedCapitalAdjustment   XdxrCategory = 4  // 未知股本变动
	GeneralCapitalAdjustment       XdxrCategory = 5  // 股本变化(保留, 但慎用)
	NewShareIssuance               XdxrCategory = 6  // 增发新股
	ShareRepurchase                XdxrCategory = 7  // 股份回购
	NewSharesListing               XdxrCategory = 8  // 增发新股上市
	TransferredRightsSharesListing XdxrCategory = 9  // 转配股上市(中国特有)
	ConvertibleBondListing         XdxrCategory = 10 // 可转债上市
	StockSplitOrReverseSplit       XdxrCategory = 11 // 拆股或合股
	RestrictedSharesConsolidation  XdxrCategory = 12 // 非流通股缩股
	IssueCallWarrants              XdxrCategory = 13 // 送认购权证
	IssuePutWarrants               XdxrCategory = 14 // 送认沽权证
)

// ToString 将枚举值转换为描述文本
func (c XdxrCategory) ToString() string {
	switch c {
	case ExDividend:
		return "除权除息"
	case BonusSharesListing:
		return "送配股上市"
	case RestrictedSharesListing:
		return "非流通股上市"
	case UnspecifiedCapitalAdjustment:
		return "未知股本变动"
	case GeneralCapitalAdjustment:
		return "股本变化"
	case NewShareIssuance:
		return "增发新股"
	case ShareRepurchase:
		return "股份回购"
	case NewSharesListing:
		return "增发新股上市"
	case TransferredRightsSharesListing:
		return "转配股上市"
	case ConvertibleBondListing:
		return "可转债上市"
	case StockSplitOrReverseSplit:
		return "扩缩股"
	case RestrictedSharesConsolidation:
		return "非流通股缩股"
	case IssueCallWarrants:
		return "送认购权证"
	case IssuePutWarrants:
		return "送认沽权证"
	default:
		return fmt.Sprintf("Unknown(%d)", int(c))
	}
}

// XdxrInfoRequest encodes the XDXR_INFO request payload.
type XdxrInfoRequest struct {
	//Market uint8
	//Code   [6]byte
	Instrument exchange.InstrumentInfo
}

func (r XdxrInfoRequest) Serialize() []byte {
	payload := &bytes.Buffer{}
	padding := []byte{0x01, 0x00}
	payload.Write(padding)
	market := uint8(exchangeToMarketId(r.Instrument.Exchange))
	payload.WriteByte(market)
	payload.Write([]byte(r.Instrument.Ticker)[:6])
	return buildRequest(StdCommandXdxrInfo, packetTypeRequest, payload.Bytes())
}

func (r XdxrInfoRequest) Command() StdCommand { return StdCommandXdxrInfo }

func (r XdxrInfoRequest) String() string {
	return fmt.Sprintf("XdxrInfoRequest{%s}", r.Instrument.Symbol())
}

// XdxrInfo represents a parsed XDXR event returned by the server.
type XdxrInfo struct {
	Date          string  // 日期 YYYY-MM-DD格式
	Category      int     // 类型编号
	Name          string  // 类型名称
	FenHong       float64 // 分红(元)
	PeiGuJia      float64 // 配股价(元)
	SongZhuanGu   float64 // 送转股(股)
	PeiGu         float64 // 配股(股)
	SuoGu         float64 // 缩股(股)
	QianLiuTong   float64 // 除权前流通股(万股)
	HouLiuTong    float64 // 除权后流通股(万股)
	QianZongGuBen float64 // 除权前总股本(万股)
	HouZongGuBen  float64 // 除权后总股本(万股)
	FenShu        float64 // 权证份数
	XingQuanJia   float64 // 行权价格(元)
}

// IsAdjust 是否进行除权除息调整
func (x XdxrInfo) IsAdjust() bool {
	count := x.FenHong     // 分红
	count += x.PeiGu       // 配股
	count += x.SongZhuanGu // 送转股
	count += x.SuoGu       // 缩股
	count += x.FenShu      // 行权
	return count > 0.0
}

// AdjustFactor 计算调整因子m和a
//
// 根据股票分红配股等参数计算价格调整因子, 用于复权计算
//
// 返回调整因子m和a的元组
// - m: 价格调整乘数因子
// - a: 价格调整加数因子
//
// 当1+B接近0时, 会返回默认值m=1.0和a=0.0
func (x XdxrInfo) AdjustFactor() (float64, float64) {
	var m, a float64

	// 计算货币调整项和股本调整比率(通过独立函数)
	A := x.ComputeMonetaryAdjustment()
	B := x.ComputeShareAdjustmentRatio()

	if math.Abs(1.0+B) > 1e-10 {
		m = 1.0 / (1.0 + B)
		if m < 0 {
			m = 1.0
		}
		a = A * m
	} else {
		m = 1.0
		a = 0.0
	}

	return m, a
}

// ComputeMonetaryAdjustment 计算货币调整项 (monetary adjustment per 10 shares -> per-share adjust after /10)
func (x XdxrInfo) ComputeMonetaryAdjustment() float64 {
	// (配股数量 * 配股价 - 分红 + 权证份数 * 行权价格) / 10
	return (x.PeiGu*x.PeiGuJia - x.FenHong + x.FenShu*x.XingQuanJia) / 10.0
}

// ComputeShareAdjustmentRatio 计算股本调整比率 (新增股数/送转股/缩股/行权影响) / 10
func (x XdxrInfo) ComputeShareAdjustmentRatio() float64 {
	// (送转股 + 配股 - 缩股 + 权证份数) / 10
	return (x.SongZhuanGu + x.PeiGu - x.SuoGu + x.FenShu) / 10.0
}

// IsCapitalChange 判断是否是股本变化
// 返回: true表示是股本变化, false表示不是
func (x XdxrInfo) IsCapitalChange() bool {
	switch x.Category {
	case int(ExDividend), // 除权除息
		int(StockSplitOrReverseSplit),      // 拆股或合股
		int(RestrictedSharesConsolidation), // 非流通股缩股
		int(IssueCallWarrants),             // 送认购权证
		int(IssuePutWarrants):              // 送认沽权证
		return false
	default:
		if x.HouLiuTong > 0 && x.HouZongGuBen > 0 {
			return true
		}
	}
	return false
}

// Adjust 生成复权计算函数
// 返回: 计算复权价格的函数对象
func (x XdxrInfo) Adjust() func(float64) float64 {
	songZhuangu := x.SongZhuanGu
	peiGu := x.PeiGu
	suoGu := x.SuoGu
	xdxrGuShu := (songZhuangu + peiGu - suoGu) / 10
	fenHong := x.FenHong
	peiGuJia := x.PeiGuJia
	xdxrFenHong := (peiGuJia*peiGu - fenHong) / 10

	return func(p float64) float64 {
		return (p + xdxrFenHong) / (1 + xdxrGuShu)
	}
}

// String 返回字符串表示
func (x XdxrInfo) String() string {
	return fmt.Sprintf("Date: %s Category: %d Name: %s FenHong: %f PeiGuJia: %f SongZhuanGu: %f PeiGu: %f SuoGu: %f QianLiuTong: %f HouLiuTong: %f QianZongGuBen: %f HouZongGuBen: %f FenShu: %f XingQuanJia: %f",
		x.Date, x.Category, x.Name, x.FenHong, x.PeiGuJia, x.SongZhuanGu, x.PeiGu, x.SuoGu, x.QianLiuTong, x.HouLiuTong, x.QianZongGuBen, x.HouZongGuBen, x.FenShu, x.XingQuanJia)
}

// XdxrInfoResponse decodes the response body for XDXR_INFO
type XdxrInfoResponse struct {
	ResponseBase
	Count uint16
	List  []XdxrInfo
}

func NewXdxrInfoResponse() *XdxrInfoResponse { return &XdxrInfoResponse{} }

func (r *XdxrInfoResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	// skip 9 bytes as in C++ (Unknown header)
	if _, err := reader.Seek(9, io.SeekStart); err != nil {
		return err
	}
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}
	r.List = make([]XdxrInfo, 0, int(r.Count))
	for i := 0; i < int(r.Count); i++ {
		var market uint8
		if err := binary.Read(reader, binary.LittleEndian, &market); err != nil {
			return err
		}
		codeBuf := make([]byte, 6)
		if _, err := io.ReadFull(reader, codeBuf); err != nil {
			return err
		}
		// unknown byte
		var _unk uint8
		if err := binary.Read(reader, binary.LittleEndian, &_unk); err != nil {
			return err
		}
		var dateRaw uint32
		if err := binary.Read(reader, binary.LittleEndian, &dateRaw); err != nil {
			return err
		}
		var category uint8
		if err := binary.Read(reader, binary.LittleEndian, &category); err != nil {
			return err
		}
		data := make([]byte, 16)
		if _, err := io.ReadFull(reader, data); err != nil {
			return err
		}

		y, m, d, _, _ := getDatetimeFromUint32(9, dateRaw, 0)
		xi := XdxrInfo{Date: fmt.Sprintf("%04d-%02d-%02d", y, m, d), Category: int(category), Name: XdxrCategory(category).ToString()}

		// parse data per category similar to C++ logic
		db := bytes.NewReader(data)
		switch category {
		case 1: // 除权除息
			var f32v float32
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.FenHong = float64(f32v)
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.PeiGuJia = float64(f32v)
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.SongZhuanGu = float64(f32v)
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.PeiGu = float64(f32v)
		case 11, 12:
			// skip 8 then suogu float32
			if _, err := db.Seek(8, io.SeekStart); err == nil {
				var f32v float32
				_ = binary.Read(db, binary.LittleEndian, &f32v)
				xi.SuoGu = float64(f32v)
			}
		case 13, 14:
			var f32v float32
			_ = binary.Read(db, binary.LittleEndian, &f32v)
			xi.XingQuanJia = float64(f32v)
			if _, err := db.Seek(8, io.SeekCurrent); err == nil {
				var f32v2 float32
				_ = binary.Read(db, binary.LittleEndian, &f32v2)
				xi.FenShu = float64(f32v2)
			}
		default:
			var v uint32
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.QianLiuTong = integerToFloat64(v)
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.QianZongGuBen = integerToFloat64(v)
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.HouLiuTong = integerToFloat64(v)
			_ = binary.Read(db, binary.LittleEndian, &v)
			xi.HouZongGuBen = integerToFloat64(v)
		}

		r.List = append(r.List, xi)
	}
	return nil
}

func (r *XdxrInfoResponse) String() string {
	return fmt.Sprintf("XdxrInfoResponse{Count:%d}", r.Count)
}
