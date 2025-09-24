package fpgrowth

import "fmt"

type PatternBase struct {
	Item           string
	SubPatternBase []itemCount
}

func (p *PatternBase) String() string {
	return fmt.Sprintf("item: %s, subpattern: %v", p.Item, p.SubPatternBase)
}
