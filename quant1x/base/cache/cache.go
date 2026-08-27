package cache

import (
	"fmt"
	"hash/crc32"
	"os"
	"path/filepath"
	"sync"
	"unsafe"

	"github.com/quant1x/quant1x/quant1x/base/mem"
)

const (
	dirMode  = 0755
	fileMode = 0644
)

const (
	DefaultSize = 1 << 20 // 1MB
	THRESHOLD   = 0.8     // 80%使用率触发扩容
)

const (
	version     = 1
	headerSize  = 32
	magicNumber = 0xCAC1E5A5
	maxFileSize = 1 << 30
)

var (
	ErrInvalidFile   = fmt.Errorf("invalid cache file")
	ErrInvalidAccess = fmt.Errorf("invalid memory access")
	ErrChecksum      = fmt.Errorf("data checksum mismatch")
	ErrOutOfSpace    = fmt.Errorf("out of space")
)

// MemObject 封装内存映射操作接口
type MemObject interface {
	Flush() error
	Unmap() error
	Bytes() []byte
}

// Cache 使用内存映射的跨进程安全缓存
type Cache[E any] struct {
	mu       sync.RWMutex // 读写锁
	filename string       // 文件路径
	f        *os.File     // 文件对象
	userSize int64        // 用户指定的数据区容量(不含header)
	data     MemObject    // 内存映射对象
	header   *cacheHeader // 头结构指针
}

// 缓存头结构(16字节对齐)
type cacheHeader struct {
	headerSize  uint32 // 头信息长度, 包括headerSize字段
	magic       uint32 // 魔法数
	version     uint32 // 版本
	checksum    uint32 // 数据校验和
	dataSize    uint32 // 数据长度
	elementSize uint32 // 元素尺寸
	arrayLen    uint32 // 数组有效长度
	arrayCap    uint32 // 数组容量
	//_           [4]byte // 填充对齐
}

// OpenCache 创建或打开内存映射缓存
func OpenCache[E any](name string) (*Cache[E], error) {
	eSize := mem.TypeSize[E]()
	if eSize == 0 {
		return nil, fmt.Errorf("zero-sized type")
	}
	//totalSize := headerSize + userSize
	//if totalSize > maxFileSize || userSize < 0 {
	//	return nil, fmt.Errorf("invalid size %d (max allowed %d)", userSize, maxFileSize-headerSize)
	//}

	if err := os.MkdirAll(filepath.Dir(name), dirMode); err != nil {
		return nil, fmt.Errorf("create directory failed: %w", err)
	}

	f, err := os.OpenFile(name, os.O_RDWR|os.O_CREATE, fileMode)
	if err != nil {
		return nil, fmt.Errorf("open file failed: %w", err)
	}
	finfo, err := f.Stat()
	if err != nil {
		_ = f.Close()
		return nil, fmt.Errorf("stat file failed: %w", err)
	}

	totalSize := finfo.Size()
	if totalSize == 0 {
		totalSize = headerSize
	}
	if err := f.Truncate(totalSize); err != nil {
		_ = f.Close()
		return nil, fmt.Errorf("truncate failed: %w", err)
	}
	userSize := totalSize - headerSize
	data, err := mmap(int(totalSize), f)
	if err != nil {
		_ = f.Close()
		return nil, fmt.Errorf("mmap failed: %w", err)
	}

	c := &Cache[E]{
		filename: name,
		f:        f,
		userSize: userSize,
		data:     data,
		header:   (*cacheHeader)(unsafe.Pointer(&data.Bytes()[0])),
	}

	if err := c.initHeader(); err != nil {
		_ = c.Close()
		return nil, err
	}

	return c, nil
}

func (c *Cache[E]) typeSize() uint32 {
	size := mem.TypeSize[E]()
	return uint32(size)
}

// 初始化文件头
func (c *Cache[E]) initHeader() error {
	if c.header.magic == 0 {
		c.header.headerSize = headerSize
		c.header.magic = magicNumber
		c.header.version = version
		c.header.dataSize = 0
		c.header.elementSize = c.typeSize()
		return nil
	}
	if c.header.headerSize == 0 {
		c.header.headerSize = headerSize
	}
	if c.header.magic != magicNumber {
		return ErrInvalidFile
	}
	if c.header.elementSize == 0 {
		c.header.elementSize = c.typeSize()
	}
	return c.verifyData()
}

// WriteData 类型安全写入
func (c *Cache[E]) WriteData(offset uint32, src []byte) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	end := offset + uint32(len(src))
	if end > uint32(c.userSize) {
		return ErrOutOfSpace
	}

	if end > c.header.dataSize {
		c.header.dataSize = end
	}

	data := c.data.Bytes()
	copy(data[headerSize+offset:], src)
	c.updateChecksum()
	return nil
}

