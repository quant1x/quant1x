package tests

import (
	"encoding/binary"
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"unicode/utf16"
)

func TestCalendarParity(t *testing.T) {
	// try to locate files relative to repository root (workspace) or current dir
	jsPath := "tests/js_calendar_output_clean.json"
	goPath := "tests/go_calendar_output.json"
	// when `go test` runs a package, working dir is the package directory; adjust paths
	if wd, err := os.Getwd(); err == nil {
		base := filepath.Base(wd)
		if strings.EqualFold(base, "tests") {
			jsPath = "../tests/js_calendar_output_clean.json"
			goPath = "../tests/go_calendar_output.json"
		}
	}
	jsb, err := ioutil.ReadFile(jsPath)
	if err != nil {
		t.Fatalf("failed to read js output (%s): %v", jsPath, err)
	}
	gob, err := ioutil.ReadFile(goPath)
	if err != nil {
		t.Fatalf("failed to read go output (%s): %v", goPath, err)
	}
	// strip UTF-8 BOM if any
	if len(jsb) >= 3 && jsb[0] == 0xEF && jsb[1] == 0xBB && jsb[2] == 0xBF {
		jsb = jsb[3:]
	}
	// handle UTF-16 LE BOM
	if len(jsb) >= 2 && jsb[0] == 0xFF && jsb[1] == 0xFE {
		// convert UTF-16LE bytes to UTF-8
		// simple conversion assuming ASCII-range content: take every 2nd byte
		conv := make([]byte, 0, len(jsb)/2)
		for i := 2; i+1 < len(jsb); i += 2 {
			conv = append(conv, jsb[i])
		}
		jsb = conv
	}
	// helper: try multiple decodings
	toUTF8FromUTF16LE := func(b []byte) []byte {
		if len(b) < 2 {
			return b
		}
		start := 0
		if b[0] == 0xFF && b[1] == 0xFE {
			start = 2
		}
		if (len(b)-start)%2 != 0 {
			// trim trailing byte
			b = b[:len(b)-1]
		}
		u16 := make([]uint16, 0, (len(b)-start)/2)
		for i := start; i+1 < len(b); i += 2 {
			u16 = append(u16, binary.LittleEndian.Uint16(b[i: i+2]))
		}
		runes := utf16.Decode(u16)
		return []byte(string(runes))
	}

	tryUnmarshal := func(b []byte, v interface{}) error {
		if err := json.Unmarshal(b, v); err == nil {
			return nil
		}
		// strip UTF-8 BOM
		if len(b) >= 3 && b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF {
			if err := json.Unmarshal(b[3:], v); err == nil {
				return nil
			}
		}
		// try UTF-16LE
		if b2 := toUTF8FromUTF16LE(b); len(b2) > 0 {
			if err := json.Unmarshal(b2, v); err == nil {
				return nil
			}
		}
		return json.Unmarshal(b, v)
	}

	var js []interface{}
	var goa []interface{}
	if err := tryUnmarshal(jsb, &js); err != nil {
		t.Fatalf("failed to parse js json: %v", err)
	}
	if err := tryUnmarshal(gob, &goa); err != nil {
		t.Fatalf("failed to parse go json: %v", err)
	}
	if len(js) != len(goa) {
		t.Fatalf("length mismatch js=%d go=%d", len(js), len(goa))
	}
	for i := range js {
		aj, _ := json.Marshal(js[i])
		ag, _ := json.Marshal(goa[i])
		if string(aj) != string(ag) {
			t.Fatalf("mismatch at %d: js=%s go=%s", i, string(aj), string(ag))
		}
	}
}
