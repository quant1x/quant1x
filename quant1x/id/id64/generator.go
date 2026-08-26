package id64

import "fmt"

// Generator 将 HLC 推进结果与 nodeID 组装为 64 位 ID。
type Generator struct {
	hlc        *HLC
	nodeID     uint32
	workerBits uint8
	seqBits    uint8
}

// NewGenerator 构造生成器。
//
// nodeID 必须小于 2^workerBits（workerBits = 22 - hlc.SeqBits()，
// 由构造 HLC 时的节点总数决定）。
func NewGenerator(nodeID uint32, hlc *HLC) *Generator {
	if hlc == nil {
		panic("id64: nil HLC")
	}
	seqBits := hlc.SeqBits()
	workerBits := uint8(payloadBits) - seqBits

	if uint64(nodeID) >= uint64(1)<<workerBits {
		panic(fmt.Sprintf("id64: nodeID %d 超出 %d 位节点位宽", nodeID, workerBits))
	}

	return &Generator{
		hlc:        hlc,
		nodeID:     nodeID,
		workerBits: workerBits,
		seqBits:    seqBits,
	}
}

// WorkerBits 返回节点位宽。
func (g *Generator) WorkerBits() uint8 {
	return g.workerBits
}

// Next 返回下一个 64 位 ID。
func (g *Generator) Next() ID {
	physical, seq := g.hlc.Now()
	elapsed := checkEpoch(physical - EpochMs)
	return ID(uint64(elapsed)<<payloadBits |
		uint64(g.nodeID)<<g.seqBits |
		uint64(seq)&(uint64(1)<<g.seqBits-1))
}
