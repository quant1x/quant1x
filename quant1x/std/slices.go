package std

import "slices"

// Reverse 反转切片
func Reverse[S ~[]E, E any](s S) S {
	d := slices.Clone(s)
	for i, j := 0, len(d)-1; i < j; i, j = i+1, j-1 {
		d[i], d[j] = d[j], d[i]
	}
	return d
}

// Filter slice过滤
func Filter[S ~[]E, E any](slice S, condition func(E) bool) S {
	var filtered []E
	for _, item := range slice {
		if condition(item) {
			filtered = append(filtered, item)
		}
	}
	return filtered
}
