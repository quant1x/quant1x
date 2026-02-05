#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.

"""
二进制文件转C/C++头文件转换器

将二进制文件转换为包含嵌入式资源数据的C/C++头文件。
针对可读性和可维护性进行了优化。

用法:
    python b2c.py <输入文件> <输出文件> [选项]

选项:
    --namespace NS    包装在C++命名空间中
    --bytes-per-line N  数组每行字节数 (默认: 16)
    --no-timestamp    不包含时间戳
    --no-filename     不包含文件名字符串
"""

import argparse
import os
import sys
from pathlib import Path
from typing import BinaryIO, TextIO, Optional


class BinaryToCConverter:
    """将二进制文件转换为C/C++头文件的转换器。"""

    def __init__(self, bytes_per_line: int = 16):
        self.bytes_per_line = bytes_per_line

    def generate_include_guard(self, filepath: str) -> str:
        """从文件路径生成唯一的包含保护符。"""
        # 将路径转换为有效的C标识符
        guard = str(filepath)
        guard = guard.replace(os.sep, '_').replace('.', '_').replace('-', '_')
        # 在Windows上移除驱动器盘符
        if os.name == 'nt' and ':' in guard:
            guard = guard.split(':', 1)[1]
        return f"{guard.upper()}_INC"

    def generate_variable_name(self, filename: str) -> str:
        """从文件名生成有效的C变量名。"""
        name = Path(filename).stem
        # 将无效字符替换为下划线
        name = ''.join(c if c.isalnum() or c == '_' else '_' for c in name)
        # 确保不以数字开头
        if name and name[0].isdigit():
            name = f"_{name}"
        return name

    def format_byte_array(self, data: bytes) -> str:
        """将二进制数据格式化为带适当换行符的C数组。"""
        if not data:
            return "    // 空文件"

        lines = []
        for i in range(0, len(data), self.bytes_per_line):
            chunk = data[i:i + self.bytes_per_line]
            hex_values = [f'0x{b:02x}' for b in chunk]
            line = '    ' + ', '.join(hex_values)
            lines.append(line)

        return ',\n'.join(lines)

    def convert(self, input_path: str, output_path: str,
                namespace: Optional[str] = None, include_timestamp: bool = True,
                include_filename: bool = True) -> None:
        """将二进制文件转换为C/C++头文件。"""

        input_file = Path(input_path)
        output_file = Path(output_path)

        # 验证输入
        if not input_file.exists():
            raise FileNotFoundError(f"输入文件未找到: {input_file}")

        if input_file == output_file:
            raise ValueError("输入和输出文件不能相同")

        # 读取二进制数据
        try:
            with open(input_file, 'rb') as f:
                data = f.read()
        except IOError as e:
            raise IOError(f"读取输入文件失败: {e}")

        # 获取文件元数据
        stat = input_file.stat()
        mtime_ms = int(stat.st_mtime * 1000) if include_timestamp else None

        # 生成标识符
        var_name = self.generate_variable_name(input_file.name)
        include_guard = self.generate_include_guard(str(input_file))

        # 写入头文件
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                self._write_header(f, include_guard, namespace)
                self._write_filename(f, var_name, input_file.name, include_filename)
                self._write_data_array(f, var_name, data)
                self._write_metadata(f, var_name, data, mtime_ms, include_timestamp)
                self._write_footer(f, include_guard, namespace)

        except IOError as e:
            raise IOError(f"写入输出文件失败: {e}")

        print(f"✓ 生成C资源文件: {output_path}")
        print(f"  变量名: {var_name}")
        print(f"  数据大小: {len(data)} 字节")
        if include_timestamp:
            print(f"  时间戳: {mtime_ms}")

    def _write_header(self, f: TextIO, include_guard: str, namespace: Optional[str] = None) -> None:
        """写入文件头，包含包含保护符和命名空间。"""
        f.write("#pragma once\n")
        f.write(f"#ifndef {include_guard}\n")
        f.write(f"#define {include_guard} 1\n\n")

        if namespace:
            f.write(f"namespace {namespace} {{\n\n")

    def _write_filename(self, f: TextIO, var_name: str, filename: str, include_filename: bool) -> None:
        """如果请求，写入文件名常量。"""
        if include_filename:
            f.write(f'const char* const {var_name}_filename = "{filename}";\n\n')

    def _write_data_array(self, f: TextIO, var_name: str, data: bytes) -> None:
        """将二进制数据写入为C数组。"""
        f.write(f"const unsigned char {var_name}_data[] = {{\n")
        f.write(self.format_byte_array(data))
        f.write("\n};\n\n")

    def _write_metadata(self, f: TextIO, var_name: str, data: bytes,
                       mtime_ms: Optional[int] = None, include_timestamp: bool = True) -> None:
        """写入元数据常量。"""
        f.write(f"const unsigned int {var_name}_length = {len(data)};\n")
        if include_timestamp and mtime_ms is not None:
            f.write(f"const long long {var_name}_timestamp = {mtime_ms}LL;\n")

    def _write_footer(self, f: TextIO, include_guard: str, namespace: Optional[str] = None) -> None:
        """写入文件尾，包含命名空间结束和包含保护符。"""
        if namespace:
            f.write(f"\n}} // namespace {namespace}\n")
        f.write(f"\n#endif // {include_guard}\n")


def main():
    """主入口点。"""
    parser = argparse.ArgumentParser(
        description="将二进制文件转换为C/C++头文件",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )

    parser.add_argument('input_file', help='输入二进制文件')
    parser.add_argument('output_file', help='输出C/C++头文件')
    parser.add_argument('--namespace', '-n', help='包装在C++命名空间中')
    parser.add_argument('--bytes-per-line', '-b', type=int, default=16,
                       help='数组每行字节数 (默认: 16)')
    parser.add_argument('--no-timestamp', action='store_true',
                       help="不包含时间戳")
    parser.add_argument('--no-filename', action='store_true',
                       help="不包含文件名字符串")

    args = parser.parse_args()

    try:
        converter = BinaryToCConverter(bytes_per_line=args.bytes_per_line)
        converter.convert(
            input_path=args.input_file,
            output_path=args.output_file,
            namespace=args.namespace,
            include_timestamp=not args.no_timestamp,
            include_filename=not args.no_filename
        )
    except Exception as e:
        print(f"错误: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()