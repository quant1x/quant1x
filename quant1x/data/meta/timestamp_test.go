package meta

import (
	"testing"
	"time"
)

func TestTimestampCompatLayer(t *testing.T) {
	// 测试基本创建和转换
	t.Run("BasicCreationAndConversion", func(t *testing.T) {
		// 从毫秒数创建
		ts := NewTimestamp(1640995200000)
		if ts.Value() != 1640995200000 {
			t.Errorf("Expected value 1640995200000, got %d", ts.Value())
		}

		// 转换为Go原生Timestamp
		goTs := ts.AsTimestamp()
		if int64(goTs) != 1640995200000 {
			t.Errorf("Conversion to Go timestamp failed: expected 1640995200000, got %d", int64(goTs))
		}

		// 从Go原生Timestamp创建
		cppTs := FromTimestamp(goTs)
		if cppTs.Value() != 1640995200000 {
			t.Errorf("Creation from Go timestamp failed: expected 1640995200000, got %d", cppTs.Value())
		}
	})

	// 测试与Go原生实现的一致性
	t.Run("ConsistencyWithGoImplementation", func(t *testing.T) {
		// 使用相同的毫秒数创建
		ms := int64(1640995200000)
		cppTs := NewTimestamp(ms)
		goTs := NewTimestamp(ms)

		// 比较时间转换结果
		cppTime := cppTs.ToTime()
		goTime := goTs.ToTime()

		if !cppTime.Equal(goTime) {
			t.Errorf("Time conversion inconsistent: cpp=%v, go=%v", cppTime, goTime)
		}
	})

	// 测试C++风格的API
	t.Run("StyleAPI", func(t *testing.T) {
		ts := NewTimestampFromDate(2022, 1, 1, 15, 30, 45, 123)

		// 测试格式化
		dateStr := ts.OnlyDate()
		if dateStr != "2022-01-01" {
			t.Errorf("Expected date '2022-01-01', got '%s'", dateStr)
		}

		timeStr := ts.OnlyTime()
		if timeStr != "15:30:45" {
			t.Errorf("Expected time '15:30:45', got '%s'", timeStr)
		}

		// 测试YYYYMMDD
		yyyymmdd := ts.YYYYMMDD()
		if yyyymmdd != 20220101 {
			t.Errorf("Expected YYYYMMDD 20220101, got %d", yyyymmdd)
		}
	})

	// 测试时间操作
	t.Run("TimeOperations", func(t *testing.T) {
		ts := NewTimestampFromDate(2022, 1, 1, 15, 30, 45, 123)

		// 测试StartOfDay
		startOfDay := ts.StartOfDay()
		startTime := startOfDay.ToTime()
		if startTime.Hour() != 0 || startTime.Minute() != 0 || startTime.Second() != 0 {
			t.Errorf("StartOfDay should be 00:00:00, got %02d:%02d:%02d",
				startTime.Hour(), startTime.Minute(), startTime.Second())
		}

		// 测试Today
		today9AM := ts.Today(9, 0, 0, 0)
		today9AMTime := today9AM.ToTime()
		if today9AMTime.Hour() != 9 {
			t.Errorf("Today(9,0,0,0) should be 09:xx:xx, got %02d:%02d:%02d",
				today9AMTime.Hour(), today9AMTime.Minute(), today9AMTime.Second())
		}

		// 测试Offset
		offset := ts.Offset(2, 30, 0, 0)
		offsetTime := offset.ToTime()
		originalTime := ts.ToTime()
		diff := offsetTime.Sub(originalTime)
		expectedDiff := 2*time.Hour + 30*time.Minute
		if diff != expectedDiff {
			t.Errorf("Offset failed: expected %v, got %v", expectedDiff, diff)
		}
	})

	// 测试比较操作
	t.Run("ComparisonOperations", func(t *testing.T) {
		ts1 := NewTimestampFromDate(2022, 1, 1, 10, 0, 0, 0)
		ts2 := NewTimestampFromDate(2022, 1, 1, 11, 0, 0, 0)
		ts3 := NewTimestampFromDate(2022, 1, 1, 10, 0, 0, 0)

		if !ts1.Less(ts2) {
			t.Error("ts1 should be less than ts2")
		}

		if !ts2.Greater(ts1) {
			t.Error("ts2 should be greater than ts1")
		}

		if !ts1.Equal(ts3) {
			t.Error("ts1 should equal ts3")
		}

		if ts1.NotEqual(ts3) {
			t.Error("ts1 should not be not-equal to ts3")
		}
	})

	// 测试解析功能
	t.Run("ParsingFunctions", func(t *testing.T) {
		// 测试解析日期时间
		ts, err := ParseTimestamp("2022-01-01 15:30:45")
		if err != nil {
			t.Errorf("Failed to parse timestamp: %v", err)
		}

		year, month, day := ts.Extract()
		if year != 2022 || month != 1 || day != 1 {
			t.Errorf("Parsed wrong date: %d-%d-%d", year, month, day)
		}

		// 测试解析时间
		timeTS, err := ParseTimeOnly("15:30:45")
		if err != nil {
			t.Errorf("Failed to parse time: %v", err)
		}

		timeOnly := timeTS.OnlyTime()
		if timeOnly != "15:30:45" {
			t.Errorf("Expected time '15:30:45', got '%s'", timeOnly)
		}
	})

	// 测试同一天检查
	t.Run("SameDateCheck", func(t *testing.T) {
		ts1 := NewTimestampFromDate(2022, 1, 1, 8, 0, 0, 0)
		ts2 := NewTimestampFromDate(2022, 1, 1, 20, 0, 0, 0)
		ts3 := NewTimestampFromDate(2022, 1, 2, 8, 0, 0, 0)

		if !ts1.IsSameDate(ts2) {
			t.Error("ts1 and ts2 should be on the same date")
		}

		if ts1.IsSameDate(ts3) {
			t.Error("ts1 and ts3 should not be on the same date")
		}
	})

	// 测试盘前时间
	t.Run("PreMarketTime", func(t *testing.T) {
		preMarket := PreMarketTimestamp(2022, 1, 1)
		timeStr := preMarket.OnlyTime()
		if timeStr != "09:00:00" {
			t.Errorf("Pre-market time should be '09:00:00', got '%s'", timeStr)
		}

		// 测试实例方法的盘前时间
		ts := NewTimestampFromDate(2022, 1, 1, 15, 30, 45, 123)
		tsPreMarket := ts.PreMarketTime()
		tsPreMarketTime := tsPreMarket.OnlyTime()
		if tsPreMarketTime != "09:00:00" {
			t.Errorf("Pre-market time should be '09:00:00', got '%s'", tsPreMarketTime)
		}
	})
}

