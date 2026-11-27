package pandas

import (
	"testing"
)

func sampleFrame(t *testing.T) DataFrame {
	t.Helper()
	frame, err := DataFrameFromColumns(
		Column{Name: "open", Values: NewSeries([]float64{1, 2, 3})},
		Column{Name: "close", Values: NewSeries([]float64{1.5, 2.5, 3.5})},
	)
	if err != nil {
		t.Fatalf("构造失败: %v", err)
	}
	return frame
}

// TestDataFrameColumns 覆盖列序、形状与行数。
func TestDataFrameColumns(t *testing.T) {
	frame := sampleFrame(t)
	if got := frame.Columns(); len(got) != 2 || got[0] != "open" || got[1] != "close" {
		t.Fatalf("列序错误: %v", got)
	}
	rows, columns := frame.Shape()
	if rows != 3 || columns != 2 {
		t.Fatalf("形状错误: (%d, %d)", rows, columns)
	}
	if frame.Len() != 3 || frame.ColumnCount() != 2 || frame.IsEmpty() {
		t.Errorf("行/列数错误: %d %d %v", frame.Len(), frame.ColumnCount(), frame.IsEmpty())
	}
	if empty := NewDataFrame(); !empty.IsEmpty() {
		t.Error("空表应为空")
	}
	if rows, columns := NewDataFrame().Shape(); rows != 0 || columns != 0 {
		t.Errorf("空表形状错误: (%d, %d)", rows, columns)
	}
}

// TestDataFrameGetSelect 覆盖取列与子集。
func TestDataFrameGetSelect(t *testing.T) {
	frame := sampleFrame(t)

	closeSeries, ok := frame.Get("close")
	if !ok {
		t.Fatal("close 列缺失")
	}
	approxSeries(t, closeSeries.Values(), 1.5, 2.5, 3.5)

	if _, ok := frame.Get("volume"); ok {
		t.Error("不存在的列应返回 false")
	}
	if !frame.Contains("open") {
		t.Error("Contains 错误")
	}

	subset, err := frame.Select("close")
	if err != nil {
		t.Fatal(err)
	}
	if got := subset.Columns(); len(got) != 1 || got[0] != "close" {
		t.Errorf("子集列错误: %v", got)
	}
	if rows, columns := subset.Shape(); rows != 3 || columns != 1 {
		t.Errorf("子集形状错误: (%d, %d)", rows, columns)
	}
	if _, err := frame.Select("volume"); err == nil {
		t.Error("不存在的列应报错")
	}
}

// TestDataFrameSetReplace 覆盖原位替换。
func TestDataFrameSetReplace(t *testing.T) {
	frame := sampleFrame(t)
	if err := frame.Set("open", NewSeries([]float64{10, 20, 30})); err != nil {
		t.Fatal(err)
	}
	if got := frame.Columns(); len(got) != 2 || got[0] != "open" || got[1] != "close" {
		t.Errorf("替换不应改变列序: %v", got)
	}
	openSeries, _ := frame.Get("open")
	approxSeries(t, openSeries.Values(), 10, 20, 30)
}

// TestDataFrameSetAppend 覆盖追加。
func TestDataFrameSetAppend(t *testing.T) {
	frame := sampleFrame(t)
	if err := frame.Set("volume", NewSeries([]float64{100, 200, 300})); err != nil {
		t.Fatal(err)
	}
	if got := frame.Columns(); len(got) != 3 || got[2] != "volume" {
		t.Errorf("追加列错误: %v", got)
	}
	if rows, columns := frame.Shape(); rows != 3 || columns != 3 {
		t.Errorf("追加后形状错误: (%d, %d)", rows, columns)
	}
}

// TestDataFrameLengthMismatch 覆盖列长校验。
func TestDataFrameLengthMismatch(t *testing.T) {
	frame := sampleFrame(t)
	if err := frame.Set("volume", NewSeries([]float64{1})); err == nil {
		t.Error("列长不一致应报错")
	}
	if got := frame.Columns(); len(got) != 2 {
		t.Errorf("失败写入不应改变列: %v", got)
	}
}

// TestDataFrameCopy 覆盖深拷贝。
func TestDataFrameCopy(t *testing.T) {
	frame := sampleFrame(t)
	copied := frame.Copy()
	if err := copied.Set("open", NewSeries([]float64{0, 0, 0})); err != nil {
		t.Fatal(err)
	}

	original, _ := frame.Get("open")
	approxSeries(t, original.Values(), 1, 2, 3)

	replaced, _ := copied.Get("open")
	approxSeries(t, replaced.Values(), 0, 0, 0)
}

// TestDataFrameString 覆盖调试输出。
func TestDataFrameString(t *testing.T) {
	frame := sampleFrame(t)
	want := "DataFrame(shape=(3, 2), columns=[open, close])"
	if got := frame.String(); got != want {
		t.Errorf("期望 %q, 实际 %q", want, got)
	}
}
