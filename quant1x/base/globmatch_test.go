// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package base

import "testing"

// 与 Python / Rust / C++ / Zig 同一批样本逐条对齐 (quant1x/base/globmatch_test.py)。

func TestGlobstarZeroOrMoreSegments(t *testing.T) {
	cases := []struct {
		path, pattern string
		want          bool
	}{
		{"stb.h", "**/*.h", true},
		{"a/b/c.h", "**/*.h", true},
		{"a/c.h", "a/**/c.h", true},
		{"a/x/y/c.h", "a/**/c.h", true},
		{"third_party/stb.h", "third_party/**", true},
		{"third_party/a/b.h", "third_party/**", true},
	}
	for _, c := range cases {
		if got := Match(c.path, c.pattern); got != c.want {
			t.Errorf("Match(%q, %q) = %v, want %v", c.path, c.pattern, got, c.want)
		}
	}
}

func TestAnchored(t *testing.T) {
	cases := []struct {
		path, pattern string
		want          bool
	}{
		{"libs/stb.h", "libs/*.h", true},
		{"libs/sub/stb.h", "libs/*.h", false},
		{"x/libs/stb.h", "libs/*.h", false},
		{"x/third_party/a.h", "third_party/**", false},
		{"third_partyish/x.h", "third_party/**", false},
		{"b/a/c.h", "a/**/c.h", false},
	}
	for _, c := range cases {
		if got := Match(c.path, c.pattern); got != c.want {
			t.Errorf("Match(%q, %q) = %v, want %v", c.path, c.pattern, got, c.want)
		}
	}
}

func TestSingleSegmentWildcards(t *testing.T) {
	cases := []struct {
		path, pattern string
		want          bool
	}{
		{"a.h", "*.h", true},
		{"a.c", "*.h", false},
		{"a1.h", "a?.h", true},
		{"a1.h", "a[0-9].h", true},
		{"a.h", "a[0-9].h", false},
		{"a5.h", "a[!0-9].h", false},
		{"ax.h", "a[!0-9].h", true},
		{"a-.h", "a[a-].h", true},
		{"az.h", "a[a-].h", false},
		{"a[b.h", "a[b.h", true},
	}
	for _, c := range cases {
		if got := Match(c.path, c.pattern); got != c.want {
			t.Errorf("Match(%q, %q) = %v, want %v", c.path, c.pattern, got, c.want)
		}
	}
}

func TestRootAbsoluteForm(t *testing.T) {
	cases := []struct {
		path, pattern string
		want          bool
	}{
		{"/third_party/a.h", "third_party/**", true},
		{"third_party/a.h", "/third_party/**", true},
		{"/third_party/a.h", "/third_party/**", true},
		{"/x/third_party/a.h", "/third_party/**", false},
		{"/a/b.h", "/**/*.h", true},
	}
	for _, c := range cases {
		if got := Match(c.path, c.pattern); got != c.want {
			t.Errorf("Match(%q, %q) = %v, want %v", c.path, c.pattern, got, c.want)
		}
	}
}

func TestDotfileAndCase(t *testing.T) {
	if !Match(".hidden.h", "*.h") {
		t.Error(`Match(".hidden.h", "*.h") = false, want true`)
	}
	if Match("A.H", "*.h") {
		t.Error(`Match("A.H", "*.h") = true, want false`)
	}
}

func TestTrailingGlobstarNeedsOneSegment(t *testing.T) {
	cases := []struct {
		path, pattern string
		want          bool
	}{
		{"a", "a/**", false},
		{"a/b", "a/**", true},
		{"a/b/c", "a/**", true},
		{"a", "**", true},
	}
	for _, c := range cases {
		if got := Match(c.path, c.pattern); got != c.want {
			t.Errorf("Match(%q, %q) = %v, want %v", c.path, c.pattern, got, c.want)
		}
	}
}

func TestEmptyInputs(t *testing.T) {
	if Match("", "**") {
		t.Error(`Match("", "**") = true, want false`)
	}
	if Match("a.h", "") {
		t.Error(`Match("a.h", "") = true, want false`)
	}
	if MatchAny("a.h", nil) {
		t.Error("MatchAny(a.h, nil) = true, want false")
	}
}

func TestMatchAny(t *testing.T) {
	if !MatchAny("libs/a.h", []string{"third_party/**", "libs/*.h"}) {
		t.Error("MatchAny hit = false, want true")
	}
	if MatchAny("libs/a.h", []string{"third_party/**", "**.c"}) {
		t.Error("MatchAny hit = true, want false")
	}
}
