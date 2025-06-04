#!/bin/bash

# スクリプトの第一引数にチェック対象ディレクトリを指定 (なければカレントディレクトリ)
TARGET_DIR="${1:-.}"

# 深さ優先でディレクトリを探索
find "$TARGET_DIR" -depth -type d | while read -r dir; do
  # 隠しディレクトリ (先頭が . ) や node_modules ディレクトリ配下はスキップ
  #   例) /path/to/.../node_modules や /path/to/.../.git など
  case "$dir" in
    *"/node_modules"*) continue ;;
    # *"/.git"*) continue ;;
  esac
  # パスのどこかに隠しディレクトリ（.で始まるディレクトリ）が含まれている場合はスキップ
  if [[ "$dir" =~ /\.[^/]+/ ]]; then
    continue
  fi
  if [[ "$(basename "$dir")" =~ ^\..* ]]; then
    continue
  fi

  # このディレクトリ直下にあるファイル/ディレクトリ数をカウント
  # -mindepth 1 -maxdepth 1：直下のみ
  # \( -type f -o -type d \)：ファイルまたはディレクトリ
  # -not -name '.gitkeep'：.gitkeepは除外
  content_count=$(
    find "$dir" -mindepth 1 -maxdepth 1 \
      \( -type f -o -type d \) -not -name '.gitkeep' \
      2>/dev/null | wc -l
  )

  # content_count が 0 の場合は “空” とみなし、.gitkeep を作成
  if [ "$content_count" -eq 0 ]; then
    if [ ! -f "$dir/.gitkeep" ]; then
      touch "$dir/.gitkeep"
      echo "[ADD]   $dir/.gitkeep"
    fi
  else
    # それ以外は “空ではない” とみなし、.gitkeep があれば削除
    if [ -f "$dir/.gitkeep" ]; then
      rm "$dir/.gitkeep"
      echo "[REMOVE] $dir/.gitkeep"
    fi
  fi
done