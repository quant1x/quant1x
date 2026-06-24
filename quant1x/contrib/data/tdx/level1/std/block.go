package std

import (
	"bytes"
	"encoding/binary"
	"fmt"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
)

const (
	BlockChunksSize = 0x7530
)

type BlockInfo struct {
	BlockName  string
	BlockType  uint16
	StockCount uint16
	CodeList   []string
}

func (info BlockInfo) String() string {
	return fmt.Sprintf("BlockName: %s BlockType: %d StockCount: %d CodeList: [%s]",
		info.BlockName, info.BlockType, info.StockCount, "")
}

// BlockDataContext 对齐 C++/Rust/Python BlockFileContext, 合并请求和响应.
type BlockDataContext struct {
	tdxproto.FrameBase
	Start         uint32
	Size          uint32
	BlockFilename [100]byte
	DataSize      uint32
	Data          []byte
}

// NewBlockDataContext 构造板块数据请求, 对齐 C++/Rust.
func NewBlockDataContext(filename string, offset uint32) *BlockDataContext {
	ctx := &BlockDataContext{
		FrameBase:     tdxproto.NewFrameBase(tdxproto.StdCommandBlockData, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
		Start:         offset,
		Size:          BlockChunksSize,
	}
	copy(ctx.BlockFilename[:], filename)
	return ctx
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (b *BlockDataContext) SerializeRequestBody() []byte {
	buf := new(bytes.Buffer)
	_ = binary.Write(buf, binary.LittleEndian, b.Start)
	_ = binary.Write(buf, binary.LittleEndian, b.Size)
	buf.Write(b.BlockFilename[:])
	return buf.Bytes()
}

// DeserializeResponseBody 解析板块数据响应体, 对齐 C++/Rust/Python.
func (b *BlockDataContext) DeserializeResponseBody(data []byte) error {
	if len(data) < 4 {
		return nil
	}
	b.DataSize = binary.LittleEndian.Uint32(data[:4])
	if b.DataSize > 0 {
		b.Data = make([]byte, len(data)-4)
		copy(b.Data, data[4:])
	}
	return nil
}

func (b *BlockDataContext) String() string {
	filename := string(bytes.TrimRight(b.BlockFilename[:], "\x00"))
	return fmt.Sprintf("BlockDataContext{Start:%d, Size:%d, Filename:%s, DataSize:%d}", b.Start, b.Size, filename, b.DataSize)
}
