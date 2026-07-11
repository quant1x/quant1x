package cache

import (
	"embed"
	"io"
	"io/fs"
	"os"
	"time"
)

// OpenEmbed 打开嵌入式文件
func OpenEmbed(resources embed.FS, filename string) (fs.File, error) {
	reader, err := resources.Open(filename)
	if err != nil {
		return nil, err
	}
	return reader, nil
}

// Export 导出内嵌资源文件
func Export(resources embed.FS, source, target string) error {
	src, err := OpenEmbed(resources, source)
	if err != nil {
		return err
	}
	output, err := os.Create(target)
	if err != nil {
		return err
	}
	_, err = io.Copy(output, src)
	if err != nil {
		return err
	}
	var mtime time.Time
	fileinfo, err := src.Stat()
	if err != nil || fileinfo.ModTime().IsZero() {
		mtime = time.Now()
	} else {
		mtime = fileinfo.ModTime()
	}
	err = os.Chtimes(target, mtime, mtime)
	return err
}
