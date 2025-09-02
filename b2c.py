# bin_to_c.py
import sys
import os
import math

def main():
    if len(sys.argv) != 3:
        print("Usage: python bin_to_c.py <input_file> <output_file>")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    # 读取文件二进制数据
    with open(input_file, 'rb') as fin:
        data = fin.read()

    # 获取文件最后修改时间（mtime），转为毫秒级 Unix 时间戳
    mtime = os.path.getmtime(input_file)
    mtime_ms = int(math.floor(mtime * 1000))  # 毫秒级时间戳

    # 构造变量名（替换 . 和 -）
    filename = os.path.basename(input_file)
    var_name = os.path.splitext(filename)[0]
    var_name = var_name.replace('.', '_').replace('-', '_')
    # 构造 include guard 名称
    tmp_guard = input_file.replace('.','_').replace('\\','_').replace('-','_')
    include_guard = f"{tmp_guard.upper()}_INC"

    # 写入输出文件
    with open(output_file, 'w') as fout:
        # 写入资源字节数组
        fout.write(f"#pragma once\n")
        fout.write(f"#ifndef {include_guard}\n")
        fout.write(f"#define {include_guard} 1\n\n")
        fout.write(f"const char* const {var_name}_filename = \"{filename}\";\n")
        fout.write(f"unsigned char {var_name}_data[] = {{\n")
        fout.write(', '.join(f'0x{b:02x}' for b in data))
        fout.write("\n};\n\n")

        # 写入资源长度
        fout.write(f"unsigned int {var_name}_length = {len(data)};\n\n")

        # 写入时间戳
        fout.write(f"long long {var_name}_timestamp = {mtime_ms};\n")
        fout.write(f"\n#endif // {include_guard}\n")

    print(f"Generated C resource file '{output_file}' with timestamp: {mtime_ms}")

if __name__ == '__main__':
    main()