# Webserv テスト (test/)

このディレクトリには、WebservのE2E（エンドツーエンド）テストと堅牢性テストを含む、pytestベースのテストスイートが格納されています。

---

## 🧪 目的

このテストスイートの目的は、42-Webservプロジェクトの必須要件およびボーナス要件を包括的に検証することです。  
サーバーの起動から、HTTPメソッドの処理、ルーティング、CGIの実行、堅牢性まで、機能別に分類されています。

---

## 🚀 実行方法

### 依存ライブラリのインストール:
```bash
cd .. && make pyinit
```

### Webservバイナリのビルド:
（テスト実行前に、プロジェクトルートで`make`が完了している必要があります）

### 全テストの実行:
プロジェクトルートから`pytest`を実行します。
```bash
make test
```
または
```bash
pytest
```

### 特定のテストのみ実行:
特定のディレクトリやファイル、マーカー（`pytest.ini`で定義）を指定して実行できます。

#### 例: CGIテストのみ実行
```bash
pytest test/test_suite/test_30_cgi/
```

#### 例: GETメソッドのテストのみ実行
```bash
pytest test/test_suite/test_10_methods/test_get.py
```

---

## 📂 ディレクトリ構造とテスト概要

テストは機能の単位でサブディレクトリに分割されています。

### `test_00_startup/`
- **サーバーの起動と設定ファイルの検証**
  - `test_invalid_config.py`: 不正な構文や無効なディレクティブを持つ設定ファイルでサーバーが正しく起動に失敗すること（エラー終了）を検証します。
  - `test_valid_startup.py`: 正常な設定ファイルでサーバーが起動すること、引数なしでデフォルト設定を読み込むこと、複数のポートでリッスンできることなどを検証します。

### `test_10_methods/`
- **必須HTTPメソッドの基本動作検証**
  - `test_get.py`: GETメソッドによる静的ファイルの取得（200 OK）、存在しないファイル（404 Not Found）、アクセス権のないファイル（403 Forbidden）を検証します。
  - `test_post.py`: POSTメソッドによるファイルアップロード（201 Created）、リクエストボディが設定（`maxRequestBodySize`）を超過した場合（413 Payload Too Large）などを検証します。
  - `test_delete.py`: DELETEメソッドによるアップロードファイルの削除（204 No Content）、存在しないファイルの削除（404 Not Found）を検証します。

### `test_20_routing/`
- **設定ファイルに基づくルーティングとディレクティブの検証**
  - `test_core_directives.py`: `root`ディレクティブに基づき正しいファイルパスが解決されること、`indexFile`がディレクトリリクエスト時に正しく提供されること、`autoindex`が有効・無効の場合のディレクトリリスティングを検証します。
  - `test_method_limits.py`: `allowedMethods`ディレクティブで許可されていないメソッドがリクエストされた場合に（405 Method Not Allowed）を返すことを検証します。
  - `test_redirects.py`: `redirect`ディレクティブに基づくHTTPリダイレクト（301, 302など）が正しく動作することを検証します。
  - `test_error_pages.py`: 404や500などのエラー発生時に、設定されたカスタムエラーページが提供されること、またはデフォルトのエラーページが提供されることを検証します。

### `test_30_cgi/`
- **CGI（Common Gateway Interface）の包括的検証**
  - `test_cgi_exec.py`: GET（クエリ文字列）およびPOST（標準入力）の両方でCGIスクリプトが正しく実行されることを検証します。スクリプト自体がエラーを返した場合（500 Internal Server Error）の処理も検証します。
  - `test_cgi_env.py`: `QUERY_STRING`, `REQUEST_METHOD`, `PATH_INFO`などのCGI仕様に基づく環境変数が正しく設定されていることを検証します。
  - `test_cgi_data.py`: サーバーがChunkedリクエストを正しくデコード（Un-chunk）してCGIに渡すこと、およびCGIからのEOF（Content-Lengthなし）出力を正しく処理できることを検証します。

### `test_40_robustness/`
- **サーバーの堅牢性、ノンブロッキング動作、耐障害性の検証**
  - `test_nonblocking.py`: サーバーがノンブロッキングで動作していることを検証します（例：時間のかかるCGI処理中に、別のクライアントからの高速なGETリクエストがブロックされないこと）。
  - `test_connections.py`: クライアントが通信途中で接続を切断した場合や、アイドル状態がタイムアウトした場合に、サーバーがクラッシュせず適切にリソースを解放することを検証します。
  - `test_bad_request.py`: 不正なHTTPリクエスト（例：HTTPバージョンがない、ヘッダが壊れている）を受信した場合に、サーバーがクラッシュせず（400 Bad Request）を返すことを検証します。
  - `test_stress.py`: 短時間に多数の同時接続を行う負荷テストを実行し、サーバーが安定して動作し続けることを確認します。

### `test_90_bonus/`
- **ボーナス要件の検証**
  - `test_session.py`: クッキーを使ったセッション管理機能（もし実装した場合）が正しく動作することを検証します。
test_cgi_data.py: サーバーがChunkedリクエストを正しくデコード（Un-chunk）してCGIに渡すこと、およびCGIからのEOF（Content-Lengthなし）出力を正しく処理できることを検証します。
