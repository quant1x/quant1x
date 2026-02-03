package tdx

type SectorType = int

const (
	BK_UNKNOWN SectorType = 0  // 未知类型
	BK_HANGYE  SectorType = 2  // 行业
	BK_DIQU    SectorType = 3  // 地区
	BK_GAINIAN SectorType = 4  // 概念
	BK_FENGGE  SectorType = 5  // 风格
	BK_ZHISHU  SectorType = 6  // 指数
	BK_YJHY    SectorType = 12 // 研究行业

	BKN_HANGYE  = "行业"
	BKN_DIQU    = "地区"
	BKN_GAINIAN = "概念"
	BKN_FENGGE  = "风格"
	BKN_ZHISHU  = "指数"
	BKN_YJHY    = "研究行业"
)

var (
	mapSectorType = map[SectorType]string{
		BK_HANGYE:  BKN_HANGYE,
		BK_DIQU:    BKN_DIQU,
		BK_GAINIAN: BKN_GAINIAN,
		BK_FENGGE:  BKN_FENGGE,
		BK_ZHISHU:  BKN_ZHISHU,
		BK_YJHY:    BKN_YJHY,
	}
)

// SectorTypeNameByCode 通过板块类型代码获取板块类型名称
func SectorTypeNameByCode(sector_code int) (name string, ok bool) {
	type_ := SectorType(sector_code)
	return SectorTypeNameByType(type_)
}

// SectorTypeNameByType 通过板块类型代码获取板块类型名称
func SectorTypeNameByType(blockType SectorType) (string, bool) {
	sector_name, found := mapSectorType[blockType]
	return sector_name, found
}
