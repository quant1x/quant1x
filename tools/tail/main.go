package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
	"sync"

	"gitee.com/quant1x/pkg/tools/tail"
	"gitee.com/quant1x/quant1x/quant1x/std"
	cli "github.com/spf13/cobra"
)

func main() {
	var tailConfig tail.Config

	cmd := &cli.Command{
		Use:     "tail [flags] FILE [FILE...]",
		Example: "tail -f runtime.log",
		Short:   "文件末端阅览",
		// disable pflag parsing so we parse args in Run and support legacy '-10'
		DisableFlagParsing: true,
		Run: func(cmd *cli.Command, args []string) {
			// custom parsing in Run (we are still within cobra's command lifecycle)
			if len(args) < 1 {
				_ = cmd.Usage()
				return
			}

			// defaults
			follow := false
			quiet := false
			verbose := false
			linesMode := true
			n := 10 // default lines
			c := 0  // bytes
			fromLines := false
			fromBytes := false
			files := make([]string, 0)

			// parse args manually to support -10 and other legacy forms
			for i := 0; i < len(args); {
				a := args[i]
				// help
				if a == "-h" || a == "--help" {
					_ = cmd.Help()
					return
				}
				if a == "-f" || a == "--follow" {
					follow = true
					i++
					continue
				}
				if a == "-q" || a == "--quiet" {
					quiet = true
					i++
					continue
				}
				if a == "-v" || a == "--verbose" {
					verbose = true
					i++
					continue
				}

				// --lines=NUM
				if strings.HasPrefix(a, "--lines=") {
					val := strings.TrimPrefix(a, "--lines=")
					if strings.HasPrefix(val, "+") {
						fromLines = true
						val = strings.TrimPrefix(val, "+")
					}
					if vv, err := strconv.Atoi(val); err == nil {
						n = vv
					}
					i++
					continue
				}
				if a == "--lines" && i+1 < len(args) {
					val := args[i+1]
					if strings.HasPrefix(val, "+") {
						fromLines = true
						val = strings.TrimPrefix(val, "+")
					}
					if vv, err := strconv.Atoi(val); err == nil {
						n = vv
					}
					i += 2
					continue
				}

				// -nNUM or -n=NUM or -n NUM
				if strings.HasPrefix(a, "-n") {
					val := strings.TrimPrefix(a, "-n")
					val = strings.TrimPrefix(val, "=")
					if val == "" {
						if i+1 < len(args) {
							val = args[i+1]
							i += 2
						} else {
							i++
						}
					} else {
						i++
					}
					if strings.HasPrefix(val, "+") {
						fromLines = true
						val = strings.TrimPrefix(val, "+")
					}
					if vv, err := strconv.Atoi(val); err == nil {
						n = vv
					}
					continue
				}

				// --bytes=NUM or --bytes NUM
				if strings.HasPrefix(a, "--bytes=") {
					val := strings.TrimPrefix(a, "--bytes=")
					if strings.HasPrefix(val, "+") {
						fromBytes = true
						val = strings.TrimPrefix(val, "+")
					}
					if vv, err := strconv.Atoi(val); err == nil {
						c = vv
						linesMode = false
					}
					i++
					continue
				}
				if a == "--bytes" && i+1 < len(args) {
					val := args[i+1]
					if strings.HasPrefix(val, "+") {
						fromBytes = true
						val = strings.TrimPrefix(val, "+")
					}
					if vv, err := strconv.Atoi(val); err == nil {
						c = vv
						linesMode = false
					}
					i += 2
					continue
				}

				// -cNUM or -c=NUM or -c NUM
				if strings.HasPrefix(a, "-c") {
					val := strings.TrimPrefix(a, "-c")
					val = strings.TrimPrefix(val, "=")
					if val == "" {
						if i+1 < len(args) {
							val = args[i+1]
							i += 2
						} else {
							i++
						}
					} else {
						i++
					}
					if strings.HasPrefix(val, "+") {
						fromBytes = true
						val = strings.TrimPrefix(val, "+")
					}
					if vv, err := strconv.Atoi(val); err == nil {
						c = vv
						linesMode = false
					}
					continue
				}

				// bare -NUM (GNU style) -> treat as -n NUM
				if strings.HasPrefix(a, "-") && len(a) > 1 {
					body := strings.TrimPrefix(a, "-")
					if strings.HasPrefix(body, "+") {
						body = strings.TrimPrefix(body, "+")
						if body != "" && isDigits(body) {
							if vv, err := strconv.Atoi(body); err == nil {
								n = vv
								i++
								continue
							}
						}
					} else if isDigits(body) {
						if vv, err := strconv.Atoi(body); err == nil {
							n = vv
							i++
							continue
						}
					}
				}

				// otherwise treat as filename
				files = append(files, a)
				i++
			}

			// validation: need at least one file
			if len(files) == 0 {
				_ = cmd.Usage()
				return
			}

			// apply parsed config
			tailConfig.Location = &tail.SeekInfo{Offset: 0, Whence: 2} // start at end of file by default
			tailConfig.Follow = follow
			if tailConfig.Follow {
				tailConfig.Poll = true
			}

			multiple := len(files) > 1
			var wg sync.WaitGroup

			for idx, filename := range files {
				filename = strings.TrimSpace(filename)
				filename, _ = std.ExpandUser(filename)

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
		},
		//PreRunE: func(cmd *cli.Command, args []string) error {
		//	//fmt.Println(args)
		//	if slices.Contains(args, "--help") || slices.Contains(args, "-h") {
		//		cmd.Usage()
		//	}
		//	return nil
		//},
	}
	cmd.SetUsageFunc(func(command *cli.Command) error {
		fmt.Println("Usage:\ntail [-f] [-n #] [file]")
		return nil
	})

	// Note: flag registration removed because we parse flags manually in Run

	if err := cmd.Execute(); err != nil {
		fmt.Println(err)
	}
}

// TailFile 跟踪文件更新 tail -f
func TailFile(filename string, config tail.Config, done chan bool) {
	defer func() { done <- true }()
	filename, _ = std.ExpandUser(filename)
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
	filename, _ = std.ExpandUser(filename)
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
	filename, _ = std.ExpandUser(filename)

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
