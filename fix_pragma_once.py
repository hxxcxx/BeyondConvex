#!/usr/bin/env python3
"""
脚本：修复项目中不以 #pragma once 开头的头文件
"""

import os
import re
from pathlib import Path

def should_skip_file(filepath):
    """检查是否应该跳过该文件"""
    # 跳过第三方库和构建目录
    skip_patterns = [
        'third_party',
        'glfw',
        'imgui',
        'build',
        '.git',
        'out',
        'external'
    ]
    
    filepath_str = str(filepath)
    for pattern in skip_patterns:
        if pattern in filepath_str:
            return True
    return False

def has_pragma_once(content):
    """检查文件是否已经包含 #pragma once"""
    # 移除所有空白字符后检查
    lines = content.split('\n')
    
    # 跳过文件开头的空行和注释
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if not line or line.startswith('//') or line.startswith('/*') or line.startswith('*'):
            i += 1
        else:
            break
    
    # 检查第一个非注释、非空行是否是 #pragma once
    if i < len(lines):
        first_line = lines[i].strip()
        # 允许前面有空格
        first_line = first_line.lstrip()
        if first_line == '#pragma once':
            return True
    
    return False

def fix_header_file(filepath):
    """修复单个头文件"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 检查是否已经有 #pragma once
        if has_pragma_once(content):
            return False, "已有 #pragma once"
        
        # 移除旧的头文件保护（如果有）
        # 匹配 #ifndef XXX_H_ ... #define XXX_H_ ... #endif
        header_guard_pattern = r'^#ifndef\s+(\w+)\s*\n\s*#define\s+\1\s*\n'
        
        # 检查是否有传统的头文件保护
        lines = content.split('\n')
        
        # 找到第一个非空行
        first_code_line = 0
        while first_code_line < len(lines) and not lines[first_code_line].strip():
            first_code_line += 1
        
        if first_code_line < len(lines):
            first_line = lines[first_code_line].strip()
            
            # 如果第一行是 #ifndef，说明有传统的头文件保护
            if first_line.startswith('#ifndef'):
                # 找到对应的 #define 和最后的 #endif
                define_line = first_code_line + 1
                if define_line < len(lines) and '#define' in lines[define_line]:
                    # 移除这两行
                    new_lines = lines[:first_code_line] + lines[define_line+1:]
                    
                    # 移除最后的 #endif
                    # 从后往前找
                    for i in range(len(new_lines) - 1, -1, -1):
                        if '#endif' in new_lines[i] or '#endif' in new_lines[i]:
                            # 检查这行是否只是 #endif 或者注释
                            line_content = new_lines[i].strip()
                            if line_content.startswith('#endif'):
                                new_lines = new_lines[:i] + new_lines[i+1:]
                                break
                    
                    # 在开头添加 #pragma once
                    # 找到第一个非空行
                    insert_pos = 0
                    while insert_pos < len(new_lines) and not new_lines[insert_pos].strip():
                        insert_pos += 1
                    
                    new_lines.insert(insert_pos, '#pragma once')
                    content = '\n'.join(new_lines)
                else:
                    # 直接在开头添加 #pragma once
                    content = '#pragma once\n' + content
            else:
                # 没有头文件保护，直接添加 #pragma once
                content = '#pragma once\n' + content
        
        # 写回文件
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        
        return True, "已添加 #pragma once"
        
    except Exception as e:
        return False, f"错误: {str(e)}"

def main():
    """主函数"""
    # 获取项目根目录
    script_dir = Path(__file__).parent
    project_root = script_dir
    
    print(f"项目根目录: {project_root}")
    print("=" * 60)
    
    # 查找所有 .h 文件
    header_files = list(project_root.rglob('*.h'))
    
    print(f"找到 {len(header_files)} 个头文件")
    print("=" * 60)
    
    fixed_files = []
    skipped_files = []
    error_files = []
    
    for filepath in header_files:
        # 转换为相对路径
        rel_path = filepath.relative_to(project_root)
        
        # 检查是否应该跳过
        if should_skip_file(filepath):
            skipped_files.append((rel_path, "跳过（第三方/构建目录）"))
            continue
        
        # 修复文件
        is_fixed, message = fix_header_file(filepath)
        
        if is_fixed:
            fixed_files.append((rel_path, message))
            print(f"✓ 修复: {rel_path}")
        elif "错误" in message:
            error_files.append((rel_path, message))
            print(f"✗ 错误: {rel_path} - {message}")
        else:
            skipped_files.append((rel_path, message))
    
    print("=" * 60)
    print(f"修复完成！")
    print(f"  修复文件: {len(fixed_files)}")
    print(f"  跳过文件: {len(skipped_files)}")
    print(f"  错误文件: {len(error_files)}")
    
    if fixed_files:
        print("\n修复的文件列表:")
        for filepath, msg in fixed_files:
            print(f"  - {filepath}")
    
    if error_files:
        print("\n错误文件列表:")
        for filepath, msg in error_files:
            print(f"  - {filepath}: {msg}")

if __name__ == '__main__':
    main()
