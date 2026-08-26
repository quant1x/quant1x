package id

type Generator struct {
	hlc    *HLC
	nodeID uint32
}

func NewGenerator(nodeID uint32, hlc *HLC) *Generator {
	if hlc == nil {
		panic("hlcid: nil HLC")
	}

	return &Generator{
		hlc:    hlc,
		nodeID: nodeID,
	}
}

// Next 使用同一次 HLC.Now() 返回的 hlc 和 seq 组装 ID。
func (g *Generator) Next() Uint128 {
	hlc, seq := g.hlc.Now()

	return Uint128{
		Hi: hlc,
		Lo: (uint64(g.nodeID) << 32) | uint64(seq),
	}
}
