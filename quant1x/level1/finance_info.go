package level1

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"

	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/std"
)

const (
	FinanceInfoPerRequestMax = 100 // 单次请求的最大记录数
)

// FinanceRequest 请求结构
type FinanceRequest struct {
	Count  uint16
	Market uint8
	Code   [6]byte
	Codes  []string
}

// NewFinanceRequest 构建请求，securityCode 可传入任意形式的代码
func NewFinanceRequest(securityCode string) FinanceRequest {
	var req FinanceRequest
	req.Count = 1
	mid, _, symbol, err := exchange.DetectMarket(securityCode)
	if err == nil {
		req.Market = uint8(mid)
		// copy up to 6 bytes
		sym := symbol
		if len(sym) > 6 {
			sym = sym[:6]
		}
		copy(req.Code[:], []byte(sym))
	} else {
		// fallback: fill code with trimmed input
		s := securityCode
		if len(s) > 6 {
			s = s[:6]
		}
		copy(req.Code[:], []byte(s))
	}
	return req
}

func (r FinanceRequest) Serialize() []byte {
	buf := &bytes.Buffer{}
	// 写入代码数量
	count := uint16(len(r.Codes))
	_ = binary.Write(buf, binary.LittleEndian, count)
	// 遍历 Codes 列表，写入每个代码
	for _, code := range r.Codes {
		mid, _, symbol, err := exchange.DetectMarket(code)
		if err != nil {
			continue
		}
		// 写入市场代码
		_ = buf.WriteByte(uint8(mid))
		// 写入证券代码，固定6字节
		sym := std.String2Bytes(symbol)
		if len(sym) > 6 {
			sym = sym[:6]
		}
		buf.Write([]byte(sym))
	}
	return buildRequest(StdCommandFinanceInfo, packetTypeRequest, buf.Bytes())
}

func (FinanceRequest) Command() StdCommand { return StdCommandFinanceInfo }

func (r FinanceRequest) String() string {
	return fmt.Sprintf("FinanceRequest{Count:%d,Market:%d,Code:%s}", r.Count, r.Market, std.Bytes2String(r.Code[:]))
}

// RawFinanceInfo 对应二进制原始结构（按 finance_info.h 定义）
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
	// Market
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
	BaoLiu2            float64 `csv:"bao_liu2"`
}

// IsDelisting 判断该金融信息是否表示股票已退市
//
//	当IPO日期、总股本和流通股本均为0时返回true
func (f FinanceInfo) IsDelisting() bool {
	return f.IPODate == 0 && f.ZongGuBen == 0 && f.LiuTongGuBen == 0
}

func (f FinanceInfo) String() string {
	return fmt.Sprintf("FinanceInfo{Code: %s, LiuTongGuBen: %f, Province: %d, Industry: %d, UpdatedDate: %d, IPODate: %d, ZongGuBen: %f, GuoJiaGu: %f, FaQiRenFaRenGu: %f, FaRenGu: %f, BGu: %f, HGu: %f, ZhiGongGu: %f, ZongZiChan: %f, LiuDongZiChan: %f, GuDingZiChan: %f, WuXingZiChan: %f, GuDongRenShu: %f, LiuDongFuZhai: %f, ChangQiFuZhai: %f, ZiBenGongJiJin: %f, JingZiChan: %f, ZhuYingShouRu: %f, ZhuYingLiRun: %f, YingShouZhangKuan: %f, YingYeLiRun: %f, TouZiShouYu: %f, JingYingXianJinLiu: %f, ZongXianJinLiu: %f, CunHuo: %f, LiRunZongHe: %f, ShuiHouLiRun: %f, JingLiRun: %f, WeiFenLiRun: %f, MeiGuJingZiChan: %f, BaoLiu2: %f}", f.Code, f.LiuTongGuBen, f.Province, f.Industry, f.UpdatedDate, f.IPODate, f.ZongGuBen, f.GuoJiaGu, f.FaQiRenFaRenGu, f.FaRenGu, f.BGu, f.HGu, f.ZhiGongGu, f.ZongZiChan, f.LiuDongZiChan, f.GuDingZiChan, f.WuXingZiChan, f.GuDongRenShu, f.LiuDongFuZhai, f.ChangQiFuZhai, f.ZiBenGongJiJin, f.JingZiChan, f.ZhuYingShouRu, f.ZhuYingLiRun, f.YingShouZhangKuan, f.YingYeLiRun, f.TouZiShouYu, f.JingYingXianJinLiu, f.ZongXianJinLiu, f.CunHuo, f.LiRunZongHe, f.ShuiHouLiRun, f.JingLiRun, f.WeiFenLiRun, f.MeiGuJingZiChan, f.BaoLiu2)
}