// 更新校验和
func (c *Cache[E]) updateChecksum() {
	data := c.data.Bytes()[headerSize : headerSize+c.header.dataSize]
	c.header.checksum = crc32.ChecksumIEEE(data)
}

// 验证数据完整性
func (c *Cache[E]) verifyData() error {
	data := c.data.Bytes()[headerSize : headerSize+c.header.dataSize]
	if crc32.ChecksumIEEE(data) != c.header.checksum {
		return ErrChecksum
	}
	return nil
}

// Close 安全关闭
func (c *Cache[E]) Close() error {
	c.mu.Lock()
	defer c.mu.Unlock()

	if c.f == nil {
		return nil
	}

	var errs []error
	if err := c.data.Flush(); err != nil {
		errs = append(errs, err)
	}
	if err := c.data.Unmap(); err != nil {
		errs = append(errs, err)
	}
	if err := c.f.Close(); err != nil {
		errs = append(errs, err)
	}

	c.f = nil
	if len(errs) > 0 {
		return fmt.Errorf("close errors: %v", errs)
	}
	return nil
}

func (c *Cache[E]) Add(delta int) error {
	if delta < 0 {
		return fmt.Errorf("index out of range [%d]", delta)
	}
	c.mu.Lock()
	defer c.mu.Unlock()
	required := c.header.arrayLen + uint32(delta)
	//if required > uint32(float64(c.header.arrayCap)*THRESHOLD) {
	//	if err := c.expand(); err != nil {
	//		return err
	//	}
	//}
	if required > c.header.arrayCap {
		if err := c.expand(required); err != nil {
			return err
		}
	}
	if c.header.arrayCap < c.header.arrayLen+uint32(delta) {
		return ErrOutOfSpace
	}
	c.header.arrayLen += uint32(delta)
	return nil
}

func (c *Cache[E]) expand(required uint32) error {
	newArrayCap := c.header.arrayCap + 1
	for newArrayCap < required {
		newArrayCap *= 2
	}
	newUserSize := c.header.elementSize * newArrayCap
	newCapacity := headerSize + newUserSize

	// 0. 备份header
	var oldHeader cacheHeader
	oldHeader = *(c.header)
	// 同步磁盘
	if err := c.data.Flush(); err != nil {
		return err
	}
	//oldHeader := cacheHeader{
	//	headerSize:  c.header.headerSize,
	//	magic:       c.header.magic,
	//	version:     c.header.version,
	//	dataSize:    c.header.dataSize,
	//	checksum:    c.header.checksum,
	//	elementSize: c.header.elementSize,
	//	arrayCap:    c.header.arrayCap,
	//	arrayLen:    c.header.arrayLen,
	//}
	// 1. 解除旧映射
	if err := c.data.Unmap(); err != nil {
		return err
	}

	// 2. 扩展文件
	if err := c.f.Truncate(int64(newCapacity)); err != nil {
		return err
	}

	// 3. 重新映射
	data, err := mmap(int(newCapacity), c.f)
	if err != nil {
		_ = c.f.Close()
		return fmt.Errorf("mmap failed: %w", err)
	}

	// 4. 更新元数据
	c.userSize = int64(newUserSize)
	c.data = data
	c.header = (*cacheHeader)(unsafe.Pointer(&data.Bytes()[0]))
	*(c.header) = oldHeader
	c.header.arrayCap = newArrayCap

	if err := c.initHeader(); err != nil {
		_ = c.Close()
		return err
	}

	return nil
}

// ToSlice 安全转换为类型切片
func (c *Cache[E]) ToSlice() ([]E, error) {
	c.mu.RLock()
	defer c.mu.RUnlock()

	dataStart := uintptr(unsafe.Pointer(&c.data.Bytes()[headerSize]))
	var e E
	eSize := c.typeSize()
	if dataStart%unsafe.Alignof(e) != 0 {
		return nil, fmt.Errorf("memory address %x not aligned for %T (alignment %d)",
			dataStart, e, unsafe.Alignof(e))
	}

	usedElements := int(c.header.dataSize) / int(eSize)
	usedElements = int(c.userSize) / int(eSize)
	c.header.arrayCap = uint32(usedElements)
	c.header.elementSize = eSize
	addr := &c.data.Bytes()[headerSize]
	ptr := unsafe.Pointer(addr)
	return unsafe.Slice((*E)(ptr), usedElements), nil
}
