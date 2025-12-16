package encoding

import (
	"bytes"
	"io"

	"golang.org/x/text/encoding/simplifiedchinese"
	"golang.org/x/text/transform"
)

var (
	gbkDecoder = simplifiedchinese.GBK.NewDecoder()
)

// GBKToUTF8 converts GBK encoded bytes into a UTF-8 string.
func GBKToUTF8(data []byte) (string, error) {
	reader := transform.NewReader(bytes.NewReader(data), gbkDecoder)
	out, err := io.ReadAll(reader)
	if err != nil {
		return "", err
	}
	return string(out), nil
}
