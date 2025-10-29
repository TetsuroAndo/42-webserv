# Webserv/42

## Overview

## 概要 (Summary)

**Webserv**は、42 Webserv（version: **23.1**）の一環として、C++98スタンダードのみで実装されたHTTP/1.0サーバーです。Boostを含む外部ライブラリの使用はしておりません。

このサーバーの最大の特徴は、`epoll` (または `kqueue`, `poll`) を用いた**I/O多重化**によるノンブロッキングなリクエスト処理です。これにより、単一のスレッド（またはプロセス）で多数のクライアント接続を効率的に処理できます。

Nginxの設定ファイルにインスパイアされた 独自のコンフィグファイル（この実装ではYAML）を読み込むことで、複数のポートでのリッスンや、詳細なルーティング設定が可能です。

### 課題の主な必須要件 (Mandatory Requirements)

* **C++98** のみで実装。
* `GET`, `POST`, `DELETE` メソッドの実装。
* ノンブロッキングI/O (`poll`, `select`, `epoll`, `kqueue` のいずれかを使用)。
* 静的サイトの配信。
* CGI（PHP, Pythonなど）の実行。
* ファイルのアップロードと削除。
* 詳細な設定ファイル（ポート、ルート、リダイレクト、メソッド制限、CGI設定など）。

### このWebservでの実装機能

* **I/O多重化**: Linuxでは `epoll`を使用する `SocketsManager` を実装。
* **設定ファイル**: YAMLパーサー (`src/Lib/MyYAML`) を自作し、`config/*.yaml` ファイルを読み込みます。
* **アーキテクチャ**: リクエスト処理に**ミドルウェアパターン**を採用 (`src/Middleware`)。リクエスト解析、ルーティング、セッション管理、ハンドラー呼び出しをパイプライン化しています。
* **HTTPパーサー**: リクエストライン、ヘッダー、ボディ（`Content-Length` 及び `chunked` 転送エンコーディング）に対応したパーサーを実装 (`src/Http/Parser`)。
* **CGIハンドリング**: `php-cgi` や `python3` とノンブロッキングなパイプ通信 (`pipe`) を行い、動的コンテンツを生成します (`src/Cgi`)。
* **ロギング**: 高機能なロガー (`src/Lib/Logger`) を実装。
    * アクセスログ / エラーログ
    * 出力先: ファイル または コンソール
    * フォーマット: JSON または W3C-ELF (Nginxライクな形式)
    * ファイルサイズに基づくログローテーション
* **ボーナス機能**: クッキーベースの**セッション管理** (`src/Session`) を実装。


## Required

- **Compiler**: C++98準拠のコンパイラ（g++, clang++）
- **Support OS**: Ubuntu22.04
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
├── logs/           # ログファイルの保存先
├── src/            # ソースファイル
│   ├── main.cpp    # エントリーポイント
│   ├── Cgi/        # CGI処理
│   ├── Config/     # 設定ファイル処理
│   ├── Handlers/   # リクエストハンドラー
│   ├── Middleware/ # ミドルウェア
│   ├── Http/       # HTTPプロトコル処理
│   ├── Lib/        # 自作ライブラリ
│   ├── Server/     # サーバーコア機能
│   ├── Socket/     # ソケット・ノンブロッキングIO機能
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
- Private variable: underscore prefix (e.g: `_socketFd`)

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
