package tdx

import (
	"testing"

	"github.com/quant1x/quant1x/quant1x/data"
)

func TestTdxProvider(t *testing.T) {
	code := "sh510050"
	api := GetTdxProvider()
	bars, err := api.GetBars(code, "", "", "1d", data.AdjustForward)
	if err != nil {
		t.Errorf("Failed to get bars: %v", err)
	}
	t.Logf("Bars: %v", bars)
}
