package num

import (
	"math/rand/v2"
	"sync"
)

const (
	benchAlignLength  = 5000
	benchAlignInitNum = 1024
)

var (
	testalignOnce    sync.Once
	testDataFloat32  []float32
	testDataFloat32y []float32
	testDataFloat64  []float64
	testDataFloat64y []float64
)

func initTestData() {
	testDataFloat32 = make([]float32, benchAlignInitNum)
	testDataFloat32y = make([]float32, benchAlignInitNum)
	testDataFloat64 = make([]float64, benchAlignInitNum)
	testDataFloat64y = make([]float64, benchAlignInitNum)
	for i := 0; i < benchAlignInitNum; i++ {
		testDataFloat32[i] = rand.Float32()
		testDataFloat32y[i] = rand.Float32()
		testDataFloat64[i] = rand.Float64()
		testDataFloat64y[i] = rand.Float64()
	}
}
