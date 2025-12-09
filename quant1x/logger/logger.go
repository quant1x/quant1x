package logger

import (
	"compress/gzip"
	"io"
	"os"
	"path/filepath"
	"time"

	"gitee.com/quant1x/quant1x/quant1x/std"
	rotatelogs "github.com/lestrrat-go/file-rotatelogs"
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
		panic("invalid log level")
	}
	// 配置日志滚动器，按天切割
	path := filepath.Join(cfg.Path, filename+"_%Y%m%d.log")
	rl, err := rotatelogs.New(
		path,                              // 文件名格式，带日期
		rotatelogs.WithMaxAge(cfg.MaxAge), // 保留7天的日志
		rotatelogs.WithRotationTime(cfg.RotationTime), // 每24小时切割一次
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
	// 带缓冲的 WriteSyncer（缓冲区大小 256KB）
	bufferedWriteSyncer := &zapcore.BufferedWriteSyncer{
		WS:            writeSyncer,
		Size:          cfg.BufferSize * 1024,           // 缓冲区大小
		FlushInterval: cfg.FlushInterval * time.Second, // 定时刷新间隔
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
			//ret := false
			//switch lvl {
			//case zapcore.DebugLevel, zapcore.InfoLevel, zapcore.WarnLevel, zapcore.ErrorLevel:
			//	ret = lvl == level
			//default:
			//	ret = lvl <= zapcore.FatalLevel
			//}
			//return ret
			return lvl == level
		}),
	)
	return core, nil
}

// 压缩旧日志文件的钩子函数
func compressOldLogs(previousFile string) {
	const logExt = ".log"
	const logExtLength = len(logExt)
	if filepath.Ext(previousFile) == logExt {
		src, err := os.Open(previousFile)
		if err != nil {
			return
		}
		// 压缩文件：原文件 → 原文件.gz
		gzPath := previousFile[:len(previousFile)-logExtLength] + ".gz"
		dst, _ := os.Create(gzPath)
		defer std.CloseQuietly(dst)

		gzWriter := gzip.NewWriter(dst)
		defer std.CloseQuietly(gzWriter)
		fileStat, err := src.Stat()
		if err != nil {
			return
		}

		gzWriter.Name = fileStat.Name()
		gzWriter.ModTime = fileStat.ModTime()
		_, err = io.Copy(gzWriter, src) // 压缩内容
		if err != nil {
			return
		}
		_ = src.Close()
		_ = os.Remove(previousFile) // 删除原文件
	}
}

// NewTextLoggerWithCompression 初始化支持压缩的纯文本日志配置
func NewTextLoggerWithCompression(cfg Config) *zap.Logger {
	// --------------------------------------------
	// 2. 按级别配置日志文件（启用压缩）
	// --------------------------------------------
	// 配置日志滚动器，按天切割
	var cores []zapcore.Core
	// debug日志
	if cfg.Level <= zapcore.DebugLevel {
		debugLogger, err := getLogger(cfg, zap.DebugLevel)
		if err != nil {
			panic(err)
		}
		cores = append(cores, debugLogger)
	}
	// info日志
	if cfg.Level <= zapcore.InfoLevel {
		infoLogger, err := getLogger(cfg, zap.InfoLevel)
		if err != nil {
			panic(err)
		}
		cores = append(cores, infoLogger)
	}
	// error日志
	if cfg.Level <= zapcore.ErrorLevel {
		errorLogger, err := getLogger(cfg, zap.ErrorLevel)
		if err != nil {
			panic(err)
		}
		cores = append(cores, errorLogger)
	}
	// warn日志
	if cfg.Level <= zapcore.WarnLevel {
		warnLogger, err := getLogger(cfg, zap.WarnLevel)
		if err != nil {
			panic(err)
		}
		cores = append(cores, warnLogger)
	}
	// fatal日志
	if cfg.Level <= zapcore.FatalLevel {
		fatalLogger, err := getLogger(cfg, zap.FatalLevel)
		if err != nil {
			panic(err)
		}
		cores = append(cores, fatalLogger)
	}
	// --------------------------------------------
	// 3. 创建不同级别的 Core 并合并
	// --------------------------------------------

	core := zapcore.NewTee(cores...)

	// --------------------------------------------
	// 4. 构建 Logger
	// --------------------------------------------
	return zap.New(core, zap.AddCaller(), zap.AddCallerSkip(1))
}
