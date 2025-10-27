# Webserv テストチェックリスト

## 1. サーバー起動と設定ファイル

### 1.1. 正常な起動

- [ ] **引数付き起動**:
  `./webserv confs/valid/test.yaml` を実行し、サーバーが正常に起動し、指定ポート（例: 8081）でリッスン状態になることを確認する。

- [ ] **デフォルトパス起動**:
  `./webserv` （引数なし）を実行し、サーバーがデフォルトの設定ファイルパス（例: `config/default.yaml`）を読み込んで正常に起動することを確認する。

- [ ] **複数ポート起動**:
  複数の `listen` ディレクティブ（例: 8081, 8082）を持つ設定ファイルで起動し、すべてのポートでリッスン状態になることを確認する。

- [ ] **複数サーバーブロック**:
  複数の `server` ブロック（それぞれ異なるポート）を持つ設定ファイルで起動し、すべてのサーバーが正しく設定されて起動することを確認する。

### 1.2. 異常系（設定ファイル検証）

サーバーが不正な設定ファイルを検知し、エラーメッセージを出力して**起動しない（終了コードが0以外）**ことを確認します。

- [ ] **ファイル不在**:
  存在しない設定ファイルパスを指定して起動し、エラー終了することを確認する。

- [ ] **権限不足**:
  読み取り権限のない設定ファイルパスを指定して起動し、エラー終了することを確認する。

- [ ] **不正なディレクティブ**:
  - `server` ブロック直下に不正なキー（例: `maxEvets`）がある設定ファイル (`confs/invalid/test_invalid_key_server.yaml`) で起動し、エラー終了する。
  - `location` ブロック内に不正なキー（例: `invalid_key`）がある設定ファイル (`confs/invalid/test_invalid_key_location.yaml`) で起動し、エラー終了する。
  - `listen` ブロック内に不正なキーがある設定ファイル (`confs/invalid/test_invalid_key_listen.yaml`) で起動し、エラー終了する。

- [ ] **不正な値**:
  - `port` に数値以外（例: "abc"）を指定した場合、エラー終了する。
  - `allowedMethods` に必須要件外のメソッド（例: `PUT`）を指定した設定ファイル (`confs/invalid/config_invalid_method.yaml`) で起動し、エラー終了する。

- [ ] **ポート重複**:
  - 同一の設定ファイル内で同じ `interface:port` の組み合わせを複数回 `listen` しようとした場合、エラー終了する。
  - 既に他のプロセスが使用中のポートを `listen` しようとした場合、エラー終了する。

## 2. HTTPリクエスト処理

必須のHTTPメソッド (`GET`, `POST`, `DELETE`) の動作を検証します。

### 2.1. GET メソッド（静的ファイル）

- [ ] **200 OK (ファイル取得)**:
  `GET /static/hello.txt` で 200 OK が返る。


  - レスポンスヘッダに `Content-Type: text/plain` が含まれる。
  - レスポンスヘッダに正しい `Content-Length` が含まれる。
  - レスポンスボディが `hello.txt` の内容と一致する。

- [ ] **200 OK (HTML取得)**:
  `GET /static/index.html` で 200 OK が返る。


  - レスポンスヘッダに `Content-Type: text/html` が含まれる。
  - レスポンスボディが `index.html` の内容と一致する。

- [ ] **404 Not Found (ファイル不在)**:
  `GET /static/non_existent_file.txt` で 404 Not Found が返る。

- [ ] **403 Forbidden (権限エラー)**:
  サーバーが読み取り権限を持たないファイル（例: `chmod 000 secret.txt`）に `GET` リクエストを送信し、403 Forbidden が返る。


- [ ] **301 Moved Permanently (ディレクトリリダイレクト)**:
  `autoindex off` のディレクトリ（例: `/static`）に末尾スラッシュなしで `GET /static` リクエストを送信した場合、サーバーは `/static/` へ 301 リダイレクトする（※NGINXの標準的な挙動）。

### 2.2. POST メソッド（ファイルアップロード）

