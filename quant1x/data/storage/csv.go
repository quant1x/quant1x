package storage

import (
	"os"

	"github.com/quant1x/pkg/gocsv"
	"github.com/quant1x/quant1x/quant1x/std"
)

const (
	DefaultTagName = "csv"
)

func init() {
	gocsv.TagName = DefaultTagName
}

// CsvToSlice CSV文件转struct切片
func CsvToSlice[S ~[]E, E any](filename string, pointer *S) error {
	filepath, err := std.ExpandUser(filename)
	if err != nil {
		return err
	}
	csvFile, err := os.Open(filepath)
	if err != nil {
		return err
	}
	err = gocsv.Unmarshal(csvFile, pointer)
	std.CloseQuietly(csvFile)
	return err
}

// SliceToCsv struct切片转csv文件
func SliceToCsv[S ~[]E, E any](filename string, s S, force ...bool) error {
	filepath, err := std.ExpandUser(filename)
	if err != nil {
		return err
	}
	// 检查目录, 不存在就创建
	_ = std.CheckFilepath(filepath, true)
	csvFile, err := os.Create(filepath)
	if err != nil {
		return err
	}
	err = gocsv.MarshalFile(s, csvFile)
	if err == nil {
		forceSync := false
		if len(force) > 0 && force[0] {
			forceSync = true
		}
		// 强制刷新内存副本到磁盘
		if forceSync {
			err = csvFile.Sync()
		}
	}
	std.CloseQuietly(csvFile)
	return err
}
