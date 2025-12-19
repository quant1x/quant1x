package cache

import (
	"fmt"
	"testing"
)

func TestCachePool(t *testing.T) {
	type MyObject struct {
		Data []byte
	}
	pool := Pool[MyObject]{}

	// 获取对象（自动初始化为零值）
	obj := pool.Acquire()

	// 使用对象
	obj.Data = make([]byte, 1024)

	// 释放对象（自动重置）
	pool.Release(obj)
}

func TestPool(t *testing.T) {
	type TestStruct struct {
		Name string
	}
	var pool Pool[TestStruct]
	count := 100
	var t1 TestStruct
	t1.Name = "test"
	pool.Release(&t1)
	for i := 0; i < count; i++ {
		t1 := pool.Acquire()
		fmt.Printf("%d: %p, %+v\n", i, t1, t1)
		t1.Name = fmt.Sprintf("%d", i)
		pool.Release(t1)
	}
}
