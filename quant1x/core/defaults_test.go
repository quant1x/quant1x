package core

import (
	"testing"
	"time"
)

type innerDefault struct {
	Name string `default:"inner"`
}

type cfgDefault struct {
	Host    string        `default:"127.0.0.1"`
	Port    int           `default:"8080"`
	Enabled bool          `default:"true"`
	Timeout time.Duration `default:"5s"`
	Inner   innerDefault
	PtrI    *int `default:"42"`
}

func TestApplyDefaults_Basics(t *testing.T) {
	var cfg cfgDefault
	if err := ApplyDefaults(&cfg); err != nil {
		t.Fatalf("ApplyDefaults returned error: %v", err)
	}
	if cfg.Host != "127.0.0.1" {
		t.Fatalf("Host = %q", cfg.Host)
	}
	if cfg.Port != 8080 {
		t.Fatalf("Port = %d", cfg.Port)
	}
	if cfg.Enabled != true {
		t.Fatalf("Enabled = %v", cfg.Enabled)
	}
	if cfg.Timeout != 5*time.Second {
		t.Fatalf("Timeout = %v", cfg.Timeout)
	}
	if cfg.Inner.Name != "inner" {
		t.Fatalf("Inner.Name = %q", cfg.Inner.Name)
	}
	if cfg.PtrI == nil || *cfg.PtrI != 42 {
		if cfg.PtrI == nil {
			t.Fatalf("PtrI is nil")
		}
		t.Fatalf("*PtrI = %d", *cfg.PtrI)
	}
}

func TestApplyDefaults_DoesNotOverrideNonZero(t *testing.T) {
	cfg := cfgDefault{Host: "x", Port: 1, Enabled: false, Timeout: 2 * time.Second}
	if err := ApplyDefaults(&cfg); err != nil {
		t.Fatalf("ApplyDefaults returned error: %v", err)
	}
	if cfg.Host != "x" || cfg.Port != 1 || cfg.Timeout != 2*time.Second {
		t.Fatalf("unexpected override: %+v", cfg)
	}
	// Enabled is a bit special: false is zero value, so default should apply.
	if cfg.Enabled != true {
		t.Fatalf("Enabled should be defaulted to true, got %v", cfg.Enabled)
	}
}
