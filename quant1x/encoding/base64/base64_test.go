package base64

import (
	"fmt"
	"testing"
)

func TestBase64GetEncoder(t *testing.T) {
	s := Base64GetEncoder()
	fmt.Println(s)
}

func TestBase64(t *testing.T) {
	s := Base64GetEncoder()
	base64 := NewBase64(s)
	ev := base64.Encode("123")
	fmt.Println(ev)
	dv, err := base64.Decode(ev)
	fmt.Println(dv, err)
}

func TestPseudoBase64(t *testing.T) {
	base64 := NewPseudoBase64(1)
	ev := base64.Encode("123")
	fmt.Println(ev)
	dv, err := base64.Decode(ev)
	fmt.Println(dv, err)
}
