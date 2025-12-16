package std

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

// Export 从嵌入式文件系统导出文件到目标路径
//
// 参数:
//
//	resources - 嵌入式文件系统资源
//	source - 源文件路径, 相对于资源根目录, 如 "resources/file.txt"
//	target - 目标文件路径
//
// 返回值:
//
//	error - 操作过程中发生的错误
//
// 功能:
//  1. 从嵌入式文件系统打开源文件
//  2. 创建目标文件
//  3. 复制文件内容
//  4. 保持原始文件的修改时间，若不可用则使用当前时间
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
