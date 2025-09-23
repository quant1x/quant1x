package nn

import (
	"math"
	"math/rand"
	"sync"
)

// NeuralNetwork 神经网络结构
type NeuralNetwork struct {
	InputSize    int
	HiddenSize   int
	OutputSize   int
	LearningRate float64
	WeightsIH    [][]float64 // 输入到隐藏层权重
	WeightsHO    [][]float64 // 隐藏到输出层权重
	BiasH        []float64   // 隐藏层偏置
	BiasO        []float64   // 输出层偏置
}

// NewNeuralNetwork 初始化神经网络
func NewNeuralNetwork(input, hidden, output int, lr float64) *NeuralNetwork {
	nn := &NeuralNetwork{
		InputSize:    input,
		HiddenSize:   hidden,
		OutputSize:   output,
		LearningRate: lr,
	}

	// 使用Xavier初始化权重
	nn.WeightsIH = make([][]float64, hidden)
	nn.WeightsHO = make([][]float64, output)
	nn.BiasH = make([]float64, hidden)
	nn.BiasO = make([]float64, output)

	for i := range nn.WeightsIH {
		nn.WeightsIH[i] = make([]float64, input)
		for j := range nn.WeightsIH[i] {
			nn.WeightsIH[i][j] = rand.NormFloat64() * math.Sqrt(2.0/float64(input+hidden))
		}
	}

	for i := range nn.WeightsHO {
		nn.WeightsHO[i] = make([]float64, hidden)
		for j := range nn.WeightsHO[i] {
			nn.WeightsHO[i][j] = rand.NormFloat64() * math.Sqrt(2.0/float64(hidden+output))
		}
	}

	return nn
}

// 激活函数
func sigmoid(x float64) float64 {
	return 1.0 / (1.0 + math.Exp(-x))
}

func sigmoidDerivative(x float64) float64 {
	return x * (1.0 - x)
}

// Forward 前向传播（并发计算）
func (nn *NeuralNetwork) Forward(input []float64) (hidden, output []float64) {
	var wg sync.WaitGroup

	// 隐藏层计算
	hidden = make([]float64, nn.HiddenSize)
	wg.Add(nn.HiddenSize)
	for i := 0; i < nn.HiddenSize; i++ {
		go func(i int) {
			defer wg.Done()
			sum := nn.BiasH[i]
			for j := 0; j < nn.InputSize; j++ {
				sum += input[j] * nn.WeightsIH[i][j]
			}
			hidden[i] = sigmoid(sum)
		}(i)
	}

	// 输出层计算
	output = make([]float64, nn.OutputSize)
	wg.Add(nn.OutputSize)
	for i := 0; i < nn.OutputSize; i++ {
		go func(i int) {
			defer wg.Done()
			sum := nn.BiasO[i]
			for j := 0; j < nn.HiddenSize; j++ {
				sum += hidden[j] * nn.WeightsHO[i][j]
			}
			output[i] = sigmoid(sum)
		}(i)
	}

	wg.Wait()
	return
}

// Backpropagate 反向传播（带动量优化）
func (nn *NeuralNetwork) Backpropagate(input, target []float64) float64 {
	hidden, output := nn.Forward(input)

	// 计算输出层误差
	outputErrors := make([]float64, nn.OutputSize)
	totalError := 0.0
	for i := 0; i < nn.OutputSize; i++ {
		delta := target[i] - output[i]
		outputErrors[i] = delta * sigmoidDerivative(output[i])
		totalError += delta * delta
	}

	// 计算隐藏层误差
	hiddenErrors := make([]float64, nn.HiddenSize)
	for i := 0; i < nn.HiddenSize; i++ {
		var errorSum float64
		for j := 0; j < nn.OutputSize; j++ {
			errorSum += nn.WeightsHO[j][i] * outputErrors[j]
		}
		hiddenErrors[i] = errorSum * sigmoidDerivative(hidden[i])
	}

	// 并行更新权重
	var wg sync.WaitGroup
	alpha := 0.9 // 动量系数

	// 更新输出层权重
	wg.Add(nn.OutputSize)
	for i := 0; i < nn.OutputSize; i++ {
		go func(i int) {
			defer wg.Done()
			for j := 0; j < nn.HiddenSize; j++ {
				delta := nn.LearningRate * outputErrors[i] * hidden[j]
				nn.WeightsHO[i][j] += delta + alpha*delta // 动量项
			}
			nn.BiasO[i] += nn.LearningRate * outputErrors[i]
		}(i)
	}

	// 更新隐藏层权重
	wg.Add(nn.HiddenSize)
	for i := 0; i < nn.HiddenSize; i++ {
		go func(i int) {
			defer wg.Done()
			for j := 0; j < nn.InputSize; j++ {
				delta := nn.LearningRate * hiddenErrors[i] * input[j]
				nn.WeightsIH[i][j] += delta + alpha*delta
			}
			nn.BiasH[i] += nn.LearningRate * hiddenErrors[i]
		}(i)
	}

	wg.Wait()
	return totalError / float64(nn.OutputSize)
}

// Train 批量训练（带早停机制）
func (nn *NeuralNetwork) Train(inputs, targets [][]float64, epochs int, validationSplit float64) {
	valSize := int(float64(len(inputs)) * validationSplit)
	trainInputs := inputs[:len(inputs)-valSize]
	trainTargets := targets[:len(targets)-valSize]
	valInputs := inputs[len(inputs)-valSize:]
	valTargets := targets[len(targets)-valSize:]

	bestValLoss := math.MaxFloat64
	patience := 5
	wait := 0

	for e := 0; e < epochs; e++ {
		// 训练阶段
		trainLoss := 0.0
		for i := range trainInputs {
			trainLoss += nn.Backpropagate(trainInputs[i], trainTargets[i])
		}
		trainLoss /= float64(len(trainInputs))

		// 验证阶段
		valLoss := 0.0
		for i := range valInputs {
			_, output := nn.Forward(valInputs[i])
			for j := range output {
				delta := valTargets[i][j] - output[j]
				valLoss += delta * delta
			}
		}
		valLoss /= float64(len(valInputs) * nn.OutputSize)

		// 早停判断
		if valLoss < bestValLoss {
			bestValLoss = valLoss
			wait = 0
		} else {
			wait++
			if wait >= patience {
				break
			}
		}
	}
}
