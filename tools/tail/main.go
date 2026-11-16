// Copyright (C) 2026 Quant1X Team
// SPDX-License-Identifier: MIT

package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
	"sync"

	"github.com/nxadm/tail"
	"github.com/quant1x/quant1x/quant1x/base"
	"github.com/quant1x/quant1x/quant1x/base/argparser"
)

const (
	flagFollow   = "follow"  // 跟随文件追加
	flagQuiet    = "quiet"   // 多文件时不打印文件名标题
	flagVerbose  = "verbose" // 始终打印文件名标题
	flagLines    = "lines"   // 按行显示的取值
	flagBytes    = "bytes"   // 按字节显示的取值
	defaultLines = 10        // 默认显示的行数
)

// buildParser 构建 tail 的命令行定义
func buildParser() (*argparser.Parser, error) {
	parser := argparser.NewParser("tail")
	parser.WithUsage("tail [-f] [-n #] [file]")
	switches := []struct {
		names []string
		help  string
	}{
		{[]string{"-f", "--follow"}, "跟随文件追加"},
		{[]string{"-q", "--quiet"}, "多文件时不打印文件名标题"},
		{[]string{"-v", "--verbose"}, "始终打印文件名标题"},
	}
	for _, item := range switches {
		if _, err := parser.AddArgument(item.names, argparser.Argument{
			Action: argparser.StoreTrue,
			Help:   item.help,
		}); err != nil {
			return nil, err
		}
	}
	if _, err := parser.AddArgument([]string{"-n", "--lines"}, argparser.Argument{
		Metavar: "NUM",
		Help:    "显示末尾 N 行, +N 表示从第 N 行开始",
	}); err != nil {
		return nil, err
	}
	if _, err := parser.AddArgument([]string{"-c", "--bytes"}, argparser.Argument{
		Metavar: "NUM",
		Help:    "显示末尾 N 字节, +N 表示从第 N 字节开始",
	}); err != nil {
		return nil, err
	}
	if _, err := parser.AddArgument([]string{"file"}, argparser.Argument{
		Nargs: argparser.NargsOneOrMore(),
		Help:  "要查看的文件",
	}); err != nil {
		return nil, err
	}
	return parser, nil
}

func main() {
	parser, err := buildParser()
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(2)
	}
	namespace, code := parser.ParseOrExit(normalizeBareCount(os.Args[1:]), os.Stdout, os.Stderr)
	if code != 0 || namespace == nil {
		os.Exit(code)
	}

	linesText, hasLines := namespaceText(namespace, flagLines)
	bytesText, hasBytes := namespaceText(namespace, flagBytes)
	if hasLines && hasBytes {
		fmt.Fprintln(os.Stderr, "error: cannot combine --lines and --bytes")
		os.Exit(2)
	}

	linesMode := true
	n := defaultLines
	c := 0
	fromLines := false
	fromBytes := false
	if hasBytes {
		linesMode = false
		c, fromBytes = parseCount(bytesText, c)
	} else {
		n, fromLines = parseCount(linesText, n)
	}

	files := namespaceStrings(namespace, "file")
	if len(files) == 0 {
		parser.PrintHelp(os.Stderr, false)
		os.Exit(2)
	}
	runTail(files,
		namespaceBool(namespace, flagFollow),
		namespaceBool(namespace, flagQuiet),
		namespaceBool(namespace, flagVerbose),
		linesMode, n, c, fromLines, fromBytes)
}

