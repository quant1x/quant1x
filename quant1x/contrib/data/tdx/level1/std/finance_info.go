package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
	"github.com/quant1x/quant1x/quant1x/data"
	"github.com/quant1x/quant1x/quant1x/base"
)

const (
	FinanceInfoPerRequestMax = 100 // 单次请求的最大记录数
)

// FinanceInfoContext 对齐 C++/Rust/Python FinanceRequest/FinanceResponse, 合并请求和响应.
type FinanceInfoContext struct {
	tdxproto.FrameBase
	Codes []data.InstrumentInfo // 请求: 证券代码列表
	RespCount uint16             // 响应: 记录数量
	List  []FinanceInfo          // 响应: 解析结果
}

// NewFinanceInfoContext 构造财务信息请求, 对齐 C++/Rust.
func NewFinanceInfoContext(codes []data.InstrumentInfo) *FinanceInfoContext {
	return &FinanceInfoContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandFinanceInfo, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
		Codes:     codes,
	}
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (f *FinanceInfoContext) SerializeRequestBody() []byte {
	buf := &bytes.Buffer{}
	count := uint16(len(f.Codes))
	_ = binary.Write(buf, binary.LittleEndian, count)
	for _, code := range f.Codes {
		_ = buf.WriteByte(uint8(tdxproto.ExchangeToMarketId(code.Exchange)))
		sym := base.String2Bytes(code.Ticker)
		if len(sym) > 6 {
			sym = sym[:6]
		}
		buf.Write([]byte(sym))
	}
	return buf.Bytes()
}

// DeserializeResponseBody 解析财务信息响应体, 对齐 C++/Rust/Python.
func (f *FinanceInfoContext) DeserializeResponseBody(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &f.RespCount); err != nil {
		return err
	}
	if f.RespCount == 0 {
		return nil
	}
	for i := 0; i < int(f.RespCount); i++ {
		var raw RawFinanceInfo
		if err := raw.decode(reader); err != nil {
			return err
		}
		var info FinanceInfo
		const baseUnit = 10000.0

		ex := tdxproto.MarketIdToExchange(int(raw.Market))
		ticker := base.Bytes2String(raw.Code[:])

		info.Code = data.BuildInstrument(ex, ticker)
		info.LiuTongGuBen = tdxproto.NumberToFloat64(raw.LiuTongGuBen) * baseUnit
		info.Province = raw.Province
		info.Industry = raw.Industry
		info.UpdatedDate = raw.UpdatedDate
		info.IPODate = raw.IPODate
		info.ZongGuBen = tdxproto.NumberToFloat64(raw.ZongGuBen) * baseUnit
		info.GuoJiaGu = tdxproto.NumberToFloat64(raw.GuoJiaGu) * baseUnit
		info.FaQiRenFaRenGu = tdxproto.NumberToFloat64(raw.FaQiRenFaRenGu) * baseUnit
		info.FaRenGu = tdxproto.NumberToFloat64(raw.FaRenGu) * baseUnit
		info.BGu = tdxproto.NumberToFloat64(raw.BGu) * baseUnit
		info.HGu = tdxproto.NumberToFloat64(raw.HGu) * baseUnit
		info.ZhiGongGu = tdxproto.NumberToFloat64(raw.ZhiGongGu) * baseUnit
		info.ZongZiChan = tdxproto.NumberToFloat64(raw.ZongZiChan) * baseUnit
		info.LiuDongZiChan = tdxproto.NumberToFloat64(raw.LiuDongZiChan) * baseUnit
		info.GuDingZiChan = tdxproto.NumberToFloat64(raw.GuDingZiChan) * baseUnit
		info.WuXingZiChan = tdxproto.NumberToFloat64(raw.WuXingZiChan) * baseUnit
		info.GuDongRenShu = tdxproto.NumberToFloat64(raw.GuDongRenShu)
		info.LiuDongFuZhai = tdxproto.NumberToFloat64(raw.LiuDongFuZhai) * baseUnit
		info.ChangQiFuZhai = tdxproto.NumberToFloat64(raw.ChangQiFuZhai) * baseUnit
		info.ZiBenGongJiJin = tdxproto.NumberToFloat64(raw.ZiBenGongJiJin) * baseUnit
		info.JingZiChan = tdxproto.NumberToFloat64(raw.JingZiChan) * baseUnit
		info.ZhuYingShouRu = tdxproto.NumberToFloat64(raw.ZhuYingShouRu) * baseUnit
		info.ZhuYingLiRun = tdxproto.NumberToFloat64(raw.ZhuYingLiRun) * baseUnit
		info.YingShouZhangKuan = tdxproto.NumberToFloat64(raw.YingShouZhangKuan) * baseUnit
		info.YingYeLiRun = tdxproto.NumberToFloat64(raw.YingYeLiRun) * baseUnit
		info.TouZiShouYu = tdxproto.NumberToFloat64(raw.TouZiShouYu) * baseUnit
		info.JingYingXianJinLiu = tdxproto.NumberToFloat64(raw.JingYingXianJinLiu) * baseUnit
		info.ZongXianJinLiu = tdxproto.NumberToFloat64(raw.ZongXianJinLiu) * baseUnit
		info.CunHuo = tdxproto.NumberToFloat64(raw.CunHuo) * baseUnit
		info.LiRunZongHe = tdxproto.NumberToFloat64(raw.LiRunZongHe) * baseUnit
		info.ShuiHouLiRun = tdxproto.NumberToFloat64(raw.ShuiHouLiRun) * baseUnit
		info.JingLiRun = tdxproto.NumberToFloat64(raw.JingLiRun) * baseUnit
		info.WeiFenLiRun = tdxproto.NumberToFloat64(raw.WeiFenLiRun) * baseUnit
		info.MeiGuJingZiChan = tdxproto.NumberToFloat64(raw.BaoLiu1) * baseUnit
		f.List = append(f.List, info)
	}
	return nil
}

