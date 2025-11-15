package runtime

import (
	"fmt"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

// PeriodicOnce provides a generic once-per-period initializer similar to C++ template PeriodicOnce<T>.
type PeriodicOnce[T any] struct {
	init func() T
	done uint32
	mu   sync.Mutex
	val  *T
}

// NewPeriodicOnce creates a PeriodicOnce with the provided init function.
func NewPeriodicOnce[T any](init func() T) *PeriodicOnce[T] {
	return &PeriodicOnce[T]{init: init}
}

// Get returns the initialized value, calling init exactly once per reset.
func (p *PeriodicOnce[T]) Get() T {
	if atomic.LoadUint32(&p.done) == 1 {
		return *p.val
	}
	p.mu.Lock()
	defer p.mu.Unlock()
	if atomic.LoadUint32(&p.done) == 1 {
		return *p.val
	}
	v := p.init()
	p.val = &v
	atomic.StoreUint32(&p.done, 1)
	return v
}

// Reset clears the stored value so Get() will call init again.
func (p *PeriodicOnce[T]) Reset() {
	p.mu.Lock()
	defer p.mu.Unlock()
	atomic.StoreUint32(&p.done, 0)
	p.val = nil
}

// RollingOnce provides a sliding-window once execution, matching C++ semantics in once.h.
type RollingOnce struct {
	done uint32 // 0 = not done, 1 = done
	mu   sync.Mutex

	// scheduling
	stopCh chan struct{}
	wg     sync.WaitGroup
}

// CreateSeconds constructs a RollingOnce that resets every `seconds` seconds.
func CreateSeconds(seconds int) *RollingOnce {
	if seconds <= 0 {
		seconds = 5
	}
	r := &RollingOnce{stopCh: make(chan struct{})}
	r.wg.Add(1)
	go r.secondsLoop(seconds)
	return r
}

// CreateDaily constructs a RollingOnce that resets daily at hour:minute (local time).
func CreateDaily(hour, minute int) *RollingOnce {
	r := &RollingOnce{stopCh: make(chan struct{})}
	r.wg.Add(1)
	go r.dailyLoop(hour, minute)
	return r
}

// CreateFromSpec tries to parse simple cron-like specs used in C++ header:
//   - "*/N * * * * *"  -> every N seconds
//   - "0 M H * * *" -> daily at H:M
//
// If spec cannot be parsed, it returns a RollingOnce without background scheduling.
func CreateFromSpec(spec string) *RollingOnce {
	spec = strings.TrimSpace(spec)
	// try seconds pattern: */N ... (we only look at prefix)
	if m := regexp.MustCompile(`^\*/(\d+)`).FindStringSubmatch(spec); len(m) == 2 {
		if n, err := strconv.Atoi(m[1]); err == nil && n > 0 {
			return CreateSeconds(n)
		}
	}
	// try daily pattern: 0 M H ...
	parts := strings.Fields(spec)
	if len(parts) >= 3 && parts[0] == "0" {
		if mi, err1 := strconv.Atoi(parts[1]); err1 == nil {
			if hi, err2 := strconv.Atoi(parts[2]); err2 == nil {
				return CreateDaily(hi, mi)
			}
		}
	}
	// fallback: no background loop
	return &RollingOnce{}
}

// Do runs f at most once in the current window. If f panics, done is set and the panic is rethrown.
func (r *RollingOnce) Do(f func()) {
	if atomic.LoadUint32(&r.done) == 1 {
		return
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	if atomic.LoadUint32(&r.done) == 1 {
		return
	}
	// execute and mark done even if panic occurs
	defer atomic.StoreUint32(&r.done, 1)
	defer func() {
		if p := recover(); p != nil {
			panic(p)
		}
	}()
	f()
}

// Note: C++ RollingOnce::Do takes a callable with no return value and does not return errors.
// We intentionally do not provide DoTry here to preserve C++ parity; callers should wrap error-returning
// functions in a closure passed to Do and handle errors inside that closure if needed.

// Reset clears the done flag; it waits for any in-progress Do to finish.
func (r *RollingOnce) Reset() {
	r.mu.Lock()
	atomic.StoreUint32(&r.done, 0)
	r.mu.Unlock()
}

// Close stops any background goroutines started by factory helpers and waits for them.
func (r *RollingOnce) Close() {
	if r.stopCh == nil {
		return
	}
	close(r.stopCh)
	r.wg.Wait()
}

func (r *RollingOnce) secondsLoop(seconds int) {
	defer r.wg.Done()
	ticker := time.NewTicker(time.Duration(seconds) * time.Second)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			r.Reset()
		case <-r.stopCh:
			return
		}
	}
}

func (r *RollingOnce) dailyLoop(hour, minute int) {
	defer r.wg.Done()
	for {
		now := time.Now()
		loc := now.Location()
		next := time.Date(now.Year(), now.Month(), now.Day(), hour, minute, 0, 0, loc)
		if !next.After(now) {
			next = next.Add(24 * time.Hour)
		}
		wait := time.Until(next)
		if wait <= 0 {
			wait = time.Second
		}
		select {
		case <-time.After(wait):
			r.Reset()
		case <-r.stopCh:
			return
		}
	}
}

// MarkRun persists the done marker by setting done=true. (No disk persistence in this translation.)
func (r *RollingOnce) MarkRun() {
	atomic.StoreUint32(&r.done, 1)
}

// String implements a debugging representation.
func (r *RollingOnce) String() string {
	return fmt.Sprintf("RollingOnce{done=%d}", atomic.LoadUint32(&r.done))
}
