//go:build darwin

package api

import (
	_ "unsafe" // for go:linkname
)

//go:linkname walltime runtime.walltime
func walltime() (int64, int32)

// 此文件已不再需要，现在直接使用now()函数
