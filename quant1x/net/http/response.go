package http

import "time"

type Response struct {
	StatusCode    int
	ContentLength int
	LastModified  time.Time
	Body          []byte
}
