// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package cstruct

func Pack(obj IStruct) ([]byte, error) {
	p := NewBuffer(nil)
	err := p.Marshal(obj)
	if p.buf == nil && err == nil {
		return []byte{}, nil
	}
	return p.buf, err
}