- [ ] **201 Created (アップロード成功)**:
  `uploadStore` が設定された location（例: `/upload`）に `POST` リクエスト（例: `test_assets/size1.txt`）を送信し、201 Created が返る。


  - サーバーの `uploadStore` で指定されたディレクトリに、アップロードされたファイルが正しく保存されていることを確認する。

- [ ] **413 Request Entity Too Large (サイズ超過)**:
  `maxRequestBodySize 1KB` が設定されたサーバーに対し、2KBのファイルを `POST` し、413 Request Entity Too Large が返る。


  - この時、サーバー側にファイルが作成されていないことを確認する。

- [ ] **405 Method Not Allowed (メソッド不許可)**:
  `POST` が許可されていない location（例: `/static`）に `POST` リクエストを送信し、405 Method Not Allowed が返る。

### 2.3. DELETE メソッド

- [ ] **204 No Content (削除成功)**:
  （準備: ファイルを `/upload` に `POST` しておく）
  アップロードしたファイル（例: `/upload/file_to_delete.txt`）に `DELETE` リクエストを送信し、204 No Content（または 200 OK）が返る。


  - サーバーの `uploadStore` ディレクトリから該当ファイルが削除されていることを確認する。

- [ ] **404 Not Found (ファイル不在)**:
  存在しないファイル（例: `/upload/non_existent.txt`）に `DELETE` リクエストを送信し、404 Not Found が返る。

- [ ] **405 Method Not Allowed (メソッド不許可)**:
  `DELETE` が許可されていない location（例: `/static/hello.txt`）に `DELETE` リクエストを送信し、405 Method Not Allowed が返る。

## 3. ルーティングと設定ディレクティブ

設定ファイルの各ディレクティブが正しく解釈され、サーバーの動作に反映されているかを検証します。

### 3.1. location と root

- [ ] **root の継承**:
  `server` ブロックで `root /var/www;` が設定され、`location /` に root がない場合、`GET /index.html` で `/var/www/index.html` が返される。

- [ ] **root の上書き**:
  `server` ブロックで `root /var/www;`、`location /kapouet { root /tmp/www; }` が設定されている場合、`GET /kapouet/file.txt` で `/tmp/www/file.txt` が返される。


- [ ] **root とURLの結合**:
  `location /kapouet/ { root /tmp/www; }` の場合、`GET /kapouet/pouic.txt` で `/tmp/www/pouic.txt` が返される (PDFの例とは異なる一般的なNGINXの挙動。どちらの仕様を採用しているか確認)。

### 3.2. indexFile（デフォルトファイル）

- [ ] **indexFile の適用**:
  `location / { indexFile test.html; }` が設定されている場合、`GET /` リクエストで `test.html` の内容が返る。

- [ ] **indexFile の優先度**:
  `location /dir/ { indexFile index.html; autoindex on; }` が設定され、`index.html` が存在する場合、`autoindex` ではなく `index.html` の内容が返る。

### 3.3. autoindex（ディレクトリ一覧）

- [ ] **autoindex on**:
  `location /dir/ { autoindex on; }` が設定され、`indexFile` が存在しない場合、`GET /dir/` で 200 OK とディレクトリ一覧（例: "Index of /dir/"）を含むHTMLが返る。


- [ ] **autoindex off**:
  `location /dir/ { autoindex off; }` が設定され、`indexFile` が存在しない場合、`GET /dir/` で 403 Forbidden が返る。

### 3.4. allowedMethods（メソッド制限）

- [ ] **GET のみ許可**:
  `location /get_only { allowedMethods [GET]; }` が設定されている場合:
  - `GET /get_only` -> 200 OK（または 404 など、メソッド以外の理由）
  - `POST /get_only` -> 405 Method Not Allowed
  - `DELETE /get_only` -> 405 Method Not Allowed

- [ ] **未指定（デフォルト）**:
  `location /` で `allowedMethods` が未指定の場合、`GET`, `POST`, `DELETE` の動作が location の他の設定（`root` や `uploadStore`）に基づいて適切に処理される（例: `run.sh` の `config_no_methods.yaml` のテスト）。

### 3.5. redirect（HTTPリダイレクト）

