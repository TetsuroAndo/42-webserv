# Webserv

## Overview

## Required

- **Compiler**: C++98準拠のコンパイラ（g++, clang++）
- **OS**: Ubuntu22.04, macOS
- **Make**: GNU Make 4.3以上

## Install

```bash
# Clone repository
git clone https://github.com/yourusername/webserv.git
cd webserv

# Build
make

# Run
./webserv [設定ファイル]
```

## Usage

### Basic

```bash
# Default configuration
./webserv

# カスタム設定ファイルで起動
./webserv config/my_config.conf
```

## プロジェクト構造

```
webserv/
├── Makefile
├── LICENSE
├── config/         # 設定ファイル
├── docs/           # ドキュメント
├── gci-bin/        # GCIビルドスクリプト
├── inc/            # ヘッダーファイル
├── src/            # ソースファイル
│   ├── main.cpp    # エントリーポイント
│   ├── Cgi/        # CGI処理
│   ├── Config/     # 設定ファイル処理
│   ├── Handlers/   # リクエストハンドラー
│   ├── Http/       # HTTPプロトコル処理
│   ├── Lib/        # ライブラリ
│   ├── Logger/     # ロガー
│   ├── Server/     # サーバーコア機能
│   └── Session/    # セッション管理
├── test/           # テストスイート
├── tools/          # ツール
└── www/            # Webコンテンツ
```

## Test

### Basic test

```bash
# Unit test
make test

# ブラウザでのテスト
# サーバー起動後、以下のURLにアクセス
http://localhost:8080/

# curlでのテスト
curl http://localhost:8080/
curl -X POST -F "file=@test.txt" http://localhost:8080/upload
curl -X DELETE http://localhost:8080/upload/test.txt
```

### Stress test

```bash
# Apache Bench
ab -n 10000 -c 100 http://localhost:8080/

# siege
siege -c 50 -t 30s http://localhost:8080/
```

## Development

### Build command

```bash
make all		    Build all targets
make run		    Run the program
make clean		    Clean object files
make fclean		    Fully clean (clean + remove executable)
make re			    Rebuild (fclean + all)
make debug		    Build with debug flags
make help
```

### Coding standard

- C++98 standard
- Function name: camelCase (e.g: `processRequest`)
- Class name: PascalCase (e.g: `HttpRequest`)
- Private variable: underscore prefix (e.g: `_socket_fd`)

## Performance

## Author

- [Tetsuro Ando](https://github.com/tetsuroando)
- [Tomoki Sato](https://github.com/tomsato42)
- [Tsunami Saito](https://github.com/tsunami2170)

## THX

- [Hiro Watanabe](https://github.com/melswonder)

## Acknowledgement

- 42 Tokyo
- HTTP protocol RFC document
- NGINX project (configuration file format reference)

## Reference

- [RFC 3875 - HTTP/1.1](https://tex2e.github.io/rfc-translater/html/rfc3875.html)
- [NGINX document](http://nginx.org/en/docs/)

---
