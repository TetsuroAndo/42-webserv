#!/usr/bin/env bash

# スクリプトがエラーで停止するように設定
set -e

# --- 定義 ---
CGI_TESTER_URL="https://cdn.intra.42.fr/document/document/41002/ubuntu_cgi_tester"
TESTER_URL="https://cdn.intra.42.fr/document/document/41004/ubuntu_tester"
CGI_TESTER_BIN="cgi_test"
TESTER_BIN="ubuntu_tester"
TEST_DIR="YoupiBanane"

echo "Starting test environment setup..."

# --- 1. バイナリのダウンロード ---
echo "Downloading test binaries..."

# cgi_test (指示された名前 'cgi_test' で保存)
if ! wget -O "$CGI_TESTER_BIN" "$CGI_TESTER_URL"; then
    echo "Failed to download cgi_test. Please check the URL or network."
    exit 1
fi

# ubuntu_tester (すでに存在する場合でも、指示に基づき上書きダウンロード)
if ! wget -O "$TESTER_BIN" "$TESTER_URL"; then
    echo "Failed to download ubuntu_tester. Please check the URL or network."
    exit 1
fi

# --- 2. 実行権限の付与 ---
echo "Setting execute permissions..."
chmod +x "$CGI_TESTER_BIN"
chmod +x "$TESTER_BIN"

# --- 3. YoupiBanane ディレクトリ構造の作成 ---
echo "Creating '$TEST_DIR' directory structure..."

# メインディレクトリとサブディレクトリの作成
mkdir -p "$TEST_DIR/nop"
mkdir -p "$TEST_DIR/Yeah"

# --- 4. ダミーファイルの作成 ---
# (テスト指示により、ファイル内容は任意)
echo "Creating dummy files..."

echo "This is youpi.bad_extension" > "$TEST_DIR/youpi.bad_extension"
echo "This is youpi.bla" > "$TEST_DIR/youpi.bla"
echo "This is nop/youpi.bad_extension" > "$TEST_DIR/nop/youpi.bad_extension"
echo "This is nop/other.pouic" > "$TEST_DIR/nop/other.pouic"
echo "This is Yeah/not_happy.bad_extension" > "$TEST_DIR/Yeah/not_happy.bad_extension"

echo ""
echo "✅ Setup complete!"
echo "Created '$TEST_DIR' directory and downloaded '$CGI_TESTER_BIN', '$TESTER_BIN'."
echo "Don't forget to update your configuration file as per the test instructions."