- [ ] **301 恒久リダイレクト**:
  `redirect /old /new 301;` の設定で `GET /old` をリクエストし、301 Moved Permanently が返る。
  - レスポンスヘッダに `Location: /new` が含まれる。

- [ ] **302 一時リダイレクト**:
  `redirect /temp /other 302;` の設定で `GET /temp` をリクエストし、302 Found が返る。
  - レスポンスヘッダに `Location: /other` が含まれる。

- [ ] **外部リダイレクト**:
  `redirect /external http://example.com 302;` の設定で `GET /external` をリクエストし、302 Found が返る。
  - レスポンスヘッダに `Location: http://example.com` が含まれる。

- [ ] **クエリ文字列の保持**:
  `redirect /query /new_query 307;` の設定で `GET /query?a=1&b=2` をリクエストし、307 Temporary Redirect が返る。
  - レスポンスヘッダに `Location: /new_query?a=1&b=2` が含まれる。

### 3.6. error_pages

- [ ] **カスタム 404**:
  `error_page 404 /errors/404.html;` の設定で `GET /non_existent` をリクエストする。
  - レスポンスのステータスコードは 404 Not Found のままである。
  - レスポンスボディが `/errors/404.html` の内容と一致する。

- [ ] **デフォルト 404**:
  `error_page` の設定がない場合、`GET /non_existent` で 404 Not Found が返る。
  - レスポンスボディがサーバーのデフォルトのエラーページ（例: "404 Not Found" というテキストを含むHTML）である。

## 4. CGI (Common Gateway Interface)

CGIスクリプトの実行とデータ連携を検証します。

### 4.1. CGI実行

- [ ] **GET での実行**:
  `confs/valid/cgi.yaml` を使用し、`GET /cgi-bin/simple.py` をリクエストする。
  - 200 OK が返り、レスポンスボディが `simple.py` の標準出力（例: "Hello from CGI!"）と一致する。


- [ ] **POST での実行**:
  `POST /cgi-bin/echo.py` （ボディ: "test data"）をリクエストする。
  - 200 OK が返り、レスポンスボディが "test data" を含む（CGIが標準入力を標準出力にエコーバックする）。

- [ ] **スクリプトエラー**:
  意図的にエラーを発生させる（例: `exit(1)` する）CGIスクリプトにリクエストを送信し、500 Internal Server Error が返る。


- [ ] **実行ディレクトリ**:
  CGIスクリプト内でカレントワーキングディレクトリを参照させ、それが設定ファイルの `root`（例: `test/test_www/cgi-bin`）と一致することを確認する。

### 4.2. CGI 環境変数

環境変数をエコーバックするCGIスクリプトを使用して検証します。

- [ ] **REQUEST_METHOD**:
  `GET` リクエストで `REQUEST_METHOD=GET` がCGIに渡る。
  `POST` リクエストで `REQUEST_METHOD=POST` がCGIに渡る。

- [ ] **QUERY_STRING**:
  `GET /cgi-bin/env.py?a=1&b=2` で `QUERY_STRING=a=1&b=2` がCGIに渡る。

- [ ] **CONTENT_LENGTH / CONTENT_TYPE (POST)**:
  `POST` リクエスト（ボディ: "test"）で `CONTENT_LENGTH=4` と `Content-Type` ヘッダの値がCGIに渡る。

- [ ] **その他 (PATH_INFO, SCRIPT_NAME...)**:
  `PATH_INFO`, `SCRIPT_NAME`, `SERVER_NAME` など、CGI仕様で要求される基本的な変数がCGIに渡されていることを確認する。

### 4.3. CGI データ転送

- [ ] **チャンク解除 (Un-chunking)**:
  `Transfer-Encoding: chunked` を使用して `POST` リクエストをCGIに送信する。
  - CGIスクリプトは、デコード（チャンク解除）された完全なボディを `stdin` から読み取れる


  - CGI側で `Transfer-Encoding` ヘッダを認識する必要がないことを確認する。

- [ ] **CGIからのEOF**:
  CGIスクリプトが `Content-Length` ヘッダを出力せずに終了した場合、サーバーがCGIの `stdout` の EOF を検知し、それをレスポンスボディの終わりとして扱うことを確認する。

