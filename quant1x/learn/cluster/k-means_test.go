package cluster

import (
	"fmt"
	"testing"
)

func TestKMeans_Basic(t *testing.T) {
	data := [][]float64{
		{1.0, 2.0}, {1.1, 2.1}, {0.9, 1.9}, {1.2, 2.0},
		{5.0, 5.0}, {5.1, 5.1}, {5.2, 4.9}, {4.9, 5.1},
		{10.0, 10.0}, {10.1, 9.9}, {9.9, 10.1},
	}

	km, err := NewKMeans(3, 100, 1e-4)
	if err != nil {
		t.Fatal(err)
	}
	err = km.Fit(data)
	if err != nil {
		t.Fatal(err)
	}

	fmt.Println("聚类中心:")
	for i, c := range km.Centroids {
		fmt.Printf("簇%d: %v\n", i, c)
	}

	fmt.Println("\n每个点的标签:")
	for i, p := range data {
		fmt.Printf("点%v -> 簇%d\n", p, km.Labels[i])
	}

	fmt.Printf("\nInertia: %.4f\n", km.Inertia)
	sihouetteScore, err := km.SilhouetteScore(data)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Printf("轮廓系数: %.4f\n", sihouetteScore)

	sizes, err := km.GetClusterSizes()
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println("\n簇大小:")
	for i, s := range sizes {
		fmt.Printf("簇%d: %d个点\n", i, s)
	}

	newPoints := [][]float64{{1.5, 2.0}, {5.5, 5.0}, {10.5, 10.0}}
	newLabels, _ := km.Predict(newPoints)
	fmt.Println("\n新点预测:")
	for i, p := range newPoints {
		fmt.Printf("点%v -> 簇%d\n", p, newLabels[i])
	}
}
