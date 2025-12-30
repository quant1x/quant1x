package provider

import (
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/data"
)

func TestTdxProvider(t *testing.T) {
	code := "sh000001"
	api := GetTdxProvider()
	klines, err := api.GetKLines(code, "", "", "1d", data.AdjustForward)
	if err != nil {
		t.Errorf("Failed to get KLines: %v", err)
	}
	t.Logf("KLines: %v", klines)
}
