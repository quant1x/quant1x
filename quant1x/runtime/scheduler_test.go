package runtime

import (
	"sync/atomic"
	"testing"
	"time"
)

func TestNewAsyncScheduler(t *testing.T) {
	s := NewAsyncScheduler(2)
	if s == nil {
		t.Fatal("NewAsyncScheduler returned nil")
	}
	s.Stop()
}

func TestScheduleCron(t *testing.T) {
	s := NewAsyncScheduler(2)
	defer s.Stop()

	executed := int64(0)
	id, err := s.ScheduleCron("test", "@every 1s", func() { // every second
		atomic.AddInt64(&executed, 1)
	})
	if err != nil {
		t.Fatalf("ScheduleCron failed: %v", err)
	}
	if id <= 0 {
		t.Fatal("Invalid task ID")
	}

	// Wait for at least one execution
	time.Sleep(1500 * time.Millisecond)
	if atomic.LoadInt64(&executed) == 0 {
		t.Fatal("Task was not executed")
	}
}

func TestCancel(t *testing.T) {
	s := NewAsyncScheduler(2)
	defer s.Stop()

	executed := false
	id, err := s.ScheduleCron("test", "@every 1s", func() {
		executed = true
	})
	if err != nil {
		t.Fatalf("ScheduleCron failed: %v", err)
	}

	s.Cancel(id)
	time.Sleep(1500 * time.Millisecond)
	if executed {
		t.Fatal("Task was executed after cancel")
	}

	stats := s.GetStats()
	if stats.Canceled != 1 {
		t.Fatalf("Expected 1 canceled, got %d", stats.Canceled)
	}
}

func TestStop(t *testing.T) {
	s := NewAsyncScheduler(2)

	executed := int64(0)
	_, err := s.ScheduleCron("test", "@every 1s", func() {
		atomic.AddInt64(&executed, 1)
	})
	if err != nil {
		t.Fatalf("ScheduleCron failed: %v", err)
	}

	time.Sleep(500 * time.Millisecond)
	s.Stop()

	initialExecuted := atomic.LoadInt64(&executed)
	time.Sleep(1500 * time.Millisecond)
	finalExecuted := atomic.LoadInt64(&executed)

	if initialExecuted != finalExecuted {
		t.Fatal("Tasks executed after Stop")
	}
}

func TestTaskDelay(t *testing.T) {
	s := NewAsyncScheduler(2) // Use 2 workers to allow concurrent execution
	defer s.Stop()

	running := int64(0)
	executed := int64(0)

	id, err := s.ScheduleCron("test", "@every 1s", func() {
		atomic.AddInt64(&running, 1)
		time.Sleep(3 * time.Second) // Long task
		atomic.AddInt64(&executed, 1)
		atomic.AddInt64(&running, -1)
	})
	if err != nil {
		t.Fatalf("ScheduleCron failed: %v", err)
	}

	// Wait for first execution to start
	time.Sleep(1500 * time.Millisecond)
	if atomic.LoadInt64(&running) != 1 {
		t.Fatal("First task not running")
	}

	// Wait for next trigger (should be skipped)
	time.Sleep(4000 * time.Millisecond) // Total 5.5s
	if atomic.LoadInt64(&executed) != 1 {
		t.Fatalf("Expected 1 execution, got %d", atomic.LoadInt64(&executed))
	}

	stats := s.GetStats()
	if stats.SkippedRunning == 0 {
		t.Fatalf("Expected at least 1 skipped running, got %d", stats.SkippedRunning)
	}

	s.Cancel(id)
}

func TestInvalidCron(t *testing.T) {
	s := NewAsyncScheduler(2)
	defer s.Stop()

	_, err := s.ScheduleCron("test", "invalid", func() {})
	if err == nil {
		t.Fatal("Expected error for invalid cron")
	}
}

func TestStats(t *testing.T) {
	s := NewAsyncScheduler(2)
	defer s.Stop()

	id1, _ := s.ScheduleCron("test1", "@every 1s", func() {})
	_, _ = s.ScheduleCron("test2", "@every 1s", func() {})

	time.Sleep(1500 * time.Millisecond)

	stats := s.GetStats()
	if stats.Scheduled != 2 {
		t.Fatalf("Expected 2 scheduled, got %d", stats.Scheduled)
	}
	if stats.Executed < 2 {
		t.Fatalf("Expected at least 2 executed, got %d", stats.Executed)
	}

	s.Cancel(id1)
	if s.GetStats().Canceled != 1 {
		t.Fatal("Cancel not reflected in stats")
	}
}
