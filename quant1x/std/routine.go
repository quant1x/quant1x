package std

import (
	"github.com/petermattis/goid"
)

// GoID 获取协程id
func GoID() int64 {
	return goid.Get()
}
