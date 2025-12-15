package instruments

import (
	"fmt"
	"strings"
)

// AShareIndexList A股指数列表
var AShareIndexList = []string{
	"sh000001", // 上证综合指数
	"sh000002", // 上证A股指数
	"sh000300", // 沪深300指数
	"sh000688", // 科创50指数
	"sh000905", // 中证500指数
	"sz399001", // 深证成份指数
	"sz399006", // 创业板指
	"sz399107", // 深证A指
	"bj899050", // 北证50指数
	"sh880005", // 通达信板块-涨跌家数
	"sh510050", // 上证50ETF
	"sh510300", // 沪深300ETF
	"sh510900", // H股ETF
}

// IsNeedIgnore 证券代码是否需要忽略, 这是一个不参与数据和策略处理的开关
func IsNeedIgnore(code string) bool {
	p := GetSecurityInfo(code)
	if p == nil {
		// 没找到直接忽略
		return true
	}

	// 需要检查的关键字列表
	ignoredKeywords := []string{"ST", "退", "摘牌"}

	// 转换名称为大写
	upperName := strings.ToUpper(p.Name)

	// 检查是否存在任意关键字
	for _, keyword := range ignoredKeywords {
		if strings.Contains(upperName, keyword) {
			return true
		}
	}
	return false
}

// GetStockCodeList 获取证券代码列表, 过滤退市、摘牌和ST标记的个股
func GetStockCodeList() []string {
	var allCodes []string

	// 上海证券交易所 (sh600000-sh609999)
	for i := 600000; i <= 609999; i++ {
		fc := fmt.Sprintf("sh%06d", i)
		if !IsNeedIgnore(fc) {
			allCodes = append(allCodes, fc)
		}
	}

	// 科创板 (sh688000-sh689999)
	for i := 688000; i <= 689999; i++ {
		fc := fmt.Sprintf("sh%06d", i)
		if !IsNeedIgnore(fc) {
			allCodes = append(allCodes, fc)
		}
	}

	// 深圳主板 (sz000000-sz000999)
	for i := 0; i <= 999; i++ {
		fc := fmt.Sprintf("sz%06d", i)
		if !IsNeedIgnore(fc) {
			allCodes = append(allCodes, fc)
		}
	}

	// 中小板 (sz001000-sz009999)
	for i := 1000; i <= 9999; i++ {
		fc := fmt.Sprintf("sz%06d", i)
		if !IsNeedIgnore(fc) {
			allCodes = append(allCodes, fc)
		}
	}

	// 创业板 (sz300000-sz300999)
	for i := 300000; i <= 309999; i++ {
		fc := fmt.Sprintf("sz%06d", i)
		if !IsNeedIgnore(fc) {
			allCodes = append(allCodes, fc)
		}
	}

	// 北交所 (bj920000-bj920999)
	for i := 920000; i <= 920999; i++ {
		fc := fmt.Sprintf("bj%06d", i)
		if !IsNeedIgnore(fc) {
			allCodes = append(allCodes, fc)
		}
	}

	return allCodes
}

// GetCodeList 加载全部指数、板块和个股的代码
func GetCodeList() []string {
	var list []string
	// 1. 指数
	list = append(list, AShareIndexList...)

	// 2. 板块
	// TODO: 需要实现 GetSectorList
	// sectors := GetSectorList
	// for _, v := range sectors {
	//  list = append(list, v.Code)
	// }

	// 3. 个股, 包括场内开放式ETF基金
	stockCodeList := GetStockCodeList()
	list = append(list, stockCodeList...)

	return list
}
