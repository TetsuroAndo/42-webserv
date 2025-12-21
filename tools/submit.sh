#!/usr/bin/env bash
# ------------------------------------------------------------------------------
# Webserv Submit for 42 Script
# Usage : ./submit.sh <REPO_URL>
# Example: ./submit.sh git@vogsphere-v2.42tokyo.jp:intra/login/xxx.git
# ------------------------------------------------------------------------------

set -euo pipefail

SUBMIT_FILES=("config/default.yaml" "Makefile" ".gitignore")
REPO_NAME="submit-for-42"
SUBMIT_BRANCH="master"
SRC_DIR="src"
SRC_EXCLUDE_TEST_DIRS=(':(exclude)src/**/test' ':(exclude)src/**/test/**')

# 引数チェック
[[ $# -eq 1 ]] || { echo "Usage: $0 <repository_url>" >&2; exit 1; }
REPO_URL="$1"

# スクリプトのディレクトリとプロジェクトルートを取得
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# プロジェクトルートに移動
cd "$PROJECT_ROOT"

# 元のブランチを保存
ORIGINAL_BRANCH=$(git rev-parse --abbrev-ref HEAD)

# トラップでクリーンアップ
cleanup() {
  git checkout "$ORIGINAL_BRANCH" 2>/dev/null || true
  git branch -D "$SUBMIT_BRANCH" 2>/dev/null || true
}
trap cleanup EXIT

# リモート設定
if git remote | grep -qx "$REPO_NAME"; then
  git remote set-url "$REPO_NAME" "$REPO_URL"
else
  git remote add "$REPO_NAME" "$REPO_URL"
fi

# 提出ブランチ作成
git show-ref --verify --quiet "refs/heads/$SUBMIT_BRANCH" && \
  git branch -D "$SUBMIT_BRANCH"
git checkout -b "$SUBMIT_BRANCH"

# すべてのファイルをインデックスから削除
git rm -rf --cached .

# 提出したいファイルだけを強制的にインデックスに追加してコミット
git add "${SUBMIT_FILES[@]}"

# src/ は丸ごと追加せず、src/**/test ディレクトリ配下を除外して追加
# （例: src/Cgi/test, src/Http/.../test など）
if [[ -d "$SRC_DIR" ]]; then
  git add "$SRC_DIR" -- "${SRC_EXCLUDE_TEST_DIRS[@]}"
fi

if ! git diff --cached --quiet; then
  git commit -m "Submit for 42-review"
fi

if git push -u "$REPO_NAME" "$SUBMIT_BRANCH"; then
  :
else
  echo "Warning: The push failed. This may be due to remote changes on the branch."
  echo "If you proceed, a force push (--force-with-lease) may overwrite remote changes."
  read -p "Do you want to force push with --force-with-lease? [y/N]: " confirm
  if [[ "$confirm" =~ ^[Yy]$ ]]; then
    git push -u "$REPO_NAME" "$SUBMIT_BRANCH" --force-with-lease
  else
    echo "Aborted force push."
    exit 1
  fi
fi

echo "✔ Successfully pushed to ${REPO_URL}"
