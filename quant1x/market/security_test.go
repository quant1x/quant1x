package market

import (
	"fmt"
	"testing"
)

func TestInitSecurities(t *testing.T) {
	initSecurities()

	if len(instrumentsMap) == 0 {
		t.Fatalf("expected non-empty instrumentsMap after initSecurities")
	}

	// spot check a known security
	si := GetSecurityInfo("sh600000")
	if si == nil {
		t.Fatalf("expected to find security info for sh600000")
	}
	fmt.Println(si)
	if si.Name == "" {
		t.Fatalf("expected non-empty name for sh600000")
	}
}
