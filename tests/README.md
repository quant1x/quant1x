# 项目测试 约定
## 1. 单元测试
## 2. 基准测试
## 3. 集成测试

## calendar decoder parity

下面是重现 JS/Go 解码器输出并做逐条比对的步骤：

生成 JS 参考输出（需要 Node.js）：

```powershell
node tools/js_calendar_wrapper.js > tests/js_calendar_output_clean.json
```

生成 Go 输出（需要 Go）：

```powershell
go run tools/gen_go_calendar_output.go > tests/go_calendar_output.json
```

运行并查看 parity 单元测试：

```powershell
go test ./tests -run TestCalendarParity -v
```

说明：
- `tools/` 下的辅助 Go 程序使用 `//go:build tools` 标记，默认不会被 `go test ./...` 编译。
- parity 测试会容忍常见的 BOM/编码问题（UTF-8 BOM、UTF-16LE）。