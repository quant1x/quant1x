package id

import (
	"context"
	"errors"
	"fmt"

	"github.com/quant1x/quant1x/quant1x/runtime"
)

type Generator struct {
	hlc        *HLC
	nodeID     uint32
	workerBits uint8
	seqBits    uint8
}

func NewGenerator(nodeID uint32, hlc *HLC) *Generator {
	if hlc == nil {
		panic("distributed/id: nil HLC")
	}
	workerBits := uint8(payloadBits) - hlc.SeqBits()
	if uint64(nodeID) >= uint64(1)<<workerBits {
		panic(fmt.Sprintf("distributed/id: nodeID %d out of range", nodeID))
	}
	return &Generator{hlc: hlc, nodeID: nodeID, workerBits: workerBits, seqBits: hlc.SeqBits()}
}

func (g *Generator) WorkerBits() uint8 { return g.workerBits }

func (g *Generator) Next() ID {
	physical, sequence := g.hlc.Now()
	elapsed := checkEpoch(physical - EpochMs)
	return ID(uint64(elapsed)<<payloadBits | uint64(g.nodeID)<<g.seqBits | uint64(sequence)&(uint64(1)<<g.seqBits-1))
}

// Serve 把发号器接入 ID 队列：在当前 goroutine 持续发号并写入 q，
// 队列满时阻塞等待消费腾位，直到 ctx 取消或队列关闭。
//
// 典型用法是后台 goroutine 批量预生成、消费端 TryPop 无锁取号，
// 把 HLC 互斥锁开销从每次发号摊薄为后台批量生产：
//
//	ctx, cancel := context.WithCancel(context.Background())
//	defer cancel()
//	go generator.Serve(ctx, queue) // 生产
//	id, err := queue.TryPop()      // 消费（无锁）
//
// 返回值：队列已关闭返回 nil（未消费的存量 ID 仍可由消费者排空）；
// ctx 取消返回 ctx.Err()。多个 goroutine 可并发对同一队列 Serve（MPMC 安全）。
func (g *Generator) Serve(ctx context.Context, q *Queue) error {
	if ctx == nil {
		panic("distributed/id: nil context")
	}
	if q == nil {
		panic("distributed/id: nil queue")
	}
	for {
		select {
		case <-ctx.Done():
			return ctx.Err()
		default:
		}
		if q.IsClosed() {
			return nil
		}

		id := g.Next()
		if err := q.Push(id); err != nil {
			if errors.Is(err, runtime.ErrClosed) {
				return nil
			}
			return err
		}
	}
}
