package exchange

import (
	"testing"
)

func TestExchangeIdString_Valid(t *testing.T) {
	if ExchangeIdShenZhen.String() != "sz" {
		t.Fatalf("expected sz got %s", ExchangeIdShenZhen.String())
	}
	if ExchangeIdShangHai.String() != "sh" {
		t.Fatalf("expected sh got %s", ExchangeIdShangHai.String())
	}
	if ExchangeIdBeiJing.String() != "bj" {
		t.Fatalf("expected bj got %s", ExchangeIdBeiJing.String())
	}
	if ExchangeIdHongKong.String() != "hk" {
		t.Fatalf("expected hk got %s", ExchangeIdHongKong.String())
	}
	if ExchangeIdUSA.String() != "us" {
		t.Fatalf("expected us got %s", ExchangeIdUSA.String())
	}
}

func TestExchangeIdString_PanicOnUnknown(t *testing.T) {
	var x ExchangeId = 99
	if got := x.String(); got != "unknown" {
		t.Fatalf("expected unknown got %s", got)
	}
}

func TestExchangeInfoValidateAndNewExchange(t *testing.T) {
	e := NewExchange("sh", "Shanghai Stock Exchange", "desc", ExchangeIdShangHai)
	if e.IsActive != true {
		t.Fatalf("expected active true")
	}
	if err := e.Validate(); err != nil {
		t.Fatalf("unexpected validate error: %v", err)
	}

	bad := ExchangeInfo{ID: ExchangeIdShangHai, Code: "", Name: ""}
	if err := bad.Validate(); err == nil {
		t.Fatalf("expected validate error for empty fields")
	}
}

func TestSecurityCodeStringAndValidate(t *testing.T) {
	sc := SecurityCode{Exchange: ExchangeIdShangHai, Symbol: "600000"}
	if sc.String() != "sh600000" {
		t.Fatalf("expected sh600000 got %s", sc.String())
	}
	if err := sc.Validate(); err != nil {
		t.Fatalf("unexpected validate error: %v", err)
	}
	bad := SecurityCode{Exchange: ExchangeIdShangHai, Symbol: ""}
	if err := bad.Validate(); err == nil {
		t.Fatalf("expected validate error for empty symbol")
	}
}
