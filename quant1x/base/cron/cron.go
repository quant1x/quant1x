package cron

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"
)

// CronExpr represents a parsed cron expression
type CronExpr struct {
	seconds     uint64
	minutes     uint64
	hours       uint64
	daysOfMonth uint64
	months      uint64
	daysOfWeek  uint64
	expression  string
	traits      CronTraits
}

// CronTraits defines the valid ranges and names for cron fields
type CronTraits struct {
	MinSeconds, MaxSeconds int
	MinMinutes, MaxMinutes int
	MinHours, MaxHours     int
	MinDOM, MaxDOM         int
	MinMonths, MaxMonths   int
	MinDOW, MaxDOW         int
	Days                   []string
	Months                 []string
}

const (
	// Common constants
	minSeconds = 0
	maxSeconds = 59
	minMinutes = 0
	maxMinutes = 59
	minHours   = 0
	maxHours   = 23
	minDOM     = 1
	maxDOM     = 31

	// Standard traits constants
	minMonthsStandard = 1
	maxMonthsStandard = 12
	minDOWStandard    = 0
	maxDOWStandard    = 6

	// Oracle traits constants
	minMonthsOracle = 0
	maxMonthsOracle = 11
	minDOWOracle    = 1
	maxDOWOracle    = 7

	// Quartz traits constants
	minMonthsQuartz = 1
	maxMonthsQuartz = 12
	minDOWQuartz    = 1
	maxDOWQuartz    = 7

	// Other constants
	CronMaxYearsDiff = 4
	InvalidIndex     = -1
)

