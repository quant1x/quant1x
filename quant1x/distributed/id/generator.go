package id

import (
	"context"
	"errors"
	"fmt"
	goruntime "runtime"

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
// 队列满时以非阻塞方式重试并让出时间片，每轮检查 ctx 取消与队列关闭，
// 直到 ctx 取消或队列关闭（队满不会导致取消信号被延迟响应）。
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
		// 队列满时重试同一个 ID (非阻塞 TryPush), 每轮回到循环顶部检查取消/关闭.
		// 早期实现使用阻塞式 Push, 队列满且消费者停止时会卡在 Push 内部,
		// 无法响应 ctx 取消 (TestGeneratorServeDrainAfterCancel 即因此挂起);
		// 该缺陷已在 Rust/C++/Python 版规避, 此处同步修正以统一四语言语义.
		for {
			select {
			case <-ctx.Done():
				return ctx.Err()
			default:
			}
			if q.IsClosed() {
				return nil
			}
			err := q.TryPush(id)
			if err == nil {
				break
			}
			if errors.Is(err, runtime.ErrClosed) {
				return nil
			}
			if errors.Is(err, runtime.ErrQueueFull) {
				// 让出时间片, 等待消费者腾位后重试
				goruntime.Gosched()
				continue
			}
			return err
		}
	}
}
