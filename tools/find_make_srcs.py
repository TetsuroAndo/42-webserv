#!/usr/bin/env python3

import os

def generate_makefile(directory, extensions):
    src_files = []
    
    # 再帰的にディレクトリ内のファイルを探索
    for root, _, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                # ファイルパスをリストに追加
                src_files.append(os.path.relpath(os.path.join(root, file), directory))

    print("SRC := \\")
    for i, src in enumerate(src_files):
        if i == len(src_files) - 1:
            print(f"    {src}")  # 最後の行にはバックスラッシュを付けない
        else:
            print(f"    {src} \\")

if __name__ == "__main__":
    # 使用例: カレントディレクトリ内の .cpp と .hpp ファイルを対象にする
    directory = input("ディレクトリを指定してください: ")
    extensions = input("拡張子をカンマ区切りで指定してください (例: cpp,hpp): ").split(",")
    extensions = [f".{ext.lstrip('.')}" for ext in extensions]
    generate_makefile(directory, extensions)
