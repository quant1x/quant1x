package cache

import (
	_ "unsafe"
)

//go:linkname GetCodeList
func GetCodeList() []string
