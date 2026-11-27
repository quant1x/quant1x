// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package num

// avx2Enabled 记录调用方探测到的 AVX2 可用性; 本包不做向量化, 只保存这一开关,
// 供调用方在启动时设置、在诊断输出中读取。
var avx2Enabled bool

// SetAvx2Enabled 设置 AVX2 开关。
func SetAvx2Enabled(enabled bool) { avx2Enabled = enabled }

// GetAvx2Enabled 读取 AVX2 开关。
func GetAvx2Enabled() bool { return avx2Enabled }
