package id

import (
	"math/rand"
	"testing"
	"time"
)

func TestUint128_Compare(t *testing.T) {
	a := NewUint128(1, 2)
	b := NewUint128(1, 3)
	c := NewUint128(2, 1)

	if !a.Lt(b) {
		t.Fatal("a < b")
	}
	if !b.Gt(a) {
		t.Fatal("b > a")
	}
	if !c.Gt(b) {
		t.Fatal("c > b")
	}
}

func TestUint128_Add(t *testing.T) {
	a := NewUint128(0, ^uint64(0))
	b := NewUint128(0, 1)

	c := a.Add(b)
	if c.Hi != 1 || c.Lo != 0 {
		t.Fatalf("add carry failed: %#v", c)
	}
}

func TestUint128_Lsh(t *testing.T) {
	u := From64(1)
	v := u.Lsh(64)
	if v.Hi != 1 || v.Lo != 0 {
		t.Fatalf("lsh 64 failed: %#v", v)
	}
}

func TestUint128_BytesRoundTrip(t *testing.T) {
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))

	for i := 0; i < 10000; i++ {
		u := NewUint128(rng.Uint64(), rng.Uint64())
		b := u.Bytes()
		v := FromBytes(b)
		if u != v {
			t.Fatalf("roundtrip failed:\n u=%#v\n v=%#v", u, v)
		}
	}
}
