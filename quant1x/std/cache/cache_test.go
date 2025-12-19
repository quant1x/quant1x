package cache

import (
	"fmt"
	"testing"
	"unsafe"
)

func TestCache_000(t *testing.T) {
	_ = DefaultSize
	_ = THRESHOLD
	_ = maxFileSize
	_ = ErrInvalidAccess
}

type FinancialData struct {
	Timestamp int64
	Open      float64
	High      float64
	Low       float64
	Close     float64
}

func TestCacheToSlice(t *testing.T) {
	//s := []string{"a", "b", "c", "d", "e", "f", "g", "h", "i", "j"}
	//a := -1
	//fmt.Println(s[a])
	// 创建缓存（100万个数据点）
	const count = 1_000_000
	dataSize := int64(unsafe.Sizeof(FinancialData{})) * count
	_ = dataSize
	c, err := OpenCache[FinancialData]("market.dat")
	if err != nil {
		panic(err)
	}
	defer c.Close()
	err = c.Add(count)
	if err != nil {
		panic(err)
	}
	// 获取类型视图
	view, err := c.ToSlice()
	if err != nil {
		panic(err)
	}

	// 直接操作内存
	for i := range view {
		view[i] = FinancialData{
			Timestamp: int64(i)*60 + 1672502400,
			Open:      float64(i) + 100.0,
			High:      float64(i) + 100.5,
			Low:       float64(i) + 99.5,
			Close:     float64(i) + 100.2,
		}
	}
	// 更新数据长度
	_ = c.WriteData(0, unsafe.Slice((*byte)(unsafe.Pointer(&view[0])), len(view)*int(unsafe.Sizeof(FinancialData{}))))
	// 验证数据
	n := len(view)
	fmt.Printf("存储%d条行情数据\n", n)
	dd := view[n-1].Close
	fmt.Printf("最新收盘价: %.2f\n", dd)
}
