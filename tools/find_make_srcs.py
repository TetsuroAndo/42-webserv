#!/usr/bin/env python3

import os
import sys
import argparse
from typing import List, Set

# デフォルトで検索から除外する一般的なディレクトリ
DEFAULT_EXCLUDE_DIRS = {'.git', 'node_modules', 'build', 'dist', 'venv', '.venv', '__pycache__'}

def generate_makefile_srcs(directory: str, extensions: List[str], exclude_dirs: Set[str]) -> None:
    """
    ディレクトリを再帰的にスキャンし、指定された拡張子のファイルリストを
    Makefileの 'SRC :=' 形式で標準出力に書き出します。

    Args:
        directory (str): スキャン対象のディレクトリパス。
        extensions (List[str]): 検索対象とする拡張子のリスト（例: ['.cpp', '.hpp']）。
        exclude_dirs (Set[str]): 検索から除外するディレクトリ名のセット。
    """
    src_files = []
    
    if not os.path.isdir(directory):
        print(f"Error: Directory not found or is not a directory: {directory}", file=sys.stderr)
        sys.exit(1)

    for root, dirnames, files in os.walk(directory, topdown=True):
        # 除外ディレクトリリストに基づき、探索対象から除外
        dirnames[:] = [d for d in dirnames if d not in exclude_dirs]
        
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                rel_path = os.path.relpath(os.path.join(root, file), directory)
                src_files.append(os.path.normpath(rel_path))
    
    src_files.sort()

    if not src_files:
        print("SRC := ")
        return

    print("SRC := \\")
    print("\t" + " \\\n\t".join(src_files))


def main():
    """
    コマンドライン引数を処理し、Makefileのソースリスト生成関数を呼び出します。
    """
    parser = argparse.ArgumentParser(
        description="Recursively find source files and print a Makefile-compatible list."
    )
    
    parser.add_argument(
        "extensions",
        help="Comma-separated list of file extensions (e.g., 'cpp,hpp')."
    )
    parser.add_argument(
        "directory",
        nargs="?",  # 0 or 1 (オプション)
        default=".",
        help="The target directory to scan (default: current directory)."
    )
    
    # デフォルトの除外リスト (DEFAULT_EXCLUDE_DIRS) をカンマ区切りでヘルプに表示
    default_excludes_str = ','.join(sorted(DEFAULT_EXCLUDE_DIRS))
    parser.add_argument(
        "-e", "--exclude-dirs",
        help=(
            "Comma-separated list of directories to exclude. "
            f"Overrides the default list. (Default: {default_excludes_str})"
        )
    )
    
    args = parser.parse_args()

    # 拡張子リストのパース (空白除去、'.'の付与)
    extensions = [
        f".{ext.strip().lstrip('.')}"
        for ext in args.extensions.split(",")
        if ext.strip()
    ]

    if not extensions:
        print("Error: No valid extensions provided.", file=sys.stderr)
        sys.exit(1)

    # 除外ディレクトリセットの決定
    if args.exclude_dirs:
        # ユーザー指定があれば、それをパースして使用
        exclude_set = {d.strip() for d in args.exclude_dirs.split(',') if d.strip()} | DEFAULT_EXCLUDE_DIRS
    else:
        # 指定がなければデフォルト値を使用
        exclude_set = DEFAULT_EXCLUDE_DIRS

    generate_makefile_srcs(args.directory, extensions, exclude_set)


if __name__ == "__main__":
    main()
