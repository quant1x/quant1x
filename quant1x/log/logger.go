package log

import (
	"compress/gzip"
	"fmt"
	"io"
	"os"
	"path/filepath"

	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
	"github.com/quant1x/quant1x/quant1x/base"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

var (
	mapLevelToFilename = map[zapcore.Level]string{
		zapcore.DebugLevel:  "debug",
		zapcore.InfoLevel:   "info",
		zapcore.WarnLevel:   "warn",
		zapcore.ErrorLevel:  "error",
		zapcore.DPanicLevel: "fatal",
		zapcore.PanicLevel:  "fatal",
		zapcore.FatalLevel:  "fatal",
	}
	console = zapcore.AddSync(os.Stdout)
)

func getLogger(cfg Config, level zapcore.Level) (zapcore.Core, error) {
	filename, ok := mapLevelToFilename[level]
	if !ok {
		return nil, fmt.Errorf("invalid log level: %v", level)
	}
	// 配置日志滚动器, 按天切割
	path := filepath.Join(cfg.Path, filename+"_%Y%m%d.log")
	rl, err := rotatelogs.New(
		path,                              // 文件名格式, 带日期
		rotatelogs.WithMaxAge(cfg.MaxAge), // 日志最大保留时间
		rotatelogs.WithRotationTime(cfg.RotationTime), // 日志切割时间
		rotatelogs.WithHandler(rotatelogs.HandlerFunc(
			func(e rotatelogs.Event) {
				if e.Type() == rotatelogs.FileRotatedEventType {
					if fre, ok := e.(*rotatelogs.FileRotatedEvent); ok {
						oldFilename := fre.PreviousFile()
						if oldFilename == "" {
							return
						}
						compressOldLogs(oldFilename)
					}
				}
			})),
	)
	if err != nil {
		return nil, err
	}
	writeSyncer := zapcore.AddSync(rl)
	// 带缓冲的 WriteSyncer
	bufferedWriteSyncer := &zapcore.BufferedWriteSyncer{
		WS:            writeSyncer,
		Size:          cfg.BufferSize * 1024, // 缓冲区大小, 单位 KB
		FlushInterval: cfg.FlushInterval,     // 定时刷新间隔
	}
	var syncers []zapcore.WriteSyncer
	syncers = append(syncers, bufferedWriteSyncer)
	addBufferWriteSyncer(bufferedWriteSyncer)
	if cfg.EnableConsole {
		syncers = append(syncers, console)
	}
	core := zapcore.NewCore(
		textEncoder,
		zapcore.NewMultiWriteSyncer(syncers...),
		zap.LevelEnablerFunc(func(lvl zapcore.Level) bool {
			if level == zapcore.FatalLevel {
				// fatal 文件同时承接 DPanic/Panic/Fatal 三个级别, 避免被静默丢弃
				return lvl >= zapcore.DPanicLevel
			}
			return lvl == level
		}),
	)
	return core, nil
}

// compressOldLogs 压缩被滚动替换的旧日志文件
func compressOldLogs(previousFile string) {
	const logExt = ".log"
	const logExtLength = len(logExt)
	if filepath.Ext(previousFile) != logExt {
		return
	}
	src, err := os.Open(previousFile)
	if err != nil {
		return
	}
	defer base.CloseQuietly(src)

	// 压缩文件: 原文件 → 原文件.gz
	gzPath := previousFile[:len(previousFile)-logExtLength] + ".gz"
	dst, err := os.Create(gzPath)
	if err != nil {
		return
	}
	defer base.CloseQuietly(dst)

	fileStat, err := src.Stat()
	if err != nil {
		return
	}
	gzWriter := gzip.NewWriter(dst)
	gzWriter.Name = fileStat.Name()
	gzWriter.ModTime = fileStat.ModTime()
	if _, err = io.Copy(gzWriter, src); err != nil {
		// 压缩失败, 清理不完整的 .gz 文件
		_ = gzWriter.Close()
		_ = dst.Close()
		_ = os.Remove(gzPath)
		return
	}
	if err = gzWriter.Close(); err != nil {
		// 压缩数据不完整, 清理
		_ = dst.Close()
		_ = os.Remove(gzPath)
		return
	}
	// Windows 下文件未关闭无法删除, 需先关闭源文件
	_ = src.Close()
	if err = os.Remove(previousFile); err != nil {
		_, _ = fmt.Fprintln(os.Stderr, "remove old log file failed:", err)
	}
}

// NewTextLoggerWithCompression 初始化支持压缩的纯文本日志配置
func NewTextLoggerWithCompression(cfg Config) (*zap.Logger, error) {
	var cores []zapcore.Core
	// debug日志
	if cfg.Level <= zapcore.DebugLevel {
		debugLogger, err := getLogger(cfg, zap.DebugLevel)
		if err != nil {
			return nil, err
		}
		cores = append(cores, debugLogger)
	}
	// info日志
	if cfg.Level <= zapcore.InfoLevel {
		infoLogger, err := getLogger(cfg, zap.InfoLevel)
		if err != nil {
			return nil, err
		}
		cores = append(cores, infoLogger)
	}
	// error日志
	if cfg.Level <= zapcore.ErrorLevel {
		errorLogger, err := getLogger(cfg, zap.ErrorLevel)
		if err != nil {
			return nil, err
		}
		cores = append(cores, errorLogger)
	}
	// warn日志
	if cfg.Level <= zapcore.WarnLevel {
		warnLogger, err := getLogger(cfg, zap.WarnLevel)
		if err != nil {
			return nil, err
		}
		cores = append(cores, warnLogger)
	}
	// fatal日志
	if cfg.Level <= zapcore.FatalLevel {
		fatalLogger, err := getLogger(cfg, zap.FatalLevel)
		if err != nil {
			return nil, err
		}
		cores = append(cores, fatalLogger)
	}
	// 合并不同级别的 Core
	core := zapcore.NewTee(cores...)
	// AddCallerSkip(1): 一层跳过 SugaredLogger, 一层跳过 log 包封装函数
	return zap.New(core, zap.AddCaller(), zap.AddCallerSkip(1)), nil
}
