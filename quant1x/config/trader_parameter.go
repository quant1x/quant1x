package config

import (
	"os"
	"sync"

	"github.com/quant1x/quant1x/quant1x/core"
	"gopkg.in/yaml.v3"
)

// TraderParameter mirrors C++ config::TraderParameter (fields only).
type TraderParameter struct {
	AccountId                   string              `yaml:"account_id" json:"account_id"`
	OrderPath                   string              `yaml:"order_path" json:"order_path"`
	TopN                        int                 `yaml:"top_n" json:"top_n"`
	HaveETF                     bool                `yaml:"have_etf" json:"have_etf"`
	PriceCageRatio              float64             `yaml:"price_cage_ratio" json:"price_cage_ratio"`
	MinimumPriceFluctuationUnit float64             `yaml:"minimum_price_fluctuation_unit" json:"minimum_price_fluctuation_unit"`
	FixedSlippageForSell        float64             `yaml:"fixed_slippage_for_sell" json:"fixed_slippage_for_sell"`
	AnnualInterestRate          float64             `yaml:"annual_interest_rate" json:"annual_interest_rate"`
	StampDutyRateForBuy         float64             `yaml:"stamp_duty_rate_for_buy" json:"stamp_duty_rate_for_buy"`
	StampDutyRateForSell        float64             `yaml:"stamp_duty_rate_for_sell" json:"stamp_duty_rate_for_sell"`
	TransferRate                float64             `yaml:"transfer_rate" json:"transfer_rate"`
	CommissionRate              float64             `yaml:"commission_rate" json:"commission_rate"`
	CommissionMin               float64             `yaml:"commission_min" json:"commission_min"`
	PositionRatio               float64             `yaml:"position_ratio" json:"position_ratio"`
	KeepCash                    float64             `yaml:"keep_cash" json:"keep_cash"`
	BuyAmountMax                float64             `yaml:"buy_amount_max" json:"buy_amount_max"`
	BuyAmountMin                float64             `yaml:"buy_amount_min" json:"buy_amount_min"`
	Role                        string              `yaml:"role" json:"role"`
	ProxyUrl                    string              `yaml:"proxy_url" json:"proxy_url"`
	Strategies                  []StrategyParameter `yaml:"strategies" json:"strategies"`
	CancelSession               TradingSession      `yaml:"cancel" json:"cancel"`
	UndertakeRatio              float64             `yaml:"undertake_ratio" json:"undertake_ratio"`
}

var globalTraderOnce sync.Once
var globalTrader *TraderParameter

// loadTraderConfigFromYAML reads trader configuration from a yaml file.
func loadTraderConfigFromYAML(filename string) TraderParameter {
	// initialize with defaults matching C++
	tp := defaultTraderParameter()
	data, err := os.ReadFile(filename)
	if err != nil {
		return tp
	}

	var root yaml.Node
	if err := yaml.Unmarshal(data, &root); err != nil {
		return tp
	}

	// find top-level mapping node
	if len(root.Content) == 0 {
		return tp
	}
	doc := root.Content[0]
	if doc.Kind != yaml.MappingNode {
		return tp
	}

	var traderNode *yaml.Node
	for i := 0; i < len(doc.Content); i += 2 {
		key := doc.Content[i]
		val := doc.Content[i+1]
		if key.Value == "trader" {
			traderNode = val
			break
		}
	}
	if traderNode == nil {
		return tp
	}

	// Unmarshal all trader fields into tp (this may set Strategies but we'll replace it below if necessary)
	if b, err := yaml.Marshal(traderNode); err == nil {
		_ = yaml.Unmarshal(b, &tp)
	}

	// Manually parse strategies to ensure strategy defaults are applied per-item
	for i := 0; i < len(traderNode.Content); i += 2 {
		key := traderNode.Content[i]
		val := traderNode.Content[i+1]
		if key.Value != "strategies" {
			continue
		}
		if val.Kind != yaml.SequenceNode {
			continue
		}
		tp.Strategies = nil
		for _, item := range val.Content {
			sp := defaultStrategyParameter()
			if bs, err := yaml.Marshal(item); err == nil {
				_ = yaml.Unmarshal(bs, &sp)
			}
			tp.Strategies = append(tp.Strategies, sp)
		}
	}

	return tp
}

// defaultTraderParameter returns TraderParameter with C++ default values
func defaultTraderParameter() TraderParameter {
	return TraderParameter{
		AccountId:                   "888xxxxxxx",
		OrderPath:                   "",
		TopN:                        3,
		HaveETF:                     false,
		PriceCageRatio:              ValidDeclarationPriceRange,
		MinimumPriceFluctuationUnit: MinimumPriceFluctuationUnit,
		FixedSlippageForSell:        FixedSlippageForSell,
		AnnualInterestRate:          1.65,
		StampDutyRateForBuy:         0.0,
		StampDutyRateForSell:        0.0010,
		TransferRate:                0.0006,
		CommissionRate:              0.00025,
		CommissionMin:               5.0,
		PositionRatio:               0.5,
		KeepCash:                    10000.0,
		BuyAmountMax:                250000.0,
		BuyAmountMin:                1000.0,
		Role:                        "manual",
		ProxyUrl:                    "http://127.0.0.1:18168/qmt",
		Strategies:                  nil,
		UndertakeRatio:              0.8,
	}
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
