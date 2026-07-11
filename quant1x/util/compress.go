package util

import (
	"archive/zip"
	"bytes"
	"compress/zlib"
	"errors"
	"io"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/quant1x/quant1x/quant1x/base"
)

// ZlibCompress 进行zlib压缩
func ZlibCompress(src []byte) ([]byte, error) {
	var in bytes.Buffer
	w := zlib.NewWriter(&in)
	_, err := w.Write(src)
	if err != nil {
		return nil, err
	}
	err = w.Close()
	if err != nil {
		return nil, err
	}
	return in.Bytes(), nil
}

// ZlibUnCompress 进行zlib解压缩
func ZlibUnCompress(compressSrc []byte) ([]byte, error) {
	b := bytes.NewReader(compressSrc)
	var out bytes.Buffer
	r, err := zlib.NewReader(b)
	if err != nil {
		return nil, err
	}
	defer std.CloseQuietly(r)
	_, err = io.Copy(&out, r)
	if err != nil {
		return nil, err
	}
	return out.Bytes(), nil
}

// UnzipPreserveTimes 解压 src zip 到 dest 目录, 并尽量保留每个文件/目录的修改时间.
// 返回遇到的第一个错误(若有).
func UnzipPreserveTimes(srcZip, dest string, includes ...string) error {
	r, err := zip.OpenReader(srcZip)
	if err != nil {
		return err
	}
	defer r.Close()
	// 规范化目标路径为绝对并清理(不带尾部分隔符), 用于更可靠的 ZipSlip 检查
	destAbs, err := filepath.Abs(dest)
	if err != nil {
		return err
	}
	destAbs = filepath.Clean(destAbs)

	for _, f := range r.File {
		if f.Name == "" {
			continue
		}
		if len(includes) > 0 {
			if !slices.Contains(includes, f.Name) {
				continue
			}
		}

		// 使用 filepath.FromSlash 将 zip 内部的 '/' 路径转换为本平台路径
		targetPath := filepath.Join(destAbs, filepath.FromSlash(strings.TrimPrefix(f.Name, "./")))

		// 计算绝对并清理
		targetAbs, err := filepath.Abs(targetPath)
		if err != nil {
			return err
		}
		targetAbs = filepath.Clean(targetAbs)

		// 更可靠的 ZipSlip 检查: 使用 filepath.Rel 判断是否在 destAbs 内
		rel, err := filepath.Rel(destAbs, targetAbs)
		if err != nil {
			return err
		}
		if rel == ".." || strings.HasPrefix(rel, ".."+string(os.PathSeparator)) {
			return errors.New("illegal file path: " + f.Name)
		}

		info := f.FileInfo()
		modTime := info.ModTime()

		if info.IsDir() {
			if err := os.MkdirAll(targetAbs, info.Mode().Perm()); err != nil {
				return err
			}
			// 最佳努力设置时间戳(部分平台对目录时间设置有限制)
			_ = os.Chtimes(targetAbs, modTime, modTime)
			continue
		}

		// 确保父目录存在
		if err := os.MkdirAll(filepath.Dir(targetAbs), 0o755); err != nil {
			return err
		}

		// 处理符号链接(若 zip 中保留了该位标志)
		if info.Mode()&os.ModeSymlink != 0 {
			rc, err := f.Open()
			if err != nil {
				return err
			}
			linkBytes, err := io.ReadAll(rc)
			rc.Close()
			if err != nil {
				return err
			}
			// 尝试创建符号链接, 否则回退为写入目标内容的普通文件(Windows 上常见)
			_ = os.Remove(targetAbs)
			if err := os.Symlink(string(linkBytes), targetAbs); err != nil {
				if err := os.WriteFile(targetAbs, linkBytes, info.Mode().Perm()); err != nil {
					return err
				}
			} else {
				_ = os.Chtimes(targetAbs, modTime, modTime)
			}
			continue
		}

		// 普通文件: 打开并写入
		rc, err := f.Open()
		if err != nil {
			return err
		}
		out, err := os.OpenFile(targetAbs, os.O_CREATE|os.O_WRONLY|os.O_TRUNC, info.Mode().Perm())
		if err != nil {
			rc.Close()
			return err
		}
		if _, err := io.Copy(out, rc); err != nil {
			rc.Close()
			out.Close()
			return err
		}
		rc.Close()

		// 在文件写入后确保权限(考虑 umask)
		_ = os.Chmod(targetAbs, info.Mode().Perm())
		out.Close()

		// 尝试保留时间戳(atime 和 mtime 都设为 modTime)
		_ = os.Chtimes(targetAbs, modTime, modTime)
	}
	return nil
}
