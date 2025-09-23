package num

//go:noescape
func sse2Add(a, b, c *float64, n int)
