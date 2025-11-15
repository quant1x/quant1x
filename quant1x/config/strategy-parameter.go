package config

// StrategyParameter mirrors key fields from C++ StrategyParameter
type StrategyParameter struct {
	Id                          uint64         `yaml:"id" json:"id"`
	Auto                        bool           `yaml:"auto" json:"auto"`
	Name                        string         `yaml:"name" json:"name"`
	Flag                        string         `yaml:"flag" json:"flag"`
	Session                     TradingSession `yaml:"time" json:"time"`
	Weight                      float64        `yaml:"weight" json:"weight"`
	Total                       int            `yaml:"total" json:"total"`
	PriceCageRatio              float64        `yaml:"price_cage_ratio" json:"price_cage_ratio"`
	MinimumPriceFluctuationUnit float64        `yaml:"minimum_price_fluctuation_unit" json:"minimum_price_fluctuation_unit"`
	FixedSlippageForSell        float64        `yaml:"fixed_slippage_for_sell" json:"fixed_slippage_for_sell"`
	FeeMax                      float64        `yaml:"fee_max" json:"fee_max"`
	FeeMin                      float64        `yaml:"fee_min" json:"fee_min"`
	Sectors                     []string       `yaml:"sectors" json:"sectors"`
	IgnoreMarginTrading         bool           `yaml:"ignore_margin_trading" json:"ignore_margin_trading"`
	HoldingPeriod               int            `yaml:"holding_period" json:"holding_period"`
	SellStrategy                uint64         `yaml:"sell_strategy" json:"sell_strategy"`
	FixedYield                  float64        `yaml:"fixed_yield" json:"fixed_yield"`
	TakeProfitRatio             float64        `yaml:"take_profit_ratio" json:"take_profit_ratio"`
	StopLossRatio               float64        `yaml:"stop_loss_ratio" json:"stop_loss_ratio"`
	LowOpeningAmplitude         float64        `yaml:"low_opening_amplitude" json:"low_opening_amplitude"`
	HighOpeningAmplitude        float64        `yaml:"high_opening_amplitude" json:"high_opening_amplitude"`
	Rules                       RuleParameter  `yaml:"rules" json:"rules"`
	ExcludeCodes                []string       `yaml:"exclude_codes" json:"exclude_codes"`
}

// defaultStrategyParameter returns StrategyParameter with C++ defaults
func defaultStrategyParameter() StrategyParameter {
	return StrategyParameter{
		Id:                          1,
		Auto:                        false,
		Name:                        "",
		Flag:                        "",
		Weight:                      0.0,
		Total:                       3,
		PriceCageRatio:              0.00,
		MinimumPriceFluctuationUnit: 0.00,
		FixedSlippageForSell:        FixedSlippageForSell,
		FeeMax:                      20000.00,
		FeeMin:                      10000.00,
		Sectors:                     nil,
		IgnoreMarginTrading:         true,
		HoldingPeriod:               1,
		SellStrategy:                117,
		FixedYield:                  0.0,
		TakeProfitRatio:             15.0,
		StopLossRatio:               -2.0,
		LowOpeningAmplitude:         0.618,
		HighOpeningAmplitude:        0.382,
		Rules:                       defaultRuleParameter(),
		ExcludeCodes:                nil,
	}
}
