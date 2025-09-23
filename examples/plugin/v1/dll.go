package main

import "C"

//export Add
func Add(a, b int) int {
	return a + b
}

// go build -ldflags "-s -w" -buildmode=c-shared -o mydll.so .\plugin\v1\dll.go
func main() {} // 必须存在但不执行