## 5. 堅牢性・ノンブロッキング

サーバーがブロッキングせず、高負荷や予期せぬ切断に耐えることを検証します。

### 5.1. I/O多重化（ノンブロッキング）

- [ ] **遅いCGI vs 高速なGET**:
  クライアントAが、10秒間 sleep するCGI（例: `/cgi-bin/slow.py`）にリクエストを送信する。
  クライアントBが、クライアントAの処理中に、静的ファイル（例: `/static/hello.txt`）に `GET` リクエストを送信する。
  期待値: クライアントBは即座に（10秒待たずに）200 OK のレスポンスを受け取る。


- [ ] **遅いアップロード vs 高速なGET**:
  クライアントAが、巨大なファイル（例: 100MB）の `POST` を開始し、1バイトずつゆっくりとボディを送信する。
  クライアントBが、クライアントAのアップロード中に `/static/hello.txt` に `GET` リクエストを送信する。
  期待値: クライアントBは即座にレスポンスを受け取る。


- [ ] **遅いダウンロード vs 高速なGET**:
  クライアントAが、巨大なファイル（例: 100MB）の `GET` を開始し、1バイトずつゆっくりとTCP recv を行う（poll での書き込み監視のテスト）。


  クライアントBが、クライアントAのダウンロード中に `/static/hello.txt` に `GET` リクエストを送信する。
  期待値: クライアントBは即座にレスポンスを受け取る。

### 5.2. クライアント切断とタイムアウト

- [ ] **途中切断（アップロード）**:
  クライアントが巨大なファイルの `POST` を開始し、途中でTCP接続を強制切断（RSTパケット送信）する。
  期待値: サーバーはクラッシュせず、関連するリソース（FD、メモリ）を正常に解放する


- [ ] **途中切断（ダウンロード）**:
  クライアントが巨大なファイルの `GET` を開始し、途中でTCP接続を強制切断する。
  期待値: サーバーはクラッシュせず、関連するリソースを正常に解放する。


- [ ] **アイドルタイムアウト**:
  クライアントがサーバーに `connect()` するが、リクエストを一切送信しない。
  期待値: サーバーが設定したタイムアウト時間（例: `timeoutSec: 5`）経過後に、接続を自動的に切断する。

### 5.3. エラー耐性

- [ ] **不正なリクエスト**:
  HTTPバージョンがない（例: `GET / \r\n\r\n`）リクエストを送信し、サーバーがクラッシュせず 400 Bad Request を返すことを確認する。


  - Host ヘッダがない（HTTP/1.1）リクエストを送信し、400 Bad Request を返すことを確認する。

- [ ] **高負荷（ストレス）**:
  `stress/load.py` などを使い、短時間に大量の（例: 1000回）`GET` リクエストを並行して送信する。
  期待値: サーバーはすべてのリクエストを（エラーなく）処理し続け、クラッシュしない。

## 6. ボーナス要件

必須要件がすべて満たされている場合のみ、ボーナス項目をテストします。

### 6.1. クッキーとセッション

- [ ] **セッションID発行**:
  `confs/valid/session.yaml` を使用し、`GET /`（初回アクセス）をリクエストする。
  - レスポンスヘッダに `Set-Cookie: sessionId=...` が含まれることを確認する（`session_test/run.sh` の挙動）。
  - クッキーに `HttpOnly` や `Path=/` 属性が含まれていることを確認する。

- [ ] **セッションの認識**:
  上記で取得した `sessionId` を `Cookie` ヘッダに含めて `GET /session` にリクエストを送信する。
  - サーバーがセッションを認識し、レスポンスが初回アクセスと異なること（例: `Set-Cookie: lastAccessTime=...` が更新される）を確認する。

### 6.2. 複数CGI

- [ ] **複数CGIの実行**:
  `.py` と `.php` の両方をCGIとして設定する。
  - `GET /cgi-bin/test.py` と `GET /cgi-bin/test.php` の両方が、それぞれ正しく実行され、各スクリプトの実行結果が返ることを確認する。
