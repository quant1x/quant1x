package std

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strings"

	"github.com/quant1x/quant1x/quant1x/contrib/data/tdx"
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
		info.BlockName, info.BlockType, info.StockCount, strings.Join(info.CodeList, ","))
}

type BlockFileContext struct {
	Start         uint32
	Size          uint32
	BlockFilename [100]byte
}

func NewBlockInfoRequest(filename string, offset uint32) *BlockFileContext {
	req := &BlockFileContext{
		Start: offset,
		Size:  BlockChunksSize,
	}
	copy(req.BlockFilename[:], filename)
	return req
}

func (req *BlockFileContext) Serialize() []byte {
	buf := new(bytes.Buffer)
	_ = binary.Write(buf, binary.LittleEndian, req.Start)
	_ = binary.Write(buf, binary.LittleEndian, req.Size)
	buf.Write(req.BlockFilename[:])
	return tdx.BuildRequest(req.Command(), tdx.PacketTypeRequest, buf.Bytes())
}

func (req *BlockFileContext) Command() tdx.StdCommand {
	return tdx.StdCommandBlockData
}

func (req *BlockFileContext) String() string {
	filename := string(bytes.TrimRight(req.BlockFilename[:], "\x00"))
	return fmt.Sprintf("{Start:%d, Size:%d, BlockFilename:%s}", req.Start, req.Size, filename)
}

type BlockInfoResponse struct {
	tdx.ResponseBase
	Size uint32
	Data []byte
}

func (resp *BlockInfoResponse) Deserialize(data []byte) error {
	if len(data) < 4 {
		return nil
	}
	resp.Size = binary.LittleEndian.Uint32(data[:4])
	if resp.Size > 0 {
		resp.Data = make([]byte, len(data)-4)
		copy(resp.Data, data[4:])
	}
	return nil
}

func (resp *BlockInfoResponse) String() string {
	return fmt.Sprintf("{Size:%d}", resp.Size)
}
