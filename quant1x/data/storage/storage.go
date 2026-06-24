package storage

import (
	"fmt"

	"github.com/quant1x/quant1x/quant1x/data/meta"
)

// FileStorage 文件存储接口（专用于单一数据类型 T）
//
// 提供抽象的文件存储生命周期：初始化 → 更新 → 加载/保存。
// 实现者只需实现文件名生成和是否需要初始化/更新的判断逻辑。
type FileStorage[T any] interface {
	// FileName 返回文件名
	FileName() string

	// ShouldInitialize 判断是否需要初始化
	ShouldInitialize(timestamp ...meta.Timestamp) bool

	// ShouldUpdate 判断是否需要更新
	ShouldUpdate(timestamp ...meta.Timestamp) bool

	// Update 更新数据（无参，类型已固定）
	Update()

	// Load 加载数据
	Load() ([]T, error)

	// Save 保存数据
	Save(data []T) error

	// Checkout 检出数据（自动更新 + 加载）
	Checkout() ([]T, error)
}

// BaseFileStorage 提供 FileStorage 接口的 Load/Save/Checkout 默认实现
//
// 嵌入此结构体可减少样板代码，实现者只需实现 FileName/ShouldInitialize/ShouldUpdate/Update。
type BaseFileStorage[T any] struct {
	FileNameFunc         func() string
	ShouldInitializeFunc func(timestamp ...meta.Timestamp) bool
	ShouldUpdateFunc     func(timestamp ...meta.Timestamp) bool
	UpdateFunc           func()
}

func (b *BaseFileStorage[T]) FileName() string {
	if b.FileNameFunc != nil {
		return b.FileNameFunc()
	}
	return ""
}

func (b *BaseFileStorage[T]) ShouldInitialize(timestamp ...meta.Timestamp) bool {
	if b.ShouldInitializeFunc != nil {
		return b.ShouldInitializeFunc(timestamp...)
	}
	return false
}

func (b *BaseFileStorage[T]) ShouldUpdate(timestamp ...meta.Timestamp) bool {
	if b.ShouldUpdateFunc != nil {
		return b.ShouldUpdateFunc(timestamp...)
	}
	return false
}

func (b *BaseFileStorage[T]) Update() {
	if b.UpdateFunc != nil {
		b.UpdateFunc()
	}
}

func (b *BaseFileStorage[T]) Load() ([]T, error) {
	var result []T
	filename := b.FileName()
	if filename == "" {
		return result, nil
	}
	err := CsvToSlice(filename, &result)
	return result, err
}

func (b *BaseFileStorage[T]) Save(data []T) error {
	filename := b.FileName()
	if filename == "" {
		return nil
	}
	return SliceToCsv(filename, data, true)
}

func (b *BaseFileStorage[T]) Checkout() ([]T, error) {
	if b.ShouldInitialize() || b.ShouldUpdate() {
		b.Update()
	}
	return b.Load()
}

// TypeName 返回泛型类型 T 的名称（用于生成文件名）
func TypeName[T any]() string {
	var zero T
	return fmt.Sprintf("%T", zero)
}

// BasedataFileStorage 基础数据文件存储类
//
// 用于存储与具体证券标的（Instrument）关联的基础数据。
type BasedataFileStorage[T any] struct {
	BaseFileStorage[T]
	Inst *meta.Instrument
}

// NewBasedataFileStorage 创建基础数据文件存储实例
func NewBasedataFileStorage[T any](inst *meta.Instrument) *BasedataFileStorage[T] {
	return &BasedataFileStorage[T]{
		Inst: inst,
	}
}

// Instrument 返回关联的证券标的
func (b *BasedataFileStorage[T]) Instrument() *meta.Instrument {
	return b.Inst
}

// MetaFileStorage 元数据文件存储类
//
// 用于存储与数据类型绑定的元数据，文件名自动生成为 "{TypeName}.csv"。
type MetaFileStorage[T any] struct {
	BaseFileStorage[T]
}

// NewMetaFileStorage 创建元数据文件存储实例，文件名自动根据类型名生成
func NewMetaFileStorage[T any]() *MetaFileStorage[T] {
	s := &MetaFileStorage[T]{}
	s.FileNameFunc = func() string {
		return TypeName[T]() + ".csv"
	}
	return s
}
