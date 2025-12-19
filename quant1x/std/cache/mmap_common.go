// mmap_common.go
package cache

import (
	"os"
)

type commonMmap struct {
	data []byte
	file *os.File
}

func (m *commonMmap) Bytes() []byte {
	return m.data
}
