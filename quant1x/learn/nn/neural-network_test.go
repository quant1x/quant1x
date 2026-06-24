package nn

import (
	"fmt"
	"math/rand"
	"testing"
	"time"
)

func Test_Naive(t *testing.T) {
	rand.Seed(time.Now().UnixNano())

	// 创建神经网络: 2输入, 4隐藏, 1输出
	nn := NewNeuralNetwork(2, 4, 1, 0.1)

	// XOR训练数据
	inputs := [][]float64{
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1},
	}
	targets := [][]float64{
		{0},
		{1},
		{1},
		{0},
	}

	// 训练1000轮
	nn.Train(inputs, targets, 1000, 0.2)

	// 测试
	for _, input := range inputs {
		_, output := nn.Forward(input)
		fmt.Printf("%v => %.3f\n", input, output[0])
	}
}
