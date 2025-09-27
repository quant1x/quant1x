package tests

import (
    "testing"
)

// Test basic bit-reader y() behavior: reading individual bits from indices
func TestYReadsBits(t *testing.T) {
    d := NewCalendarDecoder("")
    // prepare indices: single base64 index with value 1 (binary ...000001)
    d.indices = []int{1}
    d.n = 1
    d.e = 0
    d.o = 0

    // first bit (o=0) should be 1
    if got := d.y(); !got {
        t.Fatalf("expected first bit true, got false")
    }
    // second bit (o=1) should be 0
    if got := d.y(); got {
        t.Fatalf("expected second bit false, got true")
    }
}

// Test N() variable-length integer behavior for simple cases
func TestNBasic(t *testing.T) {
    d := NewCalendarDecoder("")
    // case: bits start with 1 then 0 => N() == 1
    d.indices = []int{1}
    d.n = 1
    d.e = 0
    d.o = 0
    if got := d.N(); got != 1 {
        t.Fatalf("expected N()==1, got %d", got)
    }

    // case: bits start with 0 then 0 => N() == -1
    d.indices = []int{0}
    d.n = 1
    d.e = 0
    d.o = 0
    if got := d.N(); got != -1 {
        t.Fatalf("expected N()==-1, got %d", got)
    }
}

// Test w() to read a small number of bits (3 bits) from a single index
func TestWReadsSmall(t *testing.T) {
    d := NewCalendarDecoder("")
    // index value 5 (binary 000101) when reading 3 bits -> 5
    d.indices = []int{5}
    d.n = 1
    d.e = 0
    d.o = 0
    out := d.w([]int{3}, nil, nil)
    if len(out) != 1 {
        t.Fatalf("unexpected length from w(): %d", len(out))
    }
    if out[0] != 5 {
        t.Fatalf("expected w to read 5, got %d", out[0])
    }
}
