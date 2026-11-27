// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package indicators

import (
	"encoding/csv"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"

	"github.com/quant1x/quant1x/quant1x/data/schema"
	"github.com/quant1x/quant1x/quant1x/formula"
)

func readTable(t *testing.T, name string) ([]string, [][]string) {
	t.Helper()
	path := filepath.Join("..", "..", "tests", "fixtures", "formula", "indicators", name)
	stream, err := os.Open(path)
	if err != nil {
		t.Fatalf("打开 %s 失败: %v", name, err)
	}
	defer stream.Close()
	records, err := csv.NewReader(stream).ReadAll()
	if err != nil {
		t.Fatalf("读取 %s 失败: %v", name, err)
	}
	return records[0], records[1:]
}

func loadBars(t *testing.T) []schema.Bar {
	t.Helper()
	header, rows := readTable(t, "bars.csv")
	index := map[string]int{}
	for i, h := range header {
		index[h] = i
	}
	bars := make([]schema.Bar, len(rows))
	for i, row := range rows {
		bars[i] = schema.Bar{
			Date:   row[index["date"]],
			Open:   parseDouble(row[index["open"]]),
			Close:  parseDouble(row[index["close"]]),
			High:   parseDouble(row[index["high"]]),
			Low:    parseDouble(row[index["low"]]),
			Volume: parseDouble(row[index["volume"]]),
			Amount: parseDouble(row[index["amount"]]),
		}
	}
	return bars
}

func parseDouble(text string) float64 {
	if text == "" {
		return math.NaN()
	}
	v, err := strconv.ParseFloat(text, 64)
	if err != nil {
		return math.NaN()
	}
	return v
}

func expectedColumn(t *testing.T, name, column string) []string {
	t.Helper()
	header, rows := readTable(t, name)
	index := -1
	for i, h := range header {
		if h == column {
			index = i
			break
		}
	}
	if index < 0 {
		t.Fatalf("%s 缺列 %s", name, column)
	}
	out := make([]string, len(rows))
	for i, row := range rows {
		out[i] = row[index]
	}
	return out
}

func checkSeries(t *testing.T, label string, actual formula.Series, expected []string) {
	t.Helper()
	if len(actual) != len(expected) {
		t.Errorf("%s: 长度 %d vs %d", label, len(actual), len(expected))
		return
	}
	for i := range actual {
		want := parseDouble(expected[i])
		ok := (math.IsNaN(actual[i]) && math.IsNaN(want)) || actual[i] == want
		if !ok {
			t.Errorf("%s [%d]: %v vs %v", label, i, actual[i], want)
			return
		}
	}
}

func checkBools(t *testing.T, label string, actual formula.Bools, expected []string) {
	t.Helper()
	if len(actual) != len(expected) {
		t.Errorf("%s: 长度 %d vs %d", label, len(actual), len(expected))
		return
	}
	for i := range actual {
		want := strings.TrimSpace(expected[i]) != "" && strings.TrimSpace(expected[i]) != "0"
		if actual[i] != want {
			t.Errorf("%s [%d]: %v vs %v", label, i, actual[i], expected[i])
			return
		}
	}
}

func TestIndicatorsParity(t *testing.T) {
	bars := loadBars(t)

	cdtd := Cdtd(bars)
	checkSeries(t, "cdtd.wen", cdtd.Wen, expectedColumn(t, "cdtd.csv", "cdtd.wen"))
	checkBools(t, "cdtd.S1", cdtd.S1, expectedColumn(t, "cdtd.csv", "cdtd.S1"))
	checkBools(t, "cdtd.B", cdtd.B, expectedColumn(t, "cdtd.csv", "cdtd.B"))
	checkBools(t, "cdtd.S2", cdtd.S2, expectedColumn(t, "cdtd.csv", "cdtd.S2"))
	checkBools(t, "cdtd.S3", cdtd.S3, expectedColumn(t, "cdtd.csv", "cdtd.S3"))

	f89k := F89k(bars)
	checkSeries(t, "f89k.zs", f89k.Zs, expectedColumn(t, "f89k.csv", "f89k.zs"))
	checkBools(t, "f89k.b", f89k.B, expectedColumn(t, "f89k.csv", "f89k.b"))
	checkSeries(t, "f89k.DN", f89k.Dn, expectedColumn(t, "f89k.csv", "f89k.DN"))

	hjk := Hjk(bars, 0.10)
	checkBools(t, "hjk.b", hjk.B, expectedColumn(t, "hjk.csv", "hjk.b"))

	ma1x := Ma1x(bars)
	checkSeries(t, "ma1x.DN", ma1x.Dn, expectedColumn(t, "ma1x.csv", "DN"))
	checkSeries(t, "ma1x.ZS", ma1x.Zs, expectedColumn(t, "ma1x.csv", "ZS"))
	checkSeries(t, "ma1x.YL", ma1x.Yl, expectedColumn(t, "ma1x.csv", "YL"))
	checkBools(t, "ma1x.hc", ma1x.Hc, expectedColumn(t, "ma1x.csv", "ma1x.hc"))
	checkBools(t, "ma1x.tp", ma1x.Tp, expectedColumn(t, "ma1x.csv", "ma1x.tp"))

	p2fb := P2fb(bars, 0.10)
	checkSeries(t, "p2fb.hgzx", p2fb.Hgzx, expectedColumn(t, "p2fb.csv", "p2fb.hgzx"))
}
