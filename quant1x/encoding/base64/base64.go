package base64

import (
	"encoding/base64"
	"math/rand"
	"slices"
	"time"

	"gitee.com/quant1x/gox/api"
)

const (
	BASE64CHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
)

func getRandFromSeed(seed int64) *rand.Rand {
	return rand.New(rand.NewSource(seed))
}

// Base64GetEncoder 生成一个新的base64的码表
func Base64GetEncoder() string {
	encoder := slices.Clone(api.String2Bytes(BASE64CHAR))
	seed := time.Now().UnixNano()
	r := getRandFromSeed(seed)
	r.Shuffle(len(encoder), func(i, j int) {
		encoder[i], encoder[j] = encoder[j], encoder[i]
	})
	return api.Bytes2String(encoder)
}

func getBase64Encoder(seed int64) string {
	encoder := slices.Clone(api.String2Bytes(BASE64CHAR))
	r := getRandFromSeed(seed)
	r.Shuffle(len(encoder), func(i, j int) {
		encoder[i], encoder[j] = encoder[j], encoder[i]
	})
	r = nil
	return api.Bytes2String(encoder)
}

type PseudoBase64 struct {
	base64Chars string
	encoding    *base64.Encoding
}

func NewBase64(encoder string) PseudoBase64 {
	return PseudoBase64{
		base64Chars: encoder,
		encoding:    base64.NewEncoding(encoder),
	}
}

func NewPseudoBase64(seed int64) PseudoBase64 {
	encoder := getBase64Encoder(seed)
	return PseudoBase64{
		base64Chars: encoder,
		encoding:    base64.NewEncoding(encoder),
	}
}

func (codec PseudoBase64) Encode(s string) string {
	return codec.encoding.EncodeToString(api.String2Bytes(s))
}

func (codec PseudoBase64) Decode(s string) (string, error) {
	data, err := codec.encoding.DecodeString(s)
	if err != nil {
		return "", err
	}
	return api.Bytes2String(data), nil
}
