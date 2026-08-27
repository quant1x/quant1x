package id

import "math/bits"

type Option func(*HLC)

func WithClock(now func() int64) Option {
	return func(h *HLC) {
		if now != nil {
			h.now = now
		}
	}
}
func WithSeqSeed(seed uint16) Option   { return func(h *HLC) { h.seed = seed } }
func WithStateFile(path string) Option { return func(h *HLC) { h.store = newFileStateStore(path) } }
func WithStateSyncEvery(every uint32) Option {
	return func(h *HLC) {
		if every > 0 {
			h.syncEvery = every
		}
	}
}
func WithStateStrict() Option { return func(h *HLC) { h.strict = true } }
func WithNodeCount(count uint32) Option {
	return func(h *HLC) {
		if count < 1 {
			count = 1
		}
		h.seqBits = payloadBits - uint8(bits.Len(uint(count)))
		if h.seqBits < 4 {
			panic("distributed/id: node count is too large")
		}
	}
}
func WithSeqBits(seqBits uint8) Option {
	return func(h *HLC) {
		if seqBits < 4 || seqBits > payloadBits-1 {
			panic("distributed/id: invalid seq bits")
		}
		h.seqBits = seqBits
	}
}
