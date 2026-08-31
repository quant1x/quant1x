package log

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/quant1x/quant1x/quant1x/base"
	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/runtime"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

// customTimeEncoder 自定义时间编码器, 不带时区
func customTimeEncoder(t time.Time, enc zapcore.PrimitiveArrayEncoder) {
	enc.AppendString(t.Format("2006-01-02T15:04:05.000"))
}

// coreLoggerAdapter 适配器, 实现core.Logger接口
type coreLoggerAdapter struct{}

func (c *coreLoggerAdapter) Debugf(template string, args ...any) {
	if l := getSugar(); l != nil {
		l.Debugf(template, args...)
	}
}

func (c *coreLoggerAdapter) Infof(template string, args ...any) {
	if l := getSugar(); l != nil {
		l.Infof(template, args...)
	}
}

func (c *coreLoggerAdapter) Warnf(template string, args ...any) {
	if l := getSugar(); l != nil {
		l.Warnf(template, args...)
	}
}

func (c *coreLoggerAdapter) Errorf(template string, args ...any) {
	if l := getSugar(); l != nil {
		l.Errorf(template, args...)
	}
}

func (c *coreLoggerAdapter) Fatalf(template string, args ...any) {
	if l := getSugar(); l != nil {
		l.Fatalf(template, args...)
	}
}

// Config 日志配置
type Config struct {
	Level         zapcore.Level // 日志级别
	Path          string        // 路径
	EnableConsole bool          // 控制台开关
	MaxAge        time.Duration // 最大保留时间
	RotationTime  time.Duration // 日志切割时间
	BufferSize    int           // 缓冲区大小, 单位 KB
	FlushInterval time.Duration // 定时刷新间隔
}

var (
	// 纯文本编码器
	encoderConfig = zapcore.EncoderConfig{
		TimeKey:        "time",
		LevelKey:       "level",
		NameKey:        "logger",
		CallerKey:      "caller",
		MessageKey:     "msg",
		StacktraceKey:  "stacktrace",
		LineEnding:     zapcore.DefaultLineEnding,
		EncodeLevel:    zapcore.CapitalLevelEncoder,
		EncodeTime:     customTimeEncoder,
		EncodeDuration: zapcore.StringDurationEncoder,
		EncodeCaller:   zapcore.ShortCallerEncoder,
	}
	textEncoder = zapcore.NewConsoleEncoder(encoderConfig)
)

type LogLevel uint8

const (
	DEBUG LogLevel = iota
	INFO
	WARN
	ERROR
	OFF
	FATAL
)

var (
	defaultLevel = DEBUG
	cfg          = Config{
		Level:         zapcore.DebugLevel,
		MaxAge:        7 * 24 * time.Hour,
		RotationTime:  24 * time.Hour,
		BufferSize:    256,
		FlushInterval: 5 * time.Second,
	}
)

var (
	mu     sync.Mutex
	bws    []*zapcore.BufferedWriteSyncer
	logger *zap.SugaredLogger
)

func init() {
	if err := InitLogger(config.GetLogsPath(), defaultLevel); err != nil {
		_, _ = fmt.Fprintf(os.Stderr, "init logger failed: %v\n", err)
	}
}

func addBufferWriteSyncer(bw *zapcore.BufferedWriteSyncer) {
	if bw == nil {
		return
	}
	mu.Lock()
	defer mu.Unlock()
	bws = append(bws, bw)
}

// getSugar 返回全局日志实例(线程安全), 未初始化时返回 nil
func getSugar() *zap.SugaredLogger {
	mu.Lock()
	defer mu.Unlock()
	return logger
}

// IsDebug 是否debug日志模式
func IsDebug() bool {
	mu.Lock()
	defer mu.Unlock()
	return cfg.Level == zapcore.DebugLevel
}

// InitLogger 初始化全局日志模块
func InitLogger(path string, level LogLevel) error {
	// 停止并清理上一次初始化遗留的缓冲写入器, 避免文件句柄泄漏
	mu.Lock()
	old := bws
	bws = nil
	logger = nil
	defaultLevel = level
	cfg.EnableConsole = false
	switch level {
	case DEBUG:
		cfg.Level = zapcore.DebugLevel
		cfg.EnableConsole = true
	case INFO:
		cfg.Level = zapcore.InfoLevel
	case ERROR:
		cfg.Level = zapcore.ErrorLevel
	case WARN:
		cfg.Level = zapcore.WarnLevel
	default:
		cfg.Level = zapcore.FatalLevel
	}
	cfg.Path = getLogRoot(path)
	mu.Unlock()
	for _, bw := range old {
		_ = bw.Stop()
	}

	// 日志目录不存在时创建(与 Python/C++ 实现行为一致)
	if err := base.MkDirs(cfg.Path); err != nil {
		return fmt.Errorf("create log dir %q failed: %w", cfg.Path, err)
	}
	zapLogger, err := NewTextLoggerWithCompression(cfg)
	if err != nil {
		return err
	}
	mu.Lock()
	logger = zapLogger.Sugar()
	mu.Unlock()
	runtime.SetLogger(&coreLoggerAdapter{})
	_ = runtime.RegisterHook("logger", waitForStop)
	return nil
}

func getLogRoot(path string) string {
	applicationName := getApplicationName()
	return filepath.Join(path, applicationName)
}

// getApplicationName 获取执行文件名(去掉扩展名)
func getApplicationName() string {
	path, err := os.Executable()
	if err != nil {
		return "unknown"
	}
	_, exec := filepath.Split(path)
	return strings.TrimSuffix(exec, filepath.Ext(exec))
}
