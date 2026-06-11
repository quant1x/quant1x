package market

import (
	"fmt"
	"testing"
)

func TestGetCodeList(t *testing.T) {
	list := GetCodeList()
	fmt.Println("Total codes:", len(list))
}
