package tests

import (
	"encoding/json"
	"fmt"
	"testing"
	"time"
)

// 测试用例

func TestDecodeJavaScript(t *testing.T) {
	encoded_data := "LC/AAAf8CXCw6mHbaPgkryxXv10eAJP1LW0SD39aT7+NV44Xba3PxCgTdrFc3FepphjnTBw1X4hmGu+ypVAcvFenpBXPqCc6F4ZmGueLFwbIN8QTDXPsCc1FepphjvOoCc8FepphjvcgFO3CP00wxXXWhrkUdZrIJpw9X3ThrlEp6hlGc88Kcem0VeFpZM46VV4MrTC2KScKc811U4aLXUdlzINc9lTrwFW3T52KPj0mDueVFuUR1RtiEoCXfdgFOOSGRXnUhrXWhb0kt6Rk2pU44JV4SrTyU9wSDHPwCnXdP1FuiUM44r7qwdKqcYrIZpw1DqgrlU5IrHRawxjrwBaqcbrIt9gr3UhDtOpyVNjEnCHPnC3royNWvi0gj/"
	out := d(encoded_data)
	// try direct marshal first
	b, err := json.MarshalIndent(out, "", "  ")
	if err == nil {
		fmt.Println(string(b))
		return
	}

	// fallback normalizers
	switch v := out.(type) {
	case []time.Time:
		ss := make([]string, len(v))
		for i, t := range v {
			ss[i] = t.Format("2006-01-02")
		}
		if b, err = json.MarshalIndent(ss, "", "  "); err == nil {
			fmt.Println(string(b))
			return
		}
	case []map[string]interface{}:
		// convert any time.Time values to ISO strings
		for _, m := range v {
			for k, val := range m {
				if tt, ok := val.(time.Time); ok {
					m[k] = tt.Format("2006-01-02 15:04:05")
				}
			}
		}
		if b, err = json.MarshalIndent(v, "", "  "); err == nil {
			fmt.Println(string(b))
			return
		}
	case [][]int64:
		if b, err = json.MarshalIndent(v, "", "  "); err == nil {
			fmt.Println(string(b))
			return
		}
	}

	// final fallback: print error + Go value
	fmt.Printf("json.Marshal failed: %v\n%#v\n", err, out)
}
