package config

import (
	"os"
	"sync"

	"github.com/quant1x/gox/encoding/yaml"
	"github.com/quant1x/quant1x/quant1x/core"
)

// TraderParameter mirrors C++ config::TraderParameter (fields only).
type TraderParameter struct {
	AccountId                   string              `yaml:"account_id" json:"account_id" default:"888xxxxxxx"`
	OrderPath                   string              `yaml:"order_path" json:"order_path"`
	TopN                        int                 `yaml:"top_n" json:"top_n" default:"3"`
	HaveETF                     bool                `yaml:"have_etf" json:"have_etf" default:"false"`
	PriceCageRatio              float64             `yaml:"price_cage_ratio" json:"price_cage_ratio" default:"0.02"`
	MinimumPriceFluctuationUnit float64             `yaml:"minimum_price_fluctuation_unit" json:"minimum_price_fluctuation_unit" default:"0.10"`
	FixedSlippageForSell        float64             `yaml:"fixed_slippage_for_sell" json:"fixed_slippage_for_sell" default:"0.01"`
	AnnualInterestRate          float64             `yaml:"annual_interest_rate" json:"annual_interest_rate" default:"1.65"`
	StampDutyRateForBuy         float64             `yaml:"stamp_duty_rate_for_buy" json:"stamp_duty_rate_for_buy" default:"0.0000"`
	StampDutyRateForSell        float64             `yaml:"stamp_duty_rate_for_sell" json:"stamp_duty_rate_for_sell" default:"0.0010"`
	TransferRate                float64             `yaml:"transfer_rate" json:"transfer_rate" default:"0.0006"`
	CommissionRate              float64             `yaml:"commission_rate" json:"commission_rate" default:"0.00025"`
	CommissionMin               float64             `yaml:"commission_min" json:"commission_min" default:"5.0000"`
	PositionRatio               float64             `yaml:"position_ratio" json:"position_ratio" default:"0.5000"`
	KeepCash                    float64             `yaml:"keep_cash" json:"keep_cash" default:"10000.00"`
	BuyAmountMax                float64             `yaml:"buy_amount_max" json:"buy_amount_max" default:"250000.00"`
	BuyAmountMin                float64             `yaml:"buy_amount_min" json:"buy_amount_min" default:"1000.00"`
	Role                        string              `yaml:"role" json:"role" default:"manual"`
	ProxyUrl                    string              `yaml:"proxy_url" json:"proxy_url" default:"http://127.0.0.1:18168/qmt"`
	Strategies                  []StrategyParameter `yaml:"strategies" json:"strategies"`
	CancelSession               TradingSession      `yaml:"cancel" json:"cancel"`
	UndertakeRatio              float64             `yaml:"undertake_ratio" json:"undertake_ratio" default:"0.8000"`
}

var globalTraderOnce sync.Once
var globalTrader *TraderParameter

// loadTraderConfigFromYAML reads trader configuration from a yaml file.
func loadTraderConfigFromYAML(filename string) TraderParameter {
	var cfg struct {
		Trader TraderParameter `yaml:"trader"`
	}
	data, err := os.ReadFile(filename)
	if err != nil {
		return defaultTraderParameter()
	}
	if err := yaml.Unmarshal(data, &cfg); err != nil {
		return defaultTraderParameter()
	}
	return cfg.Trader
}

// defaultTraderParameter returns TraderParameter filled from default tags
func defaultTraderParameter() TraderParameter {
	var tp TraderParameter
	_ = yaml.Unmarshal([]byte("{}"), &tp)
	return tp
}

// TraderConfig returns the global trader parameter (initialized once)
func TraderConfig() *TraderParameter {
	globalTraderOnce.Do(func() {
		// ensure config filename initialized
		_ = ConfigFilename()
		tp := loadTraderConfigFromYAML(core.GetConfigfilePath())
		globalTrader = &tp
	})
	return globalTrader
}
