package config

// RuleParameter simplified; number ranges represented via NumberRange
type RuleParameter struct {
	SectorsFilter               bool        `yaml:"sectors_filter" json:"sectors_filter"`
	SectorsTopN                 int         `yaml:"sectors_top_n" json:"sectors_top_n"`
	StockTopNInSector           int         `yaml:"stock_top_n_in_sector" json:"stock_top_n_in_sector"`
	IgnoreRuleGroup             []int       `yaml:"ignore_rule_group" json:"ignore_rule_group"`
	IgnoreCodes                 []string    `yaml:"ignore_codes" json:"ignore_codes"`
	MaximumIncreaseWithin5days  float64     `yaml:"maximum_increase_within_5d" json:"maximum_increase_within_5d"`
	MaximumIncreaseWithin10days float64     `yaml:"maximum_increase_within_10d" json:"maximum_increase_within_10d"`
	MaxReduceAmount             float64     `yaml:"max_reduce_amount" json:"max_reduce_amount"`
	SafetyScore                 NumberRange `yaml:"safety_score" json:"safety_score"`
	VolumeRatio                 NumberRange `yaml:"volume_ratio" json:"volume_ratio"`
	Capital                     NumberRange `yaml:"capital" json:"capital"`
	MarketCap                   NumberRange `yaml:"market_cap" json:"market_cap"`
	Price                       NumberRange `yaml:"price" json:"price"`
	OpenChangeRate              NumberRange `yaml:"open_change_rate" json:"open_change_rate"`
	OpenQuantityRatio           NumberRange `yaml:"open_quantity_ratio" json:"open_quantity_ratio"`
	OpenTurnZ                   NumberRange `yaml:"open_turn_z" json:"open_turn_z"`
	ChangeRate                  NumberRange `yaml:"change_rate" json:"change_rate"`
	Vix                         NumberRange `yaml:"vix" json:"vix"`
	TurnoverRate                NumberRange `yaml:"turnover_rate" json:"turnover_rate"`
	AmplitudeRatio              NumberRange `yaml:"amplitude_ratio" json:"amplitude_ratio"`
	BiddingVolume               NumberRange `yaml:"bidding_volume" json:"bidding_volume"`
	Sentiment                   NumberRange `yaml:"sentiment" json:"sentiment"`
	GapDown                     bool        `yaml:"gap_down" json:"gap_down"`
	CheckEPS                    bool        `yaml:"check_eps" json:"check_eps"`
	CheckBPS                    bool        `yaml:"check_bps" json:"check_bps"`
	CheckSafetyScore            bool        `yaml:"check_safety_score" json:"check_safety_score"`
	FinancingBalanceRatio       float64     `yaml:"financing_balance_ratio" json:"financing_balance_ratio"`
	Verbose                     bool        `yaml:"verbose" json:"verbose"`
}

// defaultRuleParameter returns RuleParameter initialized with C++ defaults
func defaultRuleParameter() RuleParameter {
	rp := RuleParameter{
		SectorsFilter:               false,
		SectorsTopN:                 3,
		StockTopNInSector:           5,
		IgnoreRuleGroup:             nil,
		IgnoreCodes:                 []string{"sh68", "bj"},
		MaximumIncreaseWithin5days:  20.00,
		MaximumIncreaseWithin10days: 70.00,
		MaxReduceAmount:             -1000,
		GapDown:                     true,
		CheckEPS:                    false,
		CheckBPS:                    false,
		CheckSafetyScore:            false,
		FinancingBalanceRatio:       10,
		Verbose:                     false,
	}

	// NumberRange defaults from C++ RuleParameter constructor
	rp.SafetyScore.ParseFromString("80~")
	rp.VolumeRatio.ParseFromString("0.382~2.800")
	rp.Capital.ParseFromString("0.5~20")
	rp.MarketCap.ParseFromString("4~600")
	rp.Price.ParseFromString("2~")
	rp.Sentiment.ParseFromString("38.2~61.80")

	return rp
}