// runTail 按解析结果输出文件内容, 需要时持续跟随
func runTail(files []string, follow, quiet, verbose, linesMode bool, n, c int, fromLines, fromBytes bool) {
	tailConfig := tail.Config{Location: &tail.SeekInfo{Offset: 0, Whence: 2}} // 默认从文件末尾开始
	tailConfig.Follow = follow
	if tailConfig.Follow {
		tailConfig.Poll = true
	}

	multiple := len(files) > 1
	var wg sync.WaitGroup
	for idx, filename := range files {
		filename = strings.TrimSpace(filename)
		filename, _ = base.ExpandUser(filename)

		showHeader := multiple && !quiet
		if verbose {
			showHeader = true
		}
		if showHeader {
			if idx > 0 {
				fmt.Println()
			}
			fmt.Printf("==> %s <==\n", filename)
		}

		// initial dump
		if linesMode {
			if fromLines {
				printFromLine(filename, n)
			} else {
				printLastLines(filename, n)
			}
		} else {
			if fromBytes {
				printFromByte(filename, c)
			} else {
				printLastBytes(filename, c)
			}
		}

		if tailConfig.Follow {
			wg.Add(1)
			go func(fn string) {
				defer wg.Done()
				prefix := ""
				if multiple {
					prefix = fn + ": "
				}
				TailFileFollow(fn, tailConfig, prefix)
			}(filename)
		}
	}

	if tailConfig.Follow {
		wg.Wait()
	}
}

// parseCount 解析行数 / 字节数; `+N` 表示从第 N 行 / 第 N 字节开始
func parseCount(text string, fallback int) (value int, fromStart bool) {
	if text == "" {
		return fallback, false
	}
	if trimmed, ok := strings.CutPrefix(text, "+"); ok {
		fromStart = true
		text = trimmed
	}
	parsed, err := strconv.Atoi(text)
	if err != nil {
		return fallback, false
	}
	return parsed, fromStart
}

// normalizeBareCount 把 GNU 风格的裸数字 (-10 / -+10) 归一成 --lines=N
func normalizeBareCount(args []string) []string {
	normalized := make([]string, 0, len(args))
	for _, token := range args {
		if isBareCount(token) {
			normalized = append(normalized, "--"+flagLines+"="+strings.TrimPrefix(token, "-"))
			continue
		}
		normalized = append(normalized, token)
	}
	return normalized
}

// isBareCount token 是否形如 -10 或 -+10
func isBareCount(token string) bool {
	if !strings.HasPrefix(token, "-") || strings.HasPrefix(token, "--") {
		return false
	}
	body := strings.TrimPrefix(token, "-")
	body = strings.TrimPrefix(body, "+")
	return isDigits(body)
}

// namespaceBool 读取布尔开关
func namespaceBool(namespace *argparser.Namespace, key string) bool {
	value, ok := namespace.Get(key)
	if !ok {
		return false
	}
	flag, _ := value.(bool)
	return flag
}

// namespaceText 读取文本取值, 同时报告该选项是否出现
func namespaceText(namespace *argparser.Namespace, key string) (string, bool) {
	value, ok := namespace.Get(key)
	if !ok {
		return "", false
	}
	text, ok := value.(string)
	return text, ok
}

// namespaceStrings 读取多值位置参数
func namespaceStrings(namespace *argparser.Namespace, key string) []string {
	value, ok := namespace.Get(key)
	if !ok {
		return nil
	}
	switch typed := value.(type) {
	case []any:
		files := make([]string, 0, len(typed))
		for _, item := range typed {
			if text, ok := item.(string); ok {
				files = append(files, text)
			}
		}
		return files
	case []string:
		return typed
	case string:
		return []string{typed}
	}
	return nil
}

// TailFile 跟踪文件更新 tail -f
func TailFile(filename string, config tail.Config, done chan bool) {
	defer func() { done <- true }()
	filename, _ = base.ExpandUser(filename)
	t, err := tail.TailFile(filename, config)
	if err != nil {
		fmt.Println(err)
		return
	}
	for line := range t.Lines {
		fmt.Println(line.Text)
	}
	err = t.Wait()
	if err != nil {
		fmt.Println(err)
	}
}

// TailFileFollow follows file and prints lines with optional prefix
func TailFileFollow(filename string, config tail.Config, prefix string) {
	filename, _ = base.ExpandUser(filename)
	t, err := tail.TailFile(filename, config)
	if err != nil {
		fmt.Println(err)
		return
	}
	for line := range t.Lines {
		if prefix != "" {
			fmt.Print(prefix)
		}
		fmt.Println(line.Text)
	}
	_ = t.Wait()
}

