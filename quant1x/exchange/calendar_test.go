package exchange

import (
	"fmt"
	"testing"
)

func TestDateRange(t *testing.T) {
	start, _ := NewTimestampFromString("20240101")
	end, _ := NewTimestampFromString("20240131")
	dates := DateRange(start, end, false)
	for _, d := range dates {
		fmt.Println(d.OnlyDate())
	}
}
