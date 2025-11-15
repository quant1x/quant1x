package exchange

import (
	"fmt"
	"testing"
	//_ "gitee.com/quant1x/quant1x/quant1x/config"
)

func TestDateRange(t *testing.T) {
	start, _ := NewTimestampFromString("20240101")
	end, _ := NewTimestampFromString("20240131")
	dates := date_range(start, end, false)
	for _, d := range dates {
		fmt.Println(d.OnlyDate())
	}
}
