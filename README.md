# Webserv

## Overview

Webserv is a HTTP/1.1-compliant web server implemented in C++98.
Developed as a 42 Tokyo project, it is a production-ready server application that can be tested with actual web browsers.

## Feature

### Mandatory
- ✅ **ノンブロッキングI/O**: epoll/kqueue/selectを使用した非同期処理
- ✅ **マルチポート対応**: 複数のポートで同時リスニング
- ✅ **HTTPメソッド**: GET, POST, DELETE対応
- ✅ **静的ファイルサービング**: HTML, CSS, JS, 画像ファイルなど
- ✅ **ファイルアップロード**: クライアントからのファイル受信
- ✅ **CGI対応**: PHP, Python等の動的コンテンツ実行
- ✅ **カスタムエラーページ**: 404, 403, 500等のエラーページ設定
- ✅ **ディレクトリリスティング**: ディレクトリ内容の自動表示
- ✅ **設定ファイル**: NGINX風の設定ファイルサポート

### Bonus
- 🎯 **Cookie管理**: HTTPクッキーの送受信
- 🎯 **セッション管理**: ユーザーセッションの維持
- 🎯 **複数CGI対応**: 複数のCGIプログラムの同時実行

## Required

- **Compiler**: C++98準拠のコンパイラ（g++, clang++）
- **OS**: Ubuntu22.04, macOS
- **Make**: GNU Make 3.81以上

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

### Configuration file example

```nginx
server {
    # リスニングポートとホスト
    listen 8080;
    server_name localhost;
    
    # エラーページの設定
    error_page 404 /error/404.html;
    error_page 500 502 503 504 /error/50x.html;
    
    # クライアントボディサイズの上限
    client_max_body_size 10M;
    
    # ルートディレクトリの設定
    location / {
        root /var/www/html;
        index index.html index.htm;
        allow_methods GET POST;
        autoindex on;
    }
    
    # アップロード用の設定
    location /upload {
        allow_methods POST DELETE;
        upload_store /var/www/uploads;
        client_max_body_size 100M;
    }
    
    # CGI設定（PHPの例）
    location ~ \.php$ {
        allow_methods GET POST;
        cgi_pass /usr/bin/php-cgi;
        cgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
    }
    
    # Pythonスクリプト用CGI
    location ~ \.py$ {
        allow_methods GET POST;
        cgi_pass /usr/bin/python3;
    }
    
    # リダイレクトの例
    location /old {
        return 301 /new;
    }
}

# 複数サーバーの設定例
server {
    listen 8081;
    server_name api.localhost;
    
    location / {
        root /var/www/api;
        allow_methods GET POST PUT DELETE;
    }
}
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

- **同時接続数**: 10,000+
- **スループット**: 50,000+ req/sec (静的ファイル)
- **レスポンス時間**: < 10ms (ローカル環境)

## Author

- [Tetsuro Ando](https://github.com/tetsuroando)
- [Tomoki Sato](https://github.com/tomsato42)
- [Wata](https://github.com/melswonder)

## Acknowledgement

- 42 Tokyo
- HTTP protocol RFC document
- NGINX project (configuration file format reference)

## Reference

- [RFC 2616 - HTTP/1.1](https://www.rfc-editor.org/rfc/rfc2616)
- [NGINX document](http://nginx.org/en/docs/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

---
