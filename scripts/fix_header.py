# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.
"""
一次性修复所有文件的头部

这个脚本用于为项目中的所有Python文件添加统一的许可证头部. 
只在项目初始化时使用一次, 不要重复运行. 
"""

import os
import glob
import sys

HEADER = '''# -*- coding: utf-8 -*-
# Copyright (c) Quant1X <wangfengxy@sina.cn>.
# Licensed under the MIT License.'''

def is_empty_file(filepath):
    """检查文件是否为空或只包含空白字符"""
    try:
        # 检查文件大小
        if os.path.getsize(filepath) == 0:
            return True
        
        # 检查文件内容是否只有空白字符
        with open(filepath, 'r', encoding='utf-8', newline='') as f:
            content = f.read().strip()
            if not content:
                return True
        
        return False
    except Exception as e:
        print(f"❌ 检查文件失败 {filepath}: {e}")
        return False

def should_add_header(filepath):
    """检查文件是否需要添加头部"""
    # 首先检查是否为空文件
    if is_empty_file(filepath):
        print(f"⏭️  跳过空文件: {filepath}")
        return False
    
    try:
        with open(filepath, 'r', encoding='utf-8', newline='') as f:
            content_start = f.read(1000)  # 读取文件开头部分
        
        lines = content_start.split('\n')
        
        # 检查前10行是否包含许可证信息
        for i, line in enumerate(lines[:10]):
            if 'Copyright (c) Quant1X' in line and 'MIT License' in content_start:
                return False  # 已经包含正确的头部
        
        # 检查是否有编码声明和许可证的任意组合
        has_coding = any('coding: utf-8' in line.lower() for line in lines[:5])
        has_copyright = any('Copyright' in line for line in lines[:10])
        
        # 如果已经有编码声明但没有版权信息, 需要添加
        if has_coding and not has_copyright:
            return True
        
        # 如果文件基本为空或没有任何头部信息
        if len(lines) <= 3 and all(not line.strip() or line.strip().startswith('#') for line in lines):
            return True
            
        # 默认给新文件添加头部
        return True
        
    except Exception as e:
        print(f"❌ 读取文件失败 {filepath}: {e}")
        return False

def add_header_smart(filepath):
    """智能添加头部到文件"""
    try:
        # 再次检查是否为空文件(防止在检查后文件被修改)
        if is_empty_file(filepath):
            print(f"⏭️  跳过空文件: {filepath}")
            return False
            
        with open(filepath, 'r', encoding='utf-8', newline='') as f:
            original_content = f.read()
        
        lines = original_content.split('\n')
        new_lines = []
        
        # 处理 shebang
        if lines and lines[0].startswith('#!'):
            new_lines.append(lines[0])  # 保留 shebang
            lines = lines[1:]
        
        # 处理编码声明
        encoding_added = False
        if lines and 'coding: utf-8' in lines[0].lower():
            new_lines.append(lines[0])  # 保留已有的编码声明
            lines = lines[1:]
            encoding_added = True
        
        # 添加许可证头部
        if not encoding_added:
            new_lines.append('# -*- coding: utf-8 -*-')
        
        new_lines.append('# Copyright (c) Quant1X <wangfengxy@sina.cn>.')
        new_lines.append('# Licensed under the MIT License.')
        
        # 添加空行分隔头部和内容
        if lines and lines[0].strip():  # 如果第一行不是空行
            new_lines.append('')
        
        # 添加剩余内容
        new_lines.extend(lines)
        
        new_content = '\n'.join(new_lines)
        
        with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
            f.write(new_content)
        
        return True
        
    except Exception as e:
        print(f"❌ 写入文件失败 {filepath}: {e}")
        return False

def get_python_files(include_tests=True):
    """获取所有Python文件"""
    patterns = ['**/*.py']
    if not include_tests:
        patterns = [p for p in patterns if not p.startswith('test')]
    
    all_files = []
    for pattern in patterns:
        all_files.extend(glob.glob(pattern, recursive=True))
    
    # 过滤掉不需要的文件
    exclude_dirs = {'.git', '__pycache__', '.pytest_cache', 'venv', 'env', 'node_modules'}
    filtered_files = []
    
    for filepath in all_files:
        if any(exclude in filepath.split(os.sep) for exclude in exclude_dirs):
            continue
        if os.path.isfile(filepath):
            filtered_files.append(filepath)
    
    return sorted(set(filtered_files))

def main():
    """主函数"""
    print("🔍 扫描Python文件...")
    
    # 获取文件列表
    if len(sys.argv) > 1:
        files = []
        for arg in sys.argv[1:]:
            if os.path.isfile(arg) and arg.endswith('.py'):
                files.append(arg)
            elif os.path.isdir(arg):
                files.extend(glob.glob(os.path.join(arg, '**/*.py'), recursive=True))
    else:
        files = get_python_files()
    
    if not files:
        print("❌ 没有找到Python文件")
        return 1
    
    print(f"📁 找到 {len(files)} 个Python文件")
    
    # 检查文件, 跳过空文件
    need_header_files = []
    empty_files = []
    
    for filepath in files:
        if is_empty_file(filepath):
            empty_files.append(filepath)
        elif should_add_header(filepath):
            need_header_files.append(filepath)
    
    # 显示统计信息
    if empty_files:
        print(f"⏭️  跳过 {len(empty_files)} 个空文件")
    
    if not need_header_files:
        print("✅ 所有非空文件都已包含正确的许可证头部")
        return 0
    
    print(f"📝 需要添加头部的文件: {len(need_header_files)}")
    for filepath in need_header_files:
        print(f"  - {filepath}")
    
    # 确认操作
    if len(need_header_files) > 10:
        response = input(f"\n⚠️  将要修改 {len(need_header_files)} 个文件, 确认继续? (y/N): ")
        if response.lower() not in ('y', 'yes'):
            print("操作已取消")
            return 0
    
    # 执行添加
    success_count = 0
    failed_files = []
    
    for filepath in need_header_files:
        if add_header_smart(filepath):
            print(f"✅ 已修复: {filepath}")
            success_count += 1
        else:
            print(f"❌ 修复失败: {filepath}")
            failed_files.append(filepath)
    
    print(f"\n📊 修复完成:")
    print(f"  - 总文件数: {len(files)}")
    print(f"  - 跳过空文件: {len(empty_files)}")
    print(f"  - 需要修复: {len(need_header_files)}")
    print(f"  - 成功修复: {success_count}")
    print(f"  - 修复失败: {len(failed_files)}")
    
    if failed_files:
        print(f"\n❌ 失败的文件:")
        for f in failed_files:
            print(f"  - {f}")
    
    if success_count > 0:
        print("\n⚠️  请使用 git diff 检查修改内容！")
        print("   确认无误后执行: git add . && git commit -m '添加许可证头部'")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())