package core

import (
	"strings"
	"testing"
)

func TestCoreGetDataPath(t *testing.T) {
	expectedSuffix := ".q1x-go"
	basePath := GetBasePath()
	if !strings.HasSuffix(basePath, ".q1x-go") {
		t.Errorf("GetBasePath() = %s, want suffix %s", basePath, ".q1x-go")
	}
	dataPath := GetDataPath()
	if !strings.HasSuffix(dataPath, expectedSuffix) {
		t.Errorf("GetDataPath() = %s, want suffix %s", dataPath, expectedSuffix)
	}
}
