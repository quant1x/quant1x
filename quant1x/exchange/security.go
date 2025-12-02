package exchange

// SecurityInfo 证券信息
type SecurityInfo struct {
	Code           string
	Name           string
	LotSize        int
	PricePrecision int
}

// GetSecurityInfo 获取证券信息
// TODO: 需要实现完整的证券信息获取逻辑, 包括:
// 1. 内存缓存
// 2. 本地文件加载 (securities.csv)
// 3. 远程接口更新 (level1)
func GetSecurityInfo(code string) *SecurityInfo {
	// 暂时返回 nil, 待实现
	return nil
}
