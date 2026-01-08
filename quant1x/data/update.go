package data

import (
	"encoding/csv"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strings"
	"sync"

	"gitee.com/quant1x/quant1x/quant1x/core"
	"gitee.com/quant1x/quant1x/quant1x/exchange"
	"gitee.com/quant1x/quant1x/quant1x/logger"
	"gitee.com/quant1x/quant1x/quant1x/markets"
)

const (
	lastUpdateTime = "22:00:00"
)

var allDateUpdateTimes = []string{"15:10:00", lastUpdateTime}

// GetVariablePath returns the variable directory for cache state files.
func GetVariablePath() string {
	return filepath.Join(core.DefaultCachePath(), "var")
}

// stateFilename returns the state filename for given date and phase timestamp.
func stateFilename(date string, ts exchange.Timestamp) string {
	// normalize date to only-date
	d := date
	if t, err := exchange.ParseTimestamp(date); err == nil {
		d = t.OnlyDate()
	}
	// compact time HHMMSS
	tm := ts.ToString("150405")
	if len(tm) >= 6 {
		tm = tm[:6]
	}
	name := fmt.Sprintf("update.%sT%s", d, tm)
	return filepath.Join(GetVariablePath(), name)
}

// CheckUpdateState returns true when update should proceed (state file absent).
func CheckUpdateState(date string, ts exchange.Timestamp) bool {
	fname := stateFilename(date, ts)
	if _, err := os.Stat(fname); err == nil {
		return false
	}
	return true
}

// DoneUpdate creates the state file to mark update finished.
func DoneUpdate(date string, ts exchange.Timestamp) error {
	fname := stateFilename(date, ts)
	if err := os.MkdirAll(filepath.Dir(fname), 0o755); err != nil {
		return err
	}
	f, err := os.Create(fname)
	if err != nil {
		return err
	}
	return f.Close()
}

// CleanExpiredStateFiles removes update.* files under var directory.
func CleanExpiredStateFiles() error {
	dir := GetVariablePath()
	entries, err := os.ReadDir(dir)
	if err != nil {
		// if directory not exists, nothing to clean
		if os.IsNotExist(err) {
			return nil
		}
		logger.Errorf("error reading state dir: %v", err)
		return err
	}
	for _, e := range entries {
		if strings.HasPrefix(e.Name(), "update.") {
			_ = os.Remove(filepath.Join(dir, e.Name()))
		}
	}
	return nil
}

// UpdateWithAdapters runs adapters (feature adapters produce CSV files).
// This is a simplified, concurrency-limited translation of cache.cpp logic.
func UpdateWithAdapters(adapters []DataAdapter, featureDate exchange.Timestamp) int {
	defaultConcurrency := runtime.NumCPU()
	if defaultConcurrency > 8 {
		defaultConcurrency = 8
	}

	// cache date uses featureDate as-is; consumers can pass next-trading-day if needed
	cacheDate := featureDate

	allCodes := markets.GetCodeList()

	count := len(adapters)
	for idx, adapter := range adapters {
		moduleName := fmt.Sprintf("%s(%d/%d)", adapter.Key(), idx+1, count)
		logger.Infof("[update] plugin=%s start", moduleName)

		// detect feature adapter
		var featureAdapter FeatureAdapter
		if fa, ok := adapter.(FeatureAdapter); ok {
			featureAdapter = fa
			featureAdapter.Init(featureDate)
		}

		// worker pool
		numThreads := defaultConcurrency
		jobs := make(chan exchange.SecurityCode)
		var wg sync.WaitGroup

		// results for feature adapter
		var resMutex sync.Mutex
		finalData := make([][]string, 0)

		// start workers
		for w := 0; w < numThreads; w++ {
			wg.Add(1)
			go func() {
				defer wg.Done()
				for code := range jobs {
					if featureAdapter != nil {
						// clone and update
						inst := featureAdapter.Clone()
						inst.Init(featureDate)
						inst.Update(code, featureDate)
						vals := inst.Values()
						if len(vals) > 0 {
							resMutex.Lock()
							finalData = append(finalData, vals)
							resMutex.Unlock()
						}
					} else {
						// base adapter just update
						adapter.Update(code, featureDate)
					}
				}
			}()
		}

		// feed jobs
		for _, code := range allCodes {
			jobs <- code
		}
		close(jobs)
		wg.Wait()

		// if feature adapter, write CSV
		if featureAdapter != nil {
			// sort finalData by code order to match markets.GetCodeList order
			order := make(map[string]int)
			for i, c := range allCodes {
				order[c.String()] = i
			}
			sort.SliceStable(finalData, func(i, j int) bool {
				if len(finalData[i]) == 0 || len(finalData[j]) == 0 {
					return false
				}
				ci := finalData[i][0]
				cj := finalData[j][0]
				return order[ci] < order[cj]
			})

			// prepend headers
			headers := featureAdapter.Headers()
			rows := make([][]string, 0, len(finalData)+1)
			rows = append(rows, headers)
			for _, r := range finalData {
				rows = append(rows, r)
			}

			fname := FeatureFilename(featureAdapter, cacheDate)
			if err := os.MkdirAll(filepath.Dir(fname), 0o755); err == nil {
				f, err := os.Create(fname)
				if err == nil {
					w := csv.NewWriter(f)
					_ = w.WriteAll(rows)
					w.Flush()
					_ = f.Close()
					logger.Infof("wrote %d rows to %s", len(rows), fname)
				} else {
					logger.Errorf("unable to create feature file %s: %v", fname, err)
				}
			} else {
				logger.Errorf("unable to mkdir for %s: %v", fname, err)
			}
		}

		logger.Infof("[update] plugin=%s end", moduleName)
	}

	return count
}

// UpdateAll decides if an update should run (based on trading day and times)
// and invokes UpdateWithAdapters for registered plugins.
func UpdateAll() {
	today := exchange.NowTimestamp().OnlyDate()
	lastTradingDay := exchange.LastTradingDay(exchange.NowTimestamp()).OnlyDate()
	currentTime := exchange.NowTimestamp().ToString("15:04:05")

	shouldUpdate := false
	var updatePhase exchange.Timestamp

	if today == lastTradingDay {
		for _, trigger := range allDateUpdateTimes {
			if currentTime >= trigger {
				if t, err := exchange.ParseTimeOnly(trigger); err == nil {
					updatePhase = t
					shouldUpdate = CheckUpdateState(today, updatePhase)
					if shouldUpdate {
						break
					}
				}
			}
		}
	} else {
		if currentTime >= lastUpdateTime {
			if t, err := exchange.ParseTimeOnly(lastUpdateTime); err == nil {
				updatePhase = t
				shouldUpdate = CheckUpdateState(today, updatePhase)
			}
		}
	}

	if shouldUpdate && !updatePhase.IsEmpty() {
		plugins := Plugins(0)
		// feature date: use NowTimestamp() as placeholder
		_ = UpdateWithAdapters(plugins, exchange.NowTimestamp())
		_ = DoneUpdate(today, updatePhase)
	}
}
