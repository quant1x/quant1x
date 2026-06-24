package std

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx/tdxproto"
)

const (
	BlockZhishu  = "block_zs.dat"
	BlockFengge  = "block_fg.dat"
	BlockGainian = "block_gn.dat"
	BlockDefault = "block.dat"
)

type BlockMeta struct {
	Size      uint32
	C1        uint8
	HashValue [32]byte
	C2        uint8
}

func (meta BlockMeta) String() string {
	return fmt.Sprintf("Size: %d C1: %d HashValue: %s C2: %d",
		meta.Size, meta.C1, hex.EncodeToString(meta.HashValue[:]), meta.C2)
}

// BlockMetaContext 对齐 C++/Rust/Python BlockFileMetaContext, 合并请求和响应.
type BlockMetaContext struct {
	tdxproto.FrameBase
	BlockFilename [40]byte
	Meta          BlockMeta
}

// NewBlockMetaContext 构造板块元数据请求, 对齐 C++/Rust.
func NewBlockMetaContext(filename string) *BlockMetaContext {
	ctx := &BlockMetaContext{
		FrameBase: tdxproto.NewFrameBase(tdxproto.StdCommandBlockMeta, tdxproto.FlagUncompressed, tdxproto.PacketTypeRequest),
	}
	copy(ctx.BlockFilename[:], filename)
	return ctx
}

// SerializeRequestBody 序列化请求体, 对齐 C++/Rust/Python.
func (b *BlockMetaContext) SerializeRequestBody() []byte { return b.BlockFilename[:] }

// DeserializeResponseBody 解析板块元数据响应体, 对齐 C++/Rust/Python.
func (b *BlockMetaContext) DeserializeResponseBody(data []byte) error {
	if len(data) < 38 {
		return nil
	}
	b.Meta.Size = binary.LittleEndian.Uint32(data[0:4])
	b.Meta.C1 = data[4]
	copy(b.Meta.HashValue[:], data[5:37])
	b.Meta.C2 = data[37]
	return nil
}

func (b *BlockMetaContext) String() string {
	filename := string(bytes.TrimRight(b.BlockFilename[:], "\x00"))
	return fmt.Sprintf("BlockMetaContext{Filename:%s, %s}", filename, b.Meta.String())
}