var (
	daysStandard   = []string{"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}
	monthsStandard = []string{"NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}

	daysOracle   = []string{"NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}
	monthsOracle = []string{"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}

	daysQuartz   = []string{"NIL", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"}
	monthsQuartz = []string{"NIL", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"}

	// StandardTraits: 0-59, 0-59, 0-23, 1-31, 1-12, 0-6 (SUN-SAT)
	StandardTraits = CronTraits{
		MinSeconds: minSeconds, MaxSeconds: maxSeconds,
		MinMinutes: minMinutes, MaxMinutes: maxMinutes,
		MinHours: minHours, MaxHours: maxHours,
		MinDOM: minDOM, MaxDOM: maxDOM,
		MinMonths: minMonthsStandard, MaxMonths: maxMonthsStandard,
		MinDOW: minDOWStandard, MaxDOW: maxDOWStandard,
		Days:   daysStandard,
		Months: monthsStandard,
	}

	// OracleTraits: 0-59, 0-59, 0-23, 1-31, 0-11, 1-7 (SUN-SAT)
	OracleTraits = CronTraits{
		MinSeconds: minSeconds, MaxSeconds: maxSeconds,
		MinMinutes: minMinutes, MaxMinutes: maxMinutes,
		MinHours: minHours, MaxHours: maxHours,
		MinDOM: minDOM, MaxDOM: maxDOM,
		MinMonths: minMonthsOracle, MaxMonths: maxMonthsOracle,
		MinDOW: minDOWOracle, MaxDOW: maxDOWOracle,
		Days:   daysOracle,
		Months: monthsOracle,
	}

	// QuartzTraits: 0-59, 0-59, 0-23, 1-31, 1-12, 1-7 (SUN-SAT)
	QuartzTraits = CronTraits{
		MinSeconds: minSeconds, MaxSeconds: maxSeconds,
		MinMinutes: minMinutes, MaxMinutes: maxMinutes,
		MinHours: minHours, MaxHours: maxHours,
		MinDOM: minDOM, MaxDOM: maxDOM,
		MinMonths: minMonthsQuartz, MaxMonths: maxMonthsQuartz,
		MinDOW: minDOWQuartz, MaxDOW: maxDOWQuartz,
		Days:   daysQuartz,
		Months: monthsQuartz,
	}
)

// MakeCron parses a cron expression string using StandardTraits
func MakeCron(expr string) (*CronExpr, error) {
	return MakeCronWithTraits(expr, StandardTraits)
}

// MakeCronWithTraits parses a cron expression string using specific traits
func MakeCronWithTraits(expr string, traits CronTraits) (*CronExpr, error) {
	fields := strings.Fields(expr)
	if len(fields) != 6 {
		return nil, errors.New("cron expression must have six fields")
	}

	c := &CronExpr{expression: expr, traits: traits}
	var err error

	c.seconds, err = parseField(fields[0], traits.MinSeconds, traits.MaxSeconds, nil)
	if err != nil {
		return nil, fmt.Errorf("seconds: %w", err)
	}

	c.minutes, err = parseField(fields[1], traits.MinMinutes, traits.MaxMinutes, nil)
	if err != nil {
		return nil, fmt.Errorf("minutes: %w", err)
	}

	c.hours, err = parseField(fields[2], traits.MinHours, traits.MaxHours, nil)
	if err != nil {
		return nil, fmt.Errorf("hours: %w", err)
	}

	// Handle ? in DOM
	domField := fields[3]
	if strings.Contains(domField, "?") {
		domField = strings.ReplaceAll(domField, "?", "*")
	}
	c.daysOfMonth, err = parseField(domField, traits.MinDOM, traits.MaxDOM, nil)
	if err != nil {
		return nil, fmt.Errorf("days of month: %w", err)
	}

	c.months, err = parseField(fields[4], traits.MinMonths, traits.MaxMonths, traits.Months)
	if err != nil {
		return nil, fmt.Errorf("months: %w", err)
	}

	// Handle ? in DOW
	dowField := fields[5]
	if strings.Contains(dowField, "?") {
		dowField = strings.ReplaceAll(dowField, "?", "*")
	}
	c.daysOfWeek, err = parseField(dowField, traits.MinDOW, traits.MaxDOW, traits.Days)
	if err != nil {
		return nil, fmt.Errorf("days of week: %w", err)
	}

	return c, nil
}

func parseField(field string, minVal, maxVal int, replacements []string) (uint64, error) {
	if len(field) > 0 && field[len(field)-1] == ',' {
		return 0, errors.New("value cannot end with comma")
	}

	var bits uint64

	// Replace names
	if replacements != nil {
		field = strings.ToUpper(field)
		for i, name := range replacements {
			if name == "NIL" {
				continue
			}
			if strings.Contains(field, name) {
				field = strings.ReplaceAll(field, name, strconv.Itoa(i))
			}
		}
	}

	parts := strings.Split(field, ",")
	for _, part := range parts {
		if strings.Contains(part, "/") {
			subparts := strings.Split(part, "/")
			if len(subparts) != 2 {
				return 0, errors.New("incrementer must have two fields")
			}
			rangeStr := subparts[0]
			deltaStr := subparts[1]

			start, end, err := parseRange(rangeStr, minVal, maxVal)
			if err != nil {
				return 0, err
			}

			if !strings.Contains(rangeStr, "-") {
				end = maxVal
			}

			delta, err := strconv.Atoi(deltaStr)
			if err != nil || delta <= 0 {
				return 0, errors.New("incrementer must be a positive value")
			}

			for i := start; i <= end; i += delta {
				bits |= (1 << (uint(i - minVal)))
			}
		} else {
			start, end, err := parseRange(part, minVal, maxVal)
			if err != nil {
				return 0, err
			}
			for i := start; i <= end; i++ {
				bits |= (1 << (uint(i - minVal)))
			}
		}
	}
	return bits, nil
}

func parseRange(field string, minVal, maxVal int) (int, int, error) {
	if field == "*" {
		return minVal, maxVal, nil
	}
	if !strings.Contains(field, "-") {
		val, err := strconv.Atoi(field)
		if err != nil {
			return 0, 0, err
		}
		if val < minVal || val > maxVal {
			return 0, 0, errors.New("value out of range")
		}
		return val, val, nil
	}
	parts := strings.Split(field, "-")
	if len(parts) != 2 {
		return 0, 0, errors.New("invalid range")
	}
	start, err := strconv.Atoi(parts[0])
	if err != nil {
		return 0, 0, err
	}
	end, err := strconv.Atoi(parts[1])
	if err != nil {
		return 0, 0, err
	}

	if start > end {
		return 0, 0, errors.New("range start exceeds end")
	}
	if start < minVal || end > maxVal {
		return 0, 0, errors.New("range out of bounds")
	}
	return start, end, nil
}

// Next returns the next time the cron expression is satisfied after the given time
func (c *CronExpr) Next(t time.Time) time.Time {
	// Truncate to seconds as cron doesn't support sub-seconds
	next := t.Truncate(time.Second)

	// If next <= t, add 1 second to start search
	if !next.After(t) {
		next = next.Add(time.Second)
	}

	// Limit search to 4 years like C++
	maxYear := next.Year() + CronMaxYearsDiff

	for {
		if next.Year() > maxYear {
			return time.Time{} // Not found
		}

		// Check Month
		month := int(next.Month())
		// Adjust month for 0-based traits (Oracle)
		// Go time.Month is 1-12.
		// If traits.MinMonths is 0, then Jan=0.
		// If traits.MinMonths is 1, then Jan=1.
		// We need to map Go month (1-12) to Cron month index.
		// Cron index = GoMonth - 1 + MinMonths?
		// Standard: Jan=1. Min=1. Index = 1. Correct.
		// Oracle: Jan=0. Min=0. Index = 0. Correct.
		// Wait, Go Month is type int 1-12.
		// If traits.MinMonths == 0, then Jan should be 0.
		// So we need to check bit (month - 1 + traits.MinMonths) - traits.MinMonths?
		// No, isSet checks bit at (val - min).
		// So we just need the value in the cron's domain.
		// If Oracle (0-11), Jan is 0. Go gives 1. So we use 1-1 = 0.
		// If Standard (1-12), Jan is 1. Go gives 1. So we use 1.
		// So: cronMonth = month - 1 + c.traits.MinMonths
		cronMonth := month - 1 + c.traits.MinMonths

		if !isSet(c.months, cronMonth-c.traits.MinMonths) {
			// Find next month bit
			nextMonthBit := findNextSetBit(c.months, cronMonth-c.traits.MinMonths, c.traits.MaxMonths-c.traits.MinMonths+1)
			if nextMonthBit == InvalidIndex {
				next = time.Date(next.Year()+1, 1, 1, 0, 0, 0, 0, next.Location())
				continue
			}
			// Convert bit index back to Go Month
			// bit index = val - min
			// val = bit + min
			// GoMonth = val + 1 - min (inverse of above) -> Wait.
			// cronMonth = bit + min.
			// GoMonth = cronMonth + 1 - min.
			targetGoMonth := time.Month(nextMonthBit + c.traits.MinMonths + 1 - c.traits.MinMonths)
			// Simplified: targetGoMonth = bit + 1
			next = time.Date(next.Year(), targetGoMonth, 1, 0, 0, 0, 0, next.Location())
			continue
		}

		// Check Day of Month and Day of Week
		dom := next.Day()
		dow := int(next.Weekday()) // 0=Sun

		// Adjust DOW for traits
		// Standard: Sun=0. Go=0.
		// Oracle/Quartz: Sun=1. Go=0.
		// So if MinDOW == 1, we need to add 1 to Go's DOW.
		cronDOW := dow
		if c.traits.MinDOW == 1 {
			cronDOW = dow + 1
		}

		domMatch := isSet(c.daysOfMonth, dom-c.traits.MinDOM)
		dowMatch := isSet(c.daysOfWeek, cronDOW-c.traits.MinDOW)

		if !domMatch || !dowMatch {
			next = time.Date(next.Year(), next.Month(), next.Day()+1, 0, 0, 0, 0, next.Location())
			continue
		}

		// Check Hour
		hour := next.Hour()
		if !isSet(c.hours, hour-c.traits.MinHours) {
			nextHour := findNextSetBit(c.hours, hour-c.traits.MinHours, c.traits.MaxHours-c.traits.MinHours+1)
			if nextHour == InvalidIndex {
				next = time.Date(next.Year(), next.Month(), next.Day()+1, 0, 0, 0, 0, next.Location())
				continue
			}
			next = time.Date(next.Year(), next.Month(), next.Day(), nextHour+c.traits.MinHours, 0, 0, 0, next.Location())
			continue
		}

		// Check Minute
		minute := next.Minute()
		if !isSet(c.minutes, minute-c.traits.MinMinutes) {
			nextMinute := findNextSetBit(c.minutes, minute-c.traits.MinMinutes, c.traits.MaxMinutes-c.traits.MinMinutes+1)
			if nextMinute == InvalidIndex {
				next = time.Date(next.Year(), next.Month(), next.Day(), next.Hour()+1, 0, 0, 0, next.Location())
				continue
			}
			next = time.Date(next.Year(), next.Month(), next.Day(), next.Hour(), nextMinute+c.traits.MinMinutes, 0, 0, next.Location())
			continue
		}

		// Check Second
		second := next.Second()
		if !isSet(c.seconds, second-c.traits.MinSeconds) {
			nextSecond := findNextSetBit(c.seconds, second-c.traits.MinSeconds, c.traits.MaxSeconds-c.traits.MinSeconds+1)
			if nextSecond == InvalidIndex {
				next = time.Date(next.Year(), next.Month(), next.Day(), next.Hour(), next.Minute()+1, 0, 0, next.Location())
				continue
			}
			next = time.Date(next.Year(), next.Month(), next.Day(), next.Hour(), next.Minute(), nextSecond+c.traits.MinSeconds, 0, next.Location())
			continue
		}

		return next
	}
}

func isSet(bits uint64, index int) bool {
	return (bits & (1 << uint(index))) != 0
}

func findNextSetBit(bits uint64, current int, max int) int {
	for i := current + 1; i < max; i++ {
		if isSet(bits, i) {
			return i
		}
	}
	return InvalidIndex
}

func (c *CronExpr) String() string {
	return c.expression
}

// Equal checks if two CronExpr are equal
func (c *CronExpr) Equal(other *CronExpr) bool {
	if c == nil || other == nil {
		return c == other
	}
	return c.seconds == other.seconds &&
		c.minutes == other.minutes &&
		c.hours == other.hours &&
		c.daysOfMonth == other.daysOfMonth &&
		c.months == other.months &&
		c.daysOfWeek == other.daysOfWeek
}

// BitsetString returns the bitset representation of the cron expression
// This matches the behavior of to_string(cronexpr) in C++
func (c *CronExpr) BitsetString() string {
	return fmt.Sprintf("%s %s %s %s %s %s",
		bitsetToString(c.seconds, 60),
		bitsetToString(c.minutes, 60),
		bitsetToString(c.hours, 24),
		bitsetToString(c.daysOfMonth, 31),
		bitsetToString(c.months, 12),
		bitsetToString(c.daysOfWeek, 7))
}

func bitsetToString(bits uint64, size int) string {
	var sb strings.Builder
	for i := size - 1; i >= 0; i-- {
		if isSet(bits, i) {
			sb.WriteByte('1')
		} else {
			sb.WriteByte('0')
		}
	}
	return sb.String()
}
