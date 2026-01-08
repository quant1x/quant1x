package markets

import (
	"fmt"
	"testing"

	"gitee.com/quant1x/quant1x/quant1x/encoding"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/runtime"
)

func TestSecurityEntity(t *testing.T) {
	se := SecurityEntity{
		Code:           exchange.SecurityCode{Market: exchange.ExchangeIdShangHai, Symbol: "600000", Type: exchange.SecurityStock},
		Name:           "Test Security",
		LotSize:        100,
		PricePrecision: 2,
	}

	if se.Code.Market != exchange.ExchangeIdShangHai || se.Code.Symbol != "600000" || se.Code.Type != exchange.SecurityStock {
		t.Fatalf("expected code sh600000, got %v", se.Code)
	}
	if se.Name != "Test Security" {
		t.Fatalf("expected name 'Test Security', got %v", se.Name)
	}
	if se.LotSize != 100 {
		t.Fatalf("expected lot size 100, got %v", se.LotSize)
	}
	if se.PricePrecision != 2 {
		t.Fatalf("expected price precision 2, got %v", se.PricePrecision)
	}
}

func TestGetSecurityFilename(t *testing.T) {
	codeList := []SecurityEntity{
		{
			Code:           exchange.SecurityCode{Market: exchange.ExchangeIdShangHai, Symbol: "600000", Type: exchange.SecurityStock},
			Name:           "Test Security 1",
			LotSize:        100,
			PricePrecision: 2,
		},
		{
			Code:           exchange.SecurityCode{Market: exchange.ExchangeIdShenZhen, Symbol: "000001", Type: exchange.SecurityStock},
			Name:           "Test Security 2",
			LotSize:        100,
			PricePrecision: 3,
		},
		{
			Code:           exchange.SecurityCode{Market: exchange.ExchangeIdBeiJing, Symbol: "920000", Type: exchange.SecurityStock},
			Name:           "Test Security 3",
			LotSize:        100,
			PricePrecision: 4,
		},
	}
	filename := GetSecurityFilename()
	fmt.Println("Security filename:", filename)
	encoding.SlicesToCsv(filename, codeList, true)

	var tmpList []SecurityEntity
	encoding.CsvToSlices(filename, &tmpList)
	fmt.Println("tmpList:", tmpList)
}

func TestGetSecurityInfo_GetUpLimitAndCalcLimit(t *testing.T) {
	// backup and restore globals
	oldMap := securityMap
	oldOnce := securityRollingOnce
	defer func() {
		securityMap = oldMap
		securityRollingOnce = oldOnce
	}()

	// disable initSecurities by marking done
	securityRollingOnce = &runtime.RollingOnce{}
	securityRollingOnce.MarkRun()

	securityMap = map[string]*SecurityInfo{}
	// add a sample ShangHai security
	shKey := exchange.CorrectSecurityCode("600000")
	securityMap[shKey] = &SecurityInfo{Code: shKey, Name: "SH Test", LotSize: 100, PricePrecision: 2}

	// add a sample ShenZhen security with higher precision
	szKey := exchange.CorrectSecurityCode("300001")
	securityMap[szKey] = &SecurityInfo{Code: szKey, Name: "SZ Test", LotSize: 100, PricePrecision: 3}

	// ShangHai normal limit
	if r := GetUpLimitRate("600000"); r != 0.10 {
		t.Fatalf("expected 0.10 for sh600000, got %v", r)
	}

	// ShenZhen (prefix 30) high limit
	if r := GetUpLimitRate("300001"); r != 0.20 {
		t.Fatalf("expected 0.20 for 300001, got %v", r)
	}

	// BeiJing (prefix 920) special limit - use explicit market flag to ensure detection
	if r := GetUpLimitRate("bj920000"); r != 0.30 {
		t.Fatalf("expected 0.30 for bj920000, got %v", r)
	}

	// CalcLimitUpPrice uses PricePrecision from securityMap when available
	p := CalcLimitUpPrice("300001", 10.0) // 20% -> 12.0, precision 3 => 12.000
	if p != 12.0 {
		t.Fatalf("expected 12.0 for calc limit up, got %v", p)
	}

	// when precision differs
	p2 := CalcLimitUpPrice("600000", 10.0) // 10% -> 11.0, precision 2 => 11.00
	if p2 != 11.0 {
		t.Fatalf("expected 11.0 for calc limit up, got %v", p2)
	}

	// GetSecurityInfo should find entries by unnormalized code
	if si := GetSecurityInfo("600000"); si == nil || si.Code != shKey {
		t.Fatalf("GetSecurityInfo did not return expected security for 600000")
	}

	if si := GetSecurityInfo("sh600000"); si == nil || si.Code != shKey {
		t.Fatalf("GetSecurityInfo did not return expected security for sh600000")
	}
}

func TestInitSecurities(t *testing.T) {
	initSecurities()

	if len(securityMap) == 0 {
		t.Fatalf("expected non-empty securityMap after initSecurities")
	}

	// spot check a known security
	si := GetSecurityInfo("sh600000")
	if si == nil {
		t.Fatalf("expected to find security info for sh600000")
	}
	if si.Name == "" {
		t.Fatalf("expected non-empty name for sh600000")
	}
}
