package exchange

import (
	"os"
)

// getFilenameModifiedTime 获取指定文件的最后修改时间
//
// 参数:
//
//	fname: 文件路径
//
// 返回值:
//
//	*exchange.Timestamp: 文件修改时间的时间戳
//	error: 获取文件信息失败时返回的错误
func getFilenameModifiedTime(fname string) (*Timestamp, error) {
	info, err := os.Lstat(fname)
	if err != nil {
		return nil, err
	}
	tp := NewTimestampFromTime(info.ModTime())
	return &tp, nil
}

// ShouldUpdateFile 检查指定文件是否需要更新
//
// 参数:
//
//	fname - 要检查的文件路径
//
// 返回值:
//
//	bool - 如果文件需要更新则返回true，否则返回false
//
// 注意:
//
//	如果获取文件修改时间失败，默认返回true
func ShouldUpdateFile(fname string) bool {
	modTime, err := getFilenameModifiedTime(fname)
	if err != nil {
		return true
	}
	return CanInitialize(modTime)
}
