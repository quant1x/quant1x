package runtime

import (
	"container/heap"
	"fmt"
	"runtime"
	"sync"
	"sync/atomic"
	"time"

	"github.com/robfig/cron/v3"
)

// AsyncScheduler provides asynchronous task scheduling with cron support.
// Tasks are delayed if the previous execution is still running.
type AsyncScheduler struct {
	pool          chan func() // worker pool channel
	running       int32       // atomic flag
	schedulerDone chan struct{}
	taskQueue     *taskHeap
	mutex         sync.Mutex
	cond          *sync.Cond
	nextID        int64 // atomic
	cronTasks     map[int64]*cronTask
	// stats
	stScheduled      int64
	stExecuted       int64
	stSkippedCancel  int64
	stSkippedRunning int64
	stRescheduled    int64
	stCanceled       int64
}

type cronTask struct {
	cronRunning bool
	canceled    bool
	expr        cron.Schedule
	task        func()
}

type scheduledTask struct {
	nextRun time.Time
	task    func()
	id      int64
	name    string
	index   int // for heap
}

// taskHeap implements heap.Interface
type taskHeap []*scheduledTask

func (h taskHeap) Len() int           { return len(h) }
func (h taskHeap) Less(i, j int) bool { return h[i].nextRun.Before(h[j].nextRun) }
func (h taskHeap) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
	h[i].index = i
	h[j].index = j
}
func (h *taskHeap) Push(x interface{}) {
	n := len(*h)
	item := x.(*scheduledTask)
	item.index = n
	*h = append(*h, item)
}
func (h *taskHeap) Pop() interface{} {
	old := *h
	n := len(old)
	item := old[n-1]
	old[n-1] = nil
	item.index = -1
	*h = old[0 : n-1]
	return item
}

// NewAsyncScheduler creates a new AsyncScheduler with the specified number of workers.
func NewAsyncScheduler(threadCount int) *AsyncScheduler {
	if threadCount <= 0 {
		threadCount = runtime.NumCPU()
	}
	s := &AsyncScheduler{
		pool:          make(chan func(), threadCount),
		running:       1,
		schedulerDone: make(chan struct{}),
		taskQueue:     &taskHeap{},
		cronTasks:     make(map[int64]*cronTask),
	}
	s.cond = sync.NewCond(&s.mutex)
	heap.Init(s.taskQueue)

	// start workers
	for i := 0; i < threadCount; i++ {
		go s.worker()
	}

	// start scheduler loop
	go s.schedulerLoop()

	return s
}

// ScheduleCron schedules a cron task.
func (s *AsyncScheduler) ScheduleCron(name, cronExpr string, task func()) (int64, error) {
	s.mutex.Lock()
	defer s.mutex.Unlock()

	if atomic.LoadInt32(&s.running) == 0 {
		return 0, fmt.Errorf("schedule_cron called after scheduler stopped")
	}

	expr, err := cron.ParseStandard(cronExpr)
	if err != nil {
		return 0, fmt.Errorf("invalid cron expression: %v", err)
	}

	id := atomic.AddInt64(&s.nextID, 1)
	firstRun := expr.Next(time.Now())
	s.cronTasks[id] = &cronTask{
		cronRunning: false,
		canceled:    false,
		expr:        expr,
		task:        task,
	}
	s.enqueueTask(&scheduledTask{
		nextRun: firstRun,
		task:    func() { s.executeCronTask(id, name) },
		id:      id,
		name:    name,
	})
	atomic.AddInt64(&s.stScheduled, 1)

	return id, nil
}

// Cancel cancels a scheduled task.
func (s *AsyncScheduler) Cancel(id int64) {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	if ct, ok := s.cronTasks[id]; ok {
		ct.canceled = true
		atomic.AddInt64(&s.stCanceled, 1)
	}
	s.cond.Broadcast()
}

// Stop stops the scheduler.
func (s *AsyncScheduler) Stop() {
	if !atomic.CompareAndSwapInt32(&s.running, 1, 0) {
		return
	}
	s.cond.Broadcast()
	close(s.schedulerDone)
	// wait for workers to finish
	close(s.pool)
}

