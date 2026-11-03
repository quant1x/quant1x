package config

// StrategyParameter mirrors key fields from C++ StrategyParameter
type StrategyParameter struct {
	Id                          uint64         `yaml:"id" json:"id" default:"1"`
	Auto                        bool           `yaml:"auto" json:"auto"`
	Name                        string         `yaml:"name" json:"name"`
	Flag                        string         `yaml:"flag" json:"flag"`
	Session                     TradingSession `yaml:"time" json:"time"`
	Weight                      float64        `yaml:"weight" json:"weight"`
	Total                       int            `yaml:"total" json:"total" default:"3"`
	PriceCageRatio              float64        `yaml:"price_cage_ratio" json:"price_cage_ratio"`
	MinimumPriceFluctuationUnit float64        `yaml:"minimum_price_fluctuation_unit" json:"minimum_price_fluctuation_unit"`
	FixedSlippageForSell        float64        `yaml:"fixed_slippage_for_sell" json:"fixed_slippage_for_sell" default:"0.01"`
	FeeMax                      float64        `yaml:"fee_max" json:"fee_max" default:"20000.00"`
	FeeMin                      float64        `yaml:"fee_min" json:"fee_min" default:"10000.00"`
	Sectors                     []string       `yaml:"sectors" json:"sectors"`
	IgnoreMarginTrading         bool           `yaml:"ignore_margin_trading" json:"ignore_margin_trading" default:"true"`
	HoldingPeriod               int            `yaml:"holding_period" json:"holding_period" default:"1"`
	SellStrategy                uint64         `yaml:"sell_strategy" json:"sell_strategy" default:"117"`
	FixedYield                  float64        `yaml:"fixed_yield" json:"fixed_yield"`
	TakeProfitRatio             float64        `yaml:"take_profit_ratio" json:"take_profit_ratio" default:"15.00"`
	StopLossRatio               float64        `yaml:"stop_loss_ratio" json:"stop_loss_ratio" default:"-2.00"`
	LowOpeningAmplitude         float64        `yaml:"low_opening_amplitude" json:"low_opening_amplitude" default:"0.618"`
	HighOpeningAmplitude        float64        `yaml:"high_opening_amplitude" json:"high_opening_amplitude" default:"0.382"`
	Rules                       RuleParameter  `yaml:"rules" json:"rules"`
	ExcludeCodes                []string       `yaml:"exclude_codes" json:"exclude_codes"`
}
