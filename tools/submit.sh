#!/usr/bin/env bash
# ------------------------------------------------------------------------------
# Webserv Submit for 42 Script
# Usage : ./submit.sh <REPO_URL>
# Example: ./submit.sh git@vogsphere-v2.42tokyo.jp:intra/login/xxx.git
# ------------------------------------------------------------------------------

set -euo pipefail

# 定数
SUBMIT_FILES=("src/" "config/" "Makefile" ".gitignore")
REPO_NAME="submit-for-42"
SUBMIT_BRANCH="master"

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

# テスト実行
echo "Running tests..."
make test || { echo "Error: Tests failed." >&2; exit 1; }

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

# コミット & プッシュ
git add "${SUBMIT_FILES[@]}"
if ! git diff --cached --quiet; then
  git commit -m "Submit for 42-review"
fi

git push -u "$REPO_NAME" "$SUBMIT_BRANCH" || \
  { echo "Info: Initial push failed, attempting force-with-lease..."; \
    git push -u "$REPO_NAME" "$SUBMIT_BRANCH" --force-with-lease; }

echo "✔ Successfully pushed to ${REPO_URL}"