func (f *FinanceInfoContext) String() string {
	return fmt.Sprintf("FinanceInfoContext{RespCount:%d, ListLen:%d}", f.RespCount, len(f.List))
}

// RawFinanceInfo 对应二进制原始结构(按 finance_info.h 定义)
type RawFinanceInfo struct {
	Market             uint8
	Code               [6]byte
	LiuTongGuBen       float32
	Province           uint16
	Industry           uint16
	UpdatedDate        uint32
	IPODate            uint32
	ZongGuBen          float32
	GuoJiaGu           float32
	FaQiRenFaRenGu     float32
	FaRenGu            float32
	BGu                float32
	HGu                float32
	ZhiGongGu          float32
	ZongZiChan         float32
	LiuDongZiChan      float32
	GuDingZiChan       float32
	WuXingZiChan       float32
	GuDongRenShu       float32
	LiuDongFuZhai      float32
	ChangQiFuZhai      float32
	ZiBenGongJiJin     float32
	JingZiChan         float32
	ZhuYingShouRu      float32
	ZhuYingLiRun       float32
	YingShouZhangKuan  float32
	YingYeLiRun        float32
	TouZiShouYu        float32
	JingYingXianJinLiu float32
	ZongXianJinLiu     float32
	CunHuo             float32
	LiRunZongHe        float32
	ShuiHouLiRun       float32
	JingLiRun          float32
	WeiFenLiRun        float32
	BaoLiu1            float32
	BaoLiu2            float32
}

func (r *RawFinanceInfo) decode(reader *bytes.Reader) error {
	if b, err := reader.ReadByte(); err != nil {
		return err
	} else {
		r.Market = b
	}
	if _, err := io.ReadFull(reader, r.Code[:]); err != nil {
		return err
	}
	read := func(data interface{}) error { return binary.Read(reader, binary.LittleEndian, data) }
	if err := read(&r.LiuTongGuBen); err != nil {
		return err
	}
	if err := read(&r.Province); err != nil {
		return err
	}
	if err := read(&r.Industry); err != nil {
		return err
	}
	if err := read(&r.UpdatedDate); err != nil {
		return err
	}
	if err := read(&r.IPODate); err != nil {
		return err
	}
	if err := read(&r.ZongGuBen); err != nil {
		return err
	}
	if err := read(&r.GuoJiaGu); err != nil {
		return err
	}
	if err := read(&r.FaQiRenFaRenGu); err != nil {
		return err
	}
	if err := read(&r.FaRenGu); err != nil {
		return err
	}
	if err := read(&r.BGu); err != nil {
		return err
	}
	if err := read(&r.HGu); err != nil {
		return err
	}
	if err := read(&r.ZhiGongGu); err != nil {
		return err
	}
	if err := read(&r.ZongZiChan); err != nil {
		return err
	}
	if err := read(&r.LiuDongZiChan); err != nil {
		return err
	}
	if err := read(&r.GuDingZiChan); err != nil {
		return err
	}
	if err := read(&r.WuXingZiChan); err != nil {
		return err
	}
	if err := read(&r.GuDongRenShu); err != nil {
		return err
	}
	if err := read(&r.LiuDongFuZhai); err != nil {
		return err
	}
	if err := read(&r.ChangQiFuZhai); err != nil {
		return err
	}
	if err := read(&r.ZiBenGongJiJin); err != nil {
		return err
	}
	if err := read(&r.JingZiChan); err != nil {
		return err
	}
	if err := read(&r.ZhuYingShouRu); err != nil {
		return err
	}
	if err := read(&r.ZhuYingLiRun); err != nil {
		return err
	}
	if err := read(&r.YingShouZhangKuan); err != nil {
		return err
	}
	if err := read(&r.YingYeLiRun); err != nil {
		return err
	}
	if err := read(&r.TouZiShouYu); err != nil {
		return err
	}
	if err := read(&r.JingYingXianJinLiu); err != nil {
		return err
	}
	if err := read(&r.ZongXianJinLiu); err != nil {
		return err
	}
	if err := read(&r.CunHuo); err != nil {
		return err
	}
	if err := read(&r.LiRunZongHe); err != nil {
		return err
	}
	if err := read(&r.ShuiHouLiRun); err != nil {
		return err
	}
	if err := read(&r.JingLiRun); err != nil {
		return err
	}
	if err := read(&r.WeiFenLiRun); err != nil {
		return err
	}
	if err := read(&r.BaoLiu1); err != nil {
		return err
	}
	if err := read(&r.BaoLiu2); err != nil {
		return err
	}
	return nil
}