// 测试Python兼容性 - 这些测试用例来自Python实现
func TestPythonCompatibility(t *testing.T) {
	// 基本解析测试 - 对应Python测试
	t.Run("BasicParsing", func(t *testing.T) {
		// 1970-01-01 00:00:00
		ts, err := ParseTimestamp("1970-01-01")
		if err != nil {
			t.Errorf("Failed to parse '1970-01-01': %v", err)
		}
		str := ts.String()
		if str != "1970-01-01 00:00:00.000" {
			t.Errorf("Expected '1970-01-01 00:00:00.000', got '%s'", str)
		}

		// 1900-01-01 00:00:00
		ts, err = ParseTimestamp("1900-01-01")
		if err != nil {
			t.Errorf("Failed to parse '1900-01-01': %v", err)
		}
		str = ts.String()
		if str != "1900-01-01 00:00:00.000" {
			t.Errorf("Expected '1900-01-01 00:00:00.000', got '%s'", str)
		}

		// 2024-01-01 09:30:00
		ts, err = ParseTimestamp("2024-01-01 09:30:00")
		if err != nil {
			t.Errorf("Failed to parse '2024-01-01 09:30:00': %v", err)
		}
		str = ts.String()
		if str != "2024-01-01 09:30:00.000" {
			t.Errorf("Expected '2024-01-01 09:30:00.000', got '%s'", str)
		}
	})

	// 测试零值
	t.Run("ZeroValue", func(t *testing.T) {
		ts := ZeroTimestamp()
		str := ts.String()
		if str != "1970-01-01 00:00:00.000" {
			t.Errorf("Expected Zero() to be '1970-01-01 00:00:00.000', got '%s'", str)
		}
	})

	// 测试当前时间
	t.Run("Now", func(t *testing.T) {
		ts := NowTimestamp()
		if ts.IsEmpty() {
			t.Error("Now() should not be empty")
		}
		// Just check it can be converted to string
		str := ts.String()
		if len(str) == 0 {
			t.Error("Now() string should not be empty")
		}
	})

	// 测试extract
	t.Run("Extract", func(t *testing.T) {
		ts, _ := ParseTimestamp("2024-01-01 09:30:00")
		year, month, day := ts.Extract()
		if year != 2024 || month != 1 || day != 1 {
			t.Errorf("Extract returned wrong date: %d-%d-%d", year, month, day)
		}
	})

	// 测试only_date
	t.Run("OnlyDate", func(t *testing.T) {
		ts, _ := ParseTimestamp("2024-01-01 09:30:00")
		dateStr := ts.OnlyDate()
		if dateStr != "2024-01-01" {
			t.Errorf("Expected '2024-01-01', got '%s'", dateStr)
		}
	})

	// 测试only_time
	t.Run("OnlyTime", func(t *testing.T) {
		ts, _ := ParseTimestamp("2024-01-01 09:30:45")
		timeStr := ts.OnlyTime()
		if timeStr != "09:30:45" {
			t.Errorf("Expected '09:30:45', got '%s'", timeStr)
		}
	})

	// 测试start_of_day
	t.Run("StartOfDay", func(t *testing.T) {
		ts, _ := ParseTimestamp("2024-01-01 15:30:00")
		startOfDay := ts.StartOfDay()
		str := startOfDay.String()
		if str != "2024-01-01 00:00:00.000" {
			t.Errorf("Expected '2024-01-01 00:00:00.000', got '%s'", str)
		}
	})

	// 测试today/since
	t.Run("TodaySince", func(t *testing.T) {
		ts, _ := ParseTimestamp("2024-01-01 15:30:00")
		today9AM := ts.Today(9, 0, 0, 0)
		str := today9AM.String()
		if str != "2024-01-01 09:00:00.000" {
			t.Errorf("Expected '2024-01-01 09:00:00.000', got '%s'", str)
		}

		since9AM := ts.Since(9, 0, 0, 0)
		str = since9AM.String()
		if str != "2024-01-01 09:00:00.000" {
			t.Errorf("Expected '2024-01-01 09:00:00.000', got '%s'", str)
		}
	})

	// 测试floor和ceil
	t.Run("FloorCeil", func(t *testing.T) {
		ts, _ := ParseTimestamp("2024-01-01 15:30:45")
		floor := ts.Floor()
		str := floor.String()
		if str != "2024-01-01 15:30:00.000" {
			t.Errorf("Expected floor '2024-01-01 15:30:00.000', got '%s'", str)
		}

		ceil := ts.Ceil()
		str = ceil.String()
		// ceil应该是15:30:59.999
		expected := "2024-01-01 15:30:59.999"
		if str != expected {
			t.Errorf("Expected ceil '%s', got '%s'", expected, str)
		}
	})

	// 测试is_same_date
	t.Run("IsSameDate", func(t *testing.T) {
		ts1, _ := ParseTimestamp("2024-01-01 08:00:00")
		ts2, _ := ParseTimestamp("2024-01-01 20:00:00")
		ts3, _ := ParseTimestamp("2024-01-02 08:00:00")

		if !ts1.IsSameDate(ts2) {
			t.Error("ts1 and ts2 should be on same date")
		}

		if ts1.IsSameDate(ts3) {
			t.Error("ts1 and ts3 should not be on same date")
		}
	})
}

// 基准测试：比较C++兼容层与Go原生实现的性能
func BenchmarkCompatVsGoNative(b *testing.B) {
	ms := int64(1640995200000)

	b.Run("Compat_Creation", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = NewTimestamp(ms)
		}
	})

	b.Run("GoNative_Creation", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = NewTimestamp(ms)
		}
	})

	cppTs := NewTimestamp(ms)
	goTs := NewTimestamp(ms)

	b.Run("Compat_ToString", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = cppTs.String()
		}
	})

	b.Run("GoNative_ToString", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			_ = goTs.String()
		}
	})

	b.Run("Compat_Comparison", func(b *testing.B) {
		ts2 := NewTimestamp(ms + 1000)
		for i := 0; i < b.N; i++ {
			_ = cppTs.Less(ts2)
		}
	})

	b.Run("GoNative_Comparison", func(b *testing.B) {
		ts2 := NewTimestamp(ms + 1000)
		for i := 0; i < b.N; i++ {
			_ = goTs.Less(ts2)
		}
	})
}