// FinanceResponse 响应
type FinanceResponse struct {
	ResponseBase
	Count uint16
	Info  FinanceInfo
	List  []FinanceInfo
}

func (r *FinanceResponse) Deserialize(body []byte) error {
	reader := bytes.NewReader(body)
	if err := binary.Read(reader, binary.LittleEndian, &r.Count); err != nil {
		return err
	}
	if r.Count == 0 {
		return nil
	}
	for i := 0; i < int(r.Count); i++ {
		var raw RawFinanceInfo
		if err := raw.decode(reader); err != nil {
			return err
		}
		var info FinanceInfo
		const baseUnit = 10000.0
		code := std.Bytes2String(raw.Code[:])
		info.Code = exchange.GetSecurityCode(exchange.ExchangeId(raw.Market), code)
		info.LiuTongGuBen = NumberToFloat64(raw.LiuTongGuBen) * baseUnit
		info.Province = raw.Province
		info.Industry = raw.Industry
		info.UpdatedDate = raw.UpdatedDate
		info.IPODate = raw.IPODate
		info.ZongGuBen = NumberToFloat64(raw.ZongGuBen) * baseUnit
		info.GuoJiaGu = NumberToFloat64(raw.GuoJiaGu) * baseUnit
		info.FaQiRenFaRenGu = NumberToFloat64(raw.FaQiRenFaRenGu) * baseUnit
		info.FaRenGu = NumberToFloat64(raw.FaRenGu) * baseUnit
		info.BGu = NumberToFloat64(raw.BGu) * baseUnit
		info.HGu = NumberToFloat64(raw.HGu) * baseUnit
		info.ZhiGongGu = NumberToFloat64(raw.ZhiGongGu) * baseUnit
		info.ZongZiChan = NumberToFloat64(raw.ZongZiChan) * baseUnit
		info.LiuDongZiChan = NumberToFloat64(raw.LiuDongZiChan) * baseUnit
		info.GuDingZiChan = NumberToFloat64(raw.GuDingZiChan) * baseUnit
		info.WuXingZiChan = NumberToFloat64(raw.WuXingZiChan) * baseUnit
		info.GuDongRenShu = NumberToFloat64(raw.GuDongRenShu)
		info.LiuDongFuZhai = NumberToFloat64(raw.LiuDongFuZhai) * baseUnit
		info.ChangQiFuZhai = NumberToFloat64(raw.ChangQiFuZhai) * baseUnit
		info.ZiBenGongJiJin = NumberToFloat64(raw.ZiBenGongJiJin) * baseUnit
		info.JingZiChan = NumberToFloat64(raw.JingZiChan) * baseUnit
		info.ZhuYingShouRu = NumberToFloat64(raw.ZhuYingShouRu) * baseUnit
		info.ZhuYingLiRun = NumberToFloat64(raw.ZhuYingLiRun) * baseUnit
		info.YingShouZhangKuan = NumberToFloat64(raw.YingShouZhangKuan) * baseUnit
		info.YingYeLiRun = NumberToFloat64(raw.YingYeLiRun) * baseUnit
		info.TouZiShouYu = NumberToFloat64(raw.TouZiShouYu) * baseUnit
		info.JingYingXianJinLiu = NumberToFloat64(raw.JingYingXianJinLiu) * baseUnit
		info.ZongXianJinLiu = NumberToFloat64(raw.ZongXianJinLiu) * baseUnit
		info.CunHuo = NumberToFloat64(raw.CunHuo) * baseUnit
		info.LiRunZongHe = NumberToFloat64(raw.LiRunZongHe) * baseUnit
		info.ShuiHouLiRun = NumberToFloat64(raw.ShuiHouLiRun) * baseUnit
		info.JingLiRun = NumberToFloat64(raw.JingLiRun) * baseUnit
		info.WeiFenLiRun = NumberToFloat64(raw.WeiFenLiRun) * baseUnit
		info.MeiGuJingZiChan = NumberToFloat64(raw.BaoLiu1) * baseUnit
		info.BaoLiu2 = NumberToFloat64(raw.BaoLiu2)
		r.List = append(r.List, info)
	}
	return nil
}

func (r *FinanceResponse) String() string {
	return fmt.Sprintf("FinanceResponse{Count:%d, List:%v}", r.Count, r.List)
}
