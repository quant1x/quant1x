package level1

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
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

type BlockMetaRequest struct {
	BlockFilename [40]byte
}

func NewBlockMetaRequest(filename string) *BlockMetaRequest {
	req := &BlockMetaRequest{}
	copy(req.BlockFilename[:], filename)
	return req
}

func (req *BlockMetaRequest) Bytes() []byte {
	return buildRequest(req.Command(), packetTypeRequest, req.BlockFilename[:])
}

func (req *BlockMetaRequest) Command() StdCommand {
	return StdCommandBlockMeta
}

func (req *BlockMetaRequest) String() string {
	filename := string(bytes.TrimRight(req.BlockFilename[:], "\x00"))
	return fmt.Sprintf("{BlockFilename:%s}", filename)
}

type BlockMetaResponse struct {
	ResponseBase
	Meta BlockMeta
}

func (resp *BlockMetaResponse) Deserialize(data []byte) error {
	if len(data) < 38 {
		return nil
	}
	resp.Meta.Size = binary.LittleEndian.Uint32(data[0:4])
	resp.Meta.C1 = data[4]
	copy(resp.Meta.HashValue[:], data[5:37])
	resp.Meta.C2 = data[37]
	return nil
}

func (resp *BlockMetaResponse) String() string {
	return fmt.Sprintf("{%s}", resp.Meta.String())
}
