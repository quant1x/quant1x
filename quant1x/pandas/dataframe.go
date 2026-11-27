package pandas

import (
	"fmt"
	"strings"
)

// Column 是构造 DataFrame 时的一列：列名 + 数据。
type Column struct {
	Name   string
	Values Series
}

// DataFrame 是多列容器，对标 quant1x/pandas/dataframe.py。
//
// 只做按列名存取（列名按插入序），数值语义在 Series 与 quant1x/num；
// Go 无运算符重载，df[name] / df[name] = value / df[names] 分别对应
// Get / Set / Select 三个方法。
type DataFrame struct {
	columns []Column
}

// NewDataFrame 返回空表。
func NewDataFrame() DataFrame { return DataFrame{} }

// DataFrameFromColumns 由列构造；所有列必须等长。
func DataFrameFromColumns(columns ...Column) (DataFrame, error) {
	frame := NewDataFrame()
	for _, column := range columns {
		if err := frame.Set(column.Name, column.Values); err != nil {
			return DataFrame{}, err
		}
	}
	return frame, nil
}

// Columns 返回列名（插入序）。
func (f DataFrame) Columns() []string {
	names := make([]string, len(f.columns))
	for i, column := range f.columns {
		names[i] = column.Name
	}
	return names
}

// ColumnCount 返回列数。
func (f DataFrame) ColumnCount() int { return len(f.columns) }

// Len 返回行数（首列决定，无列为 0）。
func (f DataFrame) Len() int {
	if len(f.columns) == 0 {
		return 0
	}
	return f.columns[0].Values.Len()
}

// IsEmpty 判断是否为空表。
func (f DataFrame) IsEmpty() bool { return f.Len() == 0 }

// Shape 返回 (行数, 列数)。
func (f DataFrame) Shape() (int, int) { return f.Len(), f.ColumnCount() }

// Contains 判断是否已有该列。
func (f DataFrame) Contains(name string) bool {
	_, ok := f.find(name)
	return ok
}

// Get 取列。
func (f DataFrame) Get(name string) (Series, bool) {
	index, ok := f.find(name)
	if !ok {
		return Series{}, false
	}
	return f.columns[index].Values, true
}

// Select 按给定列序取子集。
func (f DataFrame) Select(names ...string) (DataFrame, error) {
	frame := NewDataFrame()
	for _, name := range names {
		values, ok := f.Get(name)
		if !ok {
			return DataFrame{}, fmt.Errorf("DataFrame: 列不存在 %s", name)
		}
		frame.columns = append(frame.columns, Column{Name: name, Values: values.Copy()})
	}
	return frame, nil
}

// Set 写列：已存在则原位替换，否则追加；列长必须与现有行数一致。
func (f *DataFrame) Set(name string, values Series) error {
	if len(f.columns) > 0 && values.Len() != f.Len() {
		return fmt.Errorf("DataFrame: 列长不一致, 期望 %d 实际 %d", f.Len(), values.Len())
	}
	if index, ok := f.find(name); ok {
		f.columns[index].Values = values
		return nil
	}
	f.columns = append(f.columns, Column{Name: name, Values: values})
	return nil
}

// Copy 返回深拷贝。
func (f DataFrame) Copy() DataFrame {
	columns := make([]Column, len(f.columns))
	for i, column := range f.columns {
		columns[i] = Column{Name: column.Name, Values: column.Values.Copy()}
	}
	return DataFrame{columns: columns}
}

// String 便于调试: 形状与列名。
func (f DataFrame) String() string {
	rows, columns := f.Shape()
	return fmt.Sprintf("DataFrame(shape=(%d, %d), columns=[%s])",
		rows, columns, strings.Join(f.Columns(), ", "))
}

func (f DataFrame) find(name string) (int, bool) {
	for i, column := range f.columns {
		if column.Name == name {
			return i, true
		}
	}
	return -1, false
}
