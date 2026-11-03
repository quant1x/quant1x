package config

import (
	"testing"

	"github.com/quant1x/gox/encoding/yaml"
)

// TestUnmarshalConfigDefaults 内置配置文件的解析
func TestUnmarshalConfigDefaults(t *testing.T) {
	src := `
trader:
  top_n: 6
  strategies:
    - id: 1
      total: 6
    - id: 117
      total: 0
`
	var cfg struct {
		Trader TraderParameter `yaml:"trader"`
	}
	if err := yaml.Unmarshal([]byte(src), &cfg); err != nil {
		t.Fatal(err)
	}
	// yaml 中显式配置的值, 以 yaml 为准
	if cfg.Trader.TopN != 6 {
		t.Errorf("trader.top_n, 期望6, 实际%d", cfg.Trader.TopN)
	}
	// yaml 中显式配置的零值, 不被 default 标签覆盖
	if cfg.Trader.Strategies[1].Total != 0 {
		t.Errorf("strategies[1].total, 期望0, 实际%d", cfg.Trader.Strategies[1].Total)
	}
	// 没有出现的字段, 取 default 标签的值
	if cfg.Trader.BuyAmountMax != 250000.00 {
		t.Errorf("trader.buy_amount_max, 期望250000, 实际%v", cfg.Trader.BuyAmountMax)
	}
	if cfg.Trader.Role != "manual" {
		t.Errorf("trader.role, 期望manual, 实际%s", cfg.Trader.Role)
	}
	// 切片元素的缺省字段逐项填充
	if cfg.Trader.Strategies[0].FeeMax != 20000.00 {
		t.Errorf("strategies[0].fee_max, 期望20000, 实际%v", cfg.Trader.Strategies[0].FeeMax)
	}
	// 嵌套结构中的区间默认值
	if cfg.Trader.Strategies[0].Rules.SafetyScore.Min != 80 {
		t.Errorf("strategies[0].rules.safety_score, 期望min=80, 实际%v", cfg.Trader.Strategies[0].Rules.SafetyScore)
	}
	if cfg.Trader.Strategies[0].Rules.MarketCap.Max != 600 {
		t.Errorf("strategies[0].rules.market_cap, 期望max=600, 实际%v", cfg.Trader.Strategies[0].Rules.MarketCap)
	}
	// 切片类型的默认值
	if len(cfg.Trader.Strategies[0].Rules.IgnoreCodes) != 2 {
		t.Errorf("strategies[0].rules.ignore_codes, 期望2项, 实际%v", cfg.Trader.Strategies[0].Rules.IgnoreCodes)
	}
}

// TestTradingSessionUnmarshalText default 标签的文本解析
func TestTradingSessionUnmarshalText(t *testing.T) {
	var ts TradingSession
	if err := ts.UnmarshalText([]byte("09:30:00~11:30:00")); err != nil {
		t.Fatal(err)
	}
	if len(ts.Sessions) != 1 || ts.Sessions[0].Begin != "09:30:00" || ts.Sessions[0].End != "11:30:00" {
		t.Errorf("sessions, 期望09:30:00~11:30:00, 实际%v", ts.Sessions)
	}
}

// TestTraderSessionParse 交易时段的标量解析
func TestTraderSessionParse(t *testing.T) {
	var sp StrategyParameter
	src := `
time: "09:27:00~09:50:00,13:00:00~14:56:30"
`
	if err := yaml.Unmarshal([]byte(src), &sp); err != nil {
		t.Fatal(err)
	}
	if len(sp.Session.Sessions) != 2 {
		t.Fatalf("time, 期望2个时段, 实际%d", len(sp.Session.Sessions))
	}
	if sp.Session.Sessions[0].Begin != "09:27:00" || sp.Session.Sessions[0].End != "09:50:00" {
		t.Errorf("time[0], 期望09:27:00~09:50:00, 实际%s", sp.Session.Sessions[0])
	}
}

// TestDefaultTraderParameter 无配置文件时的默认值
func TestDefaultTraderParameter(t *testing.T) {
	tp := defaultTraderParameter()
	if tp.TopN != 3 {
		t.Errorf("top_n, 期望3, 实际%d", tp.TopN)
	}
	if tp.CommissionMin != 5.0 {
		t.Errorf("commission_min, 期望5, 实际%v", tp.CommissionMin)
	}
	if tp.Role != "manual" {
		t.Errorf("role, 期望manual, 实际%s", tp.Role)
	}
}