// printLastLines prints the last n lines of a file
func printLastLines(filename string, n int) {
	if n <= 0 {
		return
	}
	f, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer f.Close()
	reader := bufio.NewReader(f)
	ring := make([]string, 0, n)
	for {
		line, err := reader.ReadString('\n')
		if len(line) > 0 {
			line = strings.TrimRight(line, "\r\n")
			ring = append(ring, line)
			if len(ring) > n {
				ring = ring[1:]
			}
		}
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Println(err)
			break
		}
	}
	for _, line := range ring {
		fmt.Println(line)
	}
}

// isDigits returns true if s consists of ASCII digits
func isDigits(s string) bool {
	if s == "" {
		return false
	}
	for _, r := range s {
		if r < '0' || r > '9' {
			return false
		}
	}
	return true
}

// printFromLine prints file from the given 1-based line number
func printFromLine(filename string, from int) {
	if from <= 0 {
		from = 1
	}
	f, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer f.Close()
	scanner := bufio.NewScanner(f)
	buf := make([]byte, 0, 64*1024)
	scanner.Buffer(buf, 10*1024*1024)
	i := 0
	for scanner.Scan() {
		i++
		if i < from {
			continue
		}
		fmt.Println(scanner.Text())
	}
	if err := scanner.Err(); err != nil {
		fmt.Println(err)
	}
}

// printLastBytes prints the last c bytes of a file
func printLastBytes(filename string, c int) {
	if c <= 0 {
		return
	}
	f, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer f.Close()
	stat, err := f.Stat()
	if err != nil {
		fmt.Println(err)
		return
	}
	size := stat.Size()
	var start int64 = 0
	if int64(c) < size {
		start = size - int64(c)
	}
	_, err = f.Seek(start, 0)
	if err != nil {
		fmt.Println(err)
		return
	}
	buf := make([]byte, 8192)
	for {
		n, err := f.Read(buf)
		if n > 0 {
			os.Stdout.Write(buf[:n])
		}
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Println(err)
			break
		}
	}
}

// printFromByte prints from the given 1-based byte offset
func printFromByte(filename string, from int) {
	if from <= 0 {
		from = 1
	}
	f, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer f.Close()
	_, err = f.Seek(int64(from-1), 0)
	if err != nil {
		fmt.Println(err)
		return
	}
	buf := make([]byte, 8192)
	for {
		n, err := f.Read(buf)
		if n > 0 {
			os.Stdout.Write(buf[:n])
		}
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Println(err)
			break
		}
	}
}

// TailFileWithNumber 查看最后n行数据
func TailFileWithNumber(filename string, config tail.Config, n int) {
	filename, _ = base.ExpandUser(filename)

	// If not following, just read the file and keep a ring buffer of last n lines
	if !config.Follow {
		f, err := os.Open(filename)
		if err != nil {
			fmt.Println(err)
			return
		}
		defer f.Close()

		reader := bufio.NewReader(f)
		ring := make([]string, 0, n)
		for {
			line, err := reader.ReadString('\n')
			if len(line) > 0 {
				line = strings.TrimRight(line, "\r\n")
				ring = append(ring, line)
				if len(ring) > n {
					ring = ring[1:]
				}
			}
			if err != nil {
				if err == io.EOF {
					break
				}
				fmt.Println(err)
				break
			}
		}
		for _, line := range ring {
			fmt.Println(line)
		}
		return
	}

	// If following: first print last n lines, then follow new lines
	// Print last n lines by reading file first
	f, err := os.Open(filename)
	if err != nil {
		fmt.Println(err)
		return
	}
	reader := bufio.NewReader(f)
	ring := make([]string, 0, n)
	for {
		line, err := reader.ReadString('\n')
		if len(line) > 0 {
			line = strings.TrimRight(line, "\r\n")
			ring = append(ring, line)
			if len(ring) > n {
				ring = ring[1:]
			}
		}
		if err != nil {
			if err == io.EOF {
				break
			}
			fmt.Println(err)
			break
		}
	}
	_ = f.Close()
	for _, line := range ring {
		fmt.Println(line)
	}

	// Now follow appended lines
	done := make(chan bool)
	TailFile(filename, config, done)
	<-done
}
