package log

import (
	"log"
)

// A tiny project-local logging wrapper to provide leveled logging for Go code in this repo.
// This is intentionally minimal: it forwards to the standard library logger but prefixes level tags.

func Infof(format string, v ...interface{}) {
	log.Printf("[INFO] "+format, v...)
}

func Debugf(format string, v ...interface{}) {
	log.Printf("[DEBUG] "+format, v...)
}

func Errorf(format string, v ...interface{}) {
	log.Printf("[ERROR] "+format, v...)
}
