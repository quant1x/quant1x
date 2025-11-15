package exchange

import (
	"fmt"
	"testing"
)

// 测试用例

func TestDecodeJavaScript(t *testing.T) {
	encoded_data := "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrp5BkYVAc11hWvg0c/19UAc7jNtHQyWBAu2xmGuZI1NVAc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/"
	decoder := NewFinanceDecoder(encoded_data)
	out := decoder.Decode()
	// // try direct marshal first
	// b, err := json.MarshalIndent(out, "", "  ")
	// if err == nil {
	// 	fmt.Println(string(b))
	// 	return
	// }

	// convert any time.Time values to ISO strings
	for _, m := range out.([]string) {
		fmt.Printf("%v\n", m)

	}

	// final fallback: print error + Go value
	//fmt.Printf("json.Marshal failed: %v\n%#v\n", err, out)
}
