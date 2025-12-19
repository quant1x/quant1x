package std

import (
	"fmt"
	"testing"
)

func TestGoID(t *testing.T) {
	gid := GoID()
	fmt.Println(gid)
}
