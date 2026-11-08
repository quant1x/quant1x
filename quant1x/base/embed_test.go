// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package base

import (
	"embed"
	"fmt"
	"testing"
)

//go:embed *
var resources embed.FS

func TestEmbed(t *testing.T) {
	filename := "embed.go"
	file, err := OpenEmbed(resources, filename)
	fmt.Println(file, err)

	target := "embed.go.copy"
	err1 := Export(resources, filename, target)
	fmt.Println(err1)
}