// Stats holds scheduler statistics.
type Stats struct {
	Scheduled      int64
	Executed       int64
	SkippedCancel  int64
	SkippedRunning int64
	Rescheduled    int64
	Canceled       int64
}

// GetStats returns current statistics.
func (s *AsyncScheduler) GetStats() Stats {
	return Stats{
		Scheduled:      atomic.LoadInt64(&s.stScheduled),
		Executed:       atomic.LoadInt64(&s.stExecuted),
		SkippedCancel:  atomic.LoadInt64(&s.stSkippedCancel),
		SkippedRunning: atomic.LoadInt64(&s.stSkippedRunning),
		Rescheduled:    atomic.LoadInt64(&s.stRescheduled),
		Canceled:       atomic.LoadInt64(&s.stCanceled),
	}
}

func (s *AsyncScheduler) enqueueTask(task *scheduledTask) {
	heap.Push(s.taskQueue, task)
	s.cond.Broadcast()
}

func (s *AsyncScheduler) executeCronTask(id int64, name string) {
	var task func()
	var expr cron.Schedule
	var needReschedule bool

	s.mutex.Lock()
	ct, ok := s.cronTasks[id]
	if !ok {
		s.mutex.Unlock()
		return
	}
	if ct.canceled {
		s.mutex.Unlock()
		return
	}
	if ct.cronRunning {
		atomic.AddInt64(&s.stSkippedRunning, 1)
		s.mutex.Unlock()
		return
	}
	ct.cronRunning = true
	task = ct.task
	expr = ct.expr
	needReschedule = true
	s.mutex.Unlock()

	// Reschedule immediately after setting running
	s.rescheduleCron(id, name, expr)

	// execute task
	defer func() {
		if r := recover(); r != nil {
			// log error
		}
	}()
	task()

	s.mutex.Lock()
	if ct, ok := s.cronTasks[id]; ok {
		ct.cronRunning = false
		if ct.canceled || atomic.LoadInt32(&s.running) == 0 {
			needReschedule = false
		}
	} else {
		needReschedule = false
	}
	s.mutex.Unlock()

	if needReschedule {
		atomic.AddInt64(&s.stExecuted, 1)
		// No need to reschedule again, already done at start
	}
}

func (s *AsyncScheduler) rescheduleCron(id int64, name string, expr cron.Schedule) {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	if _, ok := s.cronTasks[id]; !ok || s.cronTasks[id].canceled {
		return
	}
	nextTime := expr.Next(time.Now())
	s.enqueueTask(&scheduledTask{
		nextRun: nextTime,
		task:    func() { s.executeCronTask(id, name) },
		id:      id,
		name:    name,
	})
	atomic.AddInt64(&s.stRescheduled, 1)
}

func (s *AsyncScheduler) schedulerLoop() {
	for atomic.LoadInt32(&s.running) == 1 {
		s.mutex.Lock()
		for s.taskQueue.Len() == 0 && atomic.LoadInt32(&s.running) == 1 {
			s.cond.Wait()
		}
		if atomic.LoadInt32(&s.running) == 0 {
			s.mutex.Unlock()
			break
		}
		task := heap.Pop(s.taskQueue).(*scheduledTask)
		now := time.Now()
		if now.Before(task.nextRun) {
			heap.Push(s.taskQueue, task)
			waitDur := task.nextRun.Sub(now)
			s.mutex.Unlock()
			select {
			case <-time.After(waitDur):
			case <-s.schedulerDone:
				return
			}
			continue
		}
		s.mutex.Unlock()

		if atomic.LoadInt32(&s.running) == 0 {
			break
		}
		// check cancel
		s.mutex.Lock()
		if ct, ok := s.cronTasks[task.id]; ok && ct.canceled {
			atomic.AddInt64(&s.stSkippedCancel, 1)
			s.mutex.Unlock()
			continue
		}
		s.mutex.Unlock()

		// submit to pool
		select {
		case s.pool <- task.task:
		case <-s.schedulerDone:
			return
		}
	}
}

func (s *AsyncScheduler) worker() {
	for {
		select {
		case task, ok := <-s.pool:
			if !ok {
				return
			}
			if atomic.LoadInt32(&s.running) == 1 {
				task()
			}
		case <-s.schedulerDone:
			return
		}
	}
}
