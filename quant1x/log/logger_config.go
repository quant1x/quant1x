package log

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"github.com/quant1x/quant1x/quant1x/config"
	"github.com/quant1x/quant1x/quant1x/runtime"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

// customTimeEncoder 自定义时间编码器，不带时区
func customTimeEncoder(t time.Time, enc zapcore.PrimitiveArrayEncoder) {
	enc.AppendString(t.Format("2006-01-02T15:04:05.000"))
}

// coreLoggerAdapter 适配器，实现core.Logger接口
type coreLoggerAdapter struct{}

func (c *coreLoggerAdapter) Debugf(template string, args ...any) {
	if logger != nil {
		logger.Debugf(template, args...)
	}
}

func (c *coreLoggerAdapter) Infof(template string, args ...any) {
	if logger != nil {
		logger.Infof(template, args...)
	}
}

func (c *coreLoggerAdapter) Warnf(template string, args ...any) {
	if logger != nil {
		logger.Warnf(template, args...)
	}
}

func (c *coreLoggerAdapter) Errorf(template string, args ...any) {
	if logger != nil {
		logger.Errorf(template, args...)
	}
}

func (c *coreLoggerAdapter) Fatalf(template string, args ...any) {
	if logger != nil {
		logger.Fatalf(template, args...)
	}
}

// Config 日志配置
type Config struct {
	Level         zapcore.Level // 日志级别
	Path          string        // 路径
	EnableConsole bool          // 控制台开关
	MaxAge        time.Duration // 最大保留时间
	RotationTime  time.Duration // 日志切割时间
	BufferSize    int           // 缓冲区大小, 单位KB
	FlushInterval time.Duration // 定时刷新间隔, 单位秒
}

var (
	// --------------------------------------------
	// 1. 定义纯文本编码器
	// --------------------------------------------
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
		FlushInterval: 5,
	}
	logger *zap.SugaredLogger = nil
)
var (
	mu  sync.Mutex
	bws []*zapcore.BufferedWriteSyncer
)

// func init() {
// 	tempPath := os.TempDir()
// 	//cfg.Path = getLogRoot(tempPath)
// 	//zapLogger := NewTextLoggerWithCompression(cfg)
// 	//logger = zapLogger.Sugar()
// 	fmt.Println(tempPath)
// 	InitLogger(tempPath, defaultLevel)
// }

func init() {
	InitLogger(config.GetLogsPath(), defaultLevel)
}

func addBufferWriteSyncer(bw *zapcore.BufferedWriteSyncer) {
	if bw == nil {
		return
	}
	mu.Lock()
	defer mu.Unlock()
	bws = append(bws, bw)
}

// IsDebug 是否debug日志模式
func IsDebug() bool {
	return cfg.Level == zapcore.DebugLevel
}

// InitLogger 初始化全局日志模块
func InitLogger(path string, level LogLevel) {
	path = strings.TrimSpace(path)
	if path == "" {
		path = os.TempDir()
	}
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
	zapLogger := NewTextLoggerWithCompression(cfg)
	logger = zapLogger.Sugar()
	runtime.SetLogger(&coreLoggerAdapter{})
	_ = runtime.RegisterHook("logger", waitForStop)
}

func getLogRoot(path string) string {
	applicationName := getApplicationName()
	return filepath.Join(path, applicationName)
}

// getApplicationName 获取执行文件名
func getApplicationName() string {
	path, _ := os.Executable()
	_, exec := filepath.Split(path)
	arr := strings.Split(exec, ".")
	__applicationName := arr[0]
	return __applicationName
}
