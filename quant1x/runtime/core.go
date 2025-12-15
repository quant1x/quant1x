package runtime

// globalScheduler is the global AsyncScheduler instance.
var globalScheduler *AsyncScheduler

// init initializes the global scheduler.
func init() {
	globalScheduler = NewAsyncScheduler(0) // 0 uses default runtime.NumCPU()
	// Register a shutdown hook to stop the global scheduler when the process exits
	RegisterHook("global-scheduler", func() {
		globalScheduler.Stop()
	})
}

// ScheduleCron schedules a cron task using the global scheduler.
func ScheduleCron(name, cronExpr string, task func()) (int64, error) {
	return globalScheduler.ScheduleCron(name, cronExpr, task)
}

// Cancel cancels a scheduled task using the global scheduler.
func Cancel(id int64) {
	globalScheduler.Cancel(id)
}

// GetStats returns statistics from the global scheduler.
func GetStats() Stats {
	return globalScheduler.GetStats()
}

// Stop stops the global scheduler.
func Stop() {
	globalScheduler.Stop()
}

// Example usage or additional functions can be added here to match C++ cron functionality.

// For compatibility with C++ core.cpp, provide a function to get the global scheduler.
func GlobalScheduler() *AsyncScheduler {
	return globalScheduler
}