// FinanceInfo 高级表示
type FinanceInfo struct {
	Code               string  `csv:"code"`
	LiuTongGuBen       float64 `csv:"liu_tong_gu_ben"`
	Province           uint16  `csv:"province"`
	Industry           uint16  `csv:"industry"`
	UpdatedDate        uint32  `csv:"updated_date"`
	IPODate            uint32  `csv:"ipo_date"`
	ZongGuBen          float64 `csv:"zong_gu_ben"`
	GuoJiaGu           float64 `csv:"guo_jia_gu"`
	FaQiRenFaRenGu     float64 `csv:"fa_qi_ren_fa_ren_gu"`
	FaRenGu            float64 `csv:"fa_ren_gu"`
	BGu                float64 `csv:"b_gu"`
	HGu                float64 `csv:"h_gu"`
	ZhiGongGu          float64 `csv:"zhi_gong_gu"`
	ZongZiChan         float64 `csv:"zong_zi_chan"`
	LiuDongZiChan      float64 `csv:"liu_dong_zi_chan"`
	GuDingZiChan       float64 `csv:"gu_ding_zi_chan"`
	WuXingZiChan       float64 `csv:"wu_xing_zi_chan"`
	GuDongRenShu       float64 `csv:"gu_dong_ren_shu"`
	LiuDongFuZhai      float64 `csv:"liu_dong_fu_zhai"`
	ChangQiFuZhai      float64 `csv:"chang_qi_fu_zhai"`
	ZiBenGongJiJin     float64 `csv:"zi_ben_gong_ji_jin"`
	JingZiChan         float64 `csv:"jing_zi_chan"`
	ZhuYingShouRu      float64 `csv:"zhu_ying_shou_ru"`
	ZhuYingLiRun       float64 `csv:"zhu_ying_li_run"`
	YingShouZhangKuan  float64 `csv:"ying_shou_zhang_kuan"`
	YingYeLiRun        float64 `csv:"ying_ye_li_run"`
	TouZiShouYu        float64 `csv:"tou_zi_shou_yu"`
	JingYingXianJinLiu float64 `csv:"jing_ying_xian_jin_liu"`
	ZongXianJinLiu     float64 `csv:"zong_xian_jin_liu"`
	CunHuo             float64 `csv:"cun_huo"`
	LiRunZongHe        float64 `csv:"li_run_zong_he"`
	ShuiHouLiRun       float64 `csv:"shui_hou_li_run"`
	JingLiRun          float64 `csv:"jing_li_run"`
	WeiFenLiRun        float64 `csv:"wei_fen_li_run"`
	MeiGuJingZiChan    float64 `csv:"mei_gu_jing_zi_chan"`
}

func (f FinanceInfo) IsDelisting() bool {
	return f.IPODate == 0 && f.ZongGuBen == 0 && f.LiuTongGuBen == 0
}

func (f FinanceInfo) String() string {
	return fmt.Sprintf("FinanceInfo{Code: %s, LiuTongGuBen: %f, Province: %d, Industry: %d, UpdatedDate: %d, IPODate: %d, ZongGuBen: %f, GuoJiaGu: %f, FaQiRenFaRenGu: %f, FaRenGu: %f, BGu: %f, HGu: %f, ZhiGongGu: %f, ZongZiChan: %f, LiuDongZiChan: %f, GuDingZiChan: %f, WuXingZiChan: %f, GuDongRenShu: %f, LiuDongFuZhai: %f, ChangQiFuZhai: %f, ZiBenGongJiJin: %f, JingZiChan: %f, ZhuYingShouRu: %f, ZhuYingLiRun: %f, YingShouZhangKuan: %f, YingYeLiRun: %f, TouZiShouYu: %f, JingYingXianJinLiu: %f, ZongXianJinLiu: %f, CunHuo: %f, LiRunZongHe: %f, ShuiHouLiRun: %f, JingLiRun: %f, WeiFenLiRun: %f, MeiGuJingZiChan: %f}", f.Code, f.LiuTongGuBen, f.Province, f.Industry, f.UpdatedDate, f.IPODate, f.ZongGuBen, f.GuoJiaGu, f.FaQiRenFaRenGu, f.FaRenGu, f.BGu, f.HGu, f.ZhiGongGu, f.ZongZiChan, f.LiuDongZiChan, f.GuDingZiChan, f.WuXingZiChan, f.GuDongRenShu, f.LiuDongFuZhai, f.ChangQiFuZhai, f.ZiBenGongJiJin, f.JingZiChan, f.ZhuYingShouRu, f.ZhuYingLiRun, f.YingShouZhangKuan, f.YingYeLiRun, f.TouZiShouYu, f.JingYingXianJinLiu, f.ZongXianJinLiu, f.CunHuo, f.LiRunZongHe, f.ShuiHouLiRun, f.JingLiRun, f.WeiFenLiRun, f.MeiGuJingZiChan)
}
