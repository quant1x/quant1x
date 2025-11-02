package net

import (
    "testing"
    stdnet "net"
)

func TestAddAcquireRelease_HappyPath(t *testing.T) {
    m := NewEndpointManager()

    ok := m.AddEndpoint("127.0.0.1", 8000, 2)
    if !ok {
        t.Fatalf("expected AddEndpoint to succeed")
    }

    // first acquire
    addr1, ok1 := m.AcquireEndpoint()
    if !ok1 || addr1 == nil {
        t.Fatalf("expected first acquire to succeed")
    }
    if addr1.Port != 8000 {
        t.Fatalf("expected port 8000, got %d", addr1.Port)
    }

    // second acquire should also succeed (max 2)
    addr2, ok2 := m.AcquireEndpoint()
    if !ok2 || addr2 == nil {
        t.Fatalf("expected second acquire to succeed")
    }

    // third acquire should fail because max connections reached
    addr3, ok3 := m.AcquireEndpoint()
    if ok3 || addr3 != nil {
        t.Fatalf("expected third acquire to fail when max connections reached")
    }

    // release one and acquire again
    m.ReleaseEndpoint(addr1)
    addr4, ok4 := m.AcquireEndpoint()
    if !ok4 || addr4 == nil {
        t.Fatalf("expected acquire to succeed after release")
    }
}

func TestBoundary_NoEndpointsAndInvalidPort(t *testing.T) {
    m := NewEndpointManager()

    // Acquire on empty manager should return nil,false
    addr, ok := m.AcquireEndpoint()
    if ok || addr != nil {
        t.Fatalf("expected no endpoint available on empty manager")
    }

    // invalid port (65535) should be rejected
    ok2 := m.AddEndpoint("127.0.0.1", 65535, 1)
    if ok2 {
        t.Fatalf("expected AddEndpoint to reject port 65535")
    }

    // invalid IP should be rejected
    ok3 := m.AddEndpoint("not-an-ip", 9000, 1)
    if ok3 {
        t.Fatalf("expected AddEndpoint to reject invalid IP string")
    }

    // Also test AddEndpointAddr with a nil addr
    ok4 := m.AddEndpointAddr(nil, 1)
    if ok4 {
        t.Fatalf("expected AddEndpointAddr(nil) to return false")
    }

    // Add a valid endpoint and check GetEndpointStats
    ok5 := m.AddEndpoint("127.0.0.1", 9001, 1)
    if !ok5 {
        t.Fatalf("expected AddEndpoint to succeed for 127.0.0.1:9001")
    }
    // resolve addr for stats lookup
    resolved, err := stdnet.ResolveTCPAddr("tcp", "127.0.0.1:9001")
    if err != nil {
        t.Fatalf("failed to resolve tcp addr: %v", err)
    }
    max, active, err := m.GetEndpointStats(resolved)
    if err != nil {
        t.Fatalf("GetEndpointStats returned error: %v", err)
    }
    if max != 1 || active != 0 {
        t.Fatalf("unexpected stats: max=%d active=%d", max, active)
    }
}
