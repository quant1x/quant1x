package config

// TimeRange and TradingSession simplified versions for YAML parsing
type TimeRange struct {
	Begin string `yaml:"begin" json:"begin"`
	End   string `yaml:"end" json:"end"`
}

type TradingSession struct {
	Sessions []TimeRange `yaml:"sessions" json:"sessions"`
}
