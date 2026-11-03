package config

// RuleParameter simplified; number ranges represented via NumberRange
type RuleParameter struct {
	SectorsFilter               bool        `yaml:"sectors_filter" json:"sectors_filter"`
	SectorsTopN                 int         `yaml:"sectors_top_n" json:"sectors_top_n" default:"3"`
	StockTopNInSector           int         `yaml:"stock_top_n_in_sector" json:"stock_top_n_in_sector" default:"5"`
	IgnoreRuleGroup             []int       `yaml:"ignore_rule_group" json:"ignore_rule_group"`
	IgnoreCodes                 []string    `yaml:"ignore_codes" json:"ignore_codes" default:"[\"sh68\",\"bj\"]"`
	MaximumIncreaseWithin5days  float64     `yaml:"maximum_increase_within_5d" json:"maximum_increase_within_5d" default:"20.00"`
	MaximumIncreaseWithin10days float64     `yaml:"maximum_increase_within_10d" json:"maximum_increase_within_10d" default:"70.00"`
	MaxReduceAmount             float64     `yaml:"max_reduce_amount" json:"max_reduce_amount" default:"-1000"`
	SafetyScore                 NumberRange `yaml:"safety_score" json:"safety_score" default:"80~"`
	VolumeRatio                 NumberRange `yaml:"volume_ratio" json:"volume_ratio" default:"0.382~2.800"`
	Capital                     NumberRange `yaml:"capital" json:"capital" default:"0.5~20"`
	MarketCap                   NumberRange `yaml:"market_cap" json:"market_cap" default:"4~600"`
	Price                       NumberRange `yaml:"price" json:"price" default:"2~"`
	OpenChangeRate              NumberRange `yaml:"open_change_rate" json:"open_change_rate"`
	OpenQuantityRatio           NumberRange `yaml:"open_quantity_ratio" json:"open_quantity_ratio"`
	OpenTurnZ                   NumberRange `yaml:"open_turn_z" json:"open_turn_z"`
	ChangeRate                  NumberRange `yaml:"change_rate" json:"change_rate"`
	Vix                         NumberRange `yaml:"vix" json:"vix"`
	TurnoverRate                NumberRange `yaml:"turnover_rate" json:"turnover_rate"`
	AmplitudeRatio              NumberRange `yaml:"amplitude_ratio" json:"amplitude_ratio"`
	BiddingVolume               NumberRange `yaml:"bidding_volume" json:"bidding_volume"`
	Sentiment                   NumberRange `yaml:"sentiment" json:"sentiment" default:"38.2~61.80"`
	GapDown                     bool        `yaml:"gap_down" json:"gap_down" default:"true"`
	CheckEPS                    bool        `yaml:"check_eps" json:"check_eps"`
	CheckBPS                    bool        `yaml:"check_bps" json:"check_bps"`
	CheckSafetyScore            bool        `yaml:"check_safety_score" json:"check_safety_score"`
	FinancingBalanceRatio       float64     `yaml:"financing_balance_ratio" json:"financing_balance_ratio" default:"10"`
	Verbose                     bool        `yaml:"verbose" json:"verbose"`
}
