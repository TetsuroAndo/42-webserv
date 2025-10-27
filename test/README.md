# WebServ テストスイート

このディレクトリには、WebServプロジェクトの包括的なテストスイートが含まれています。

## 概要

元々の`test.bk`ディレクトリにあったBashベースのテストを、モダンで再利用可能なpytestベースのテストに移行しました。

## ディレクトリ構造

```
test/
├── confs/              # テスト用設定ファイル
│   ├── valid/          # 正常系の設定ファイル
│   └── invalid/        # 異常系の設定ファイル（バリデーションテスト用）
├── test_www/            # テスト用Webリソース
│   ├── static/          # 静的ファイル
│   ├── cgi-bin/         # CGIスクリプト
│   └── uploads/         # アップロードテスト用ディレクトリ
├── test_suite/          # テストコード
│   ├── test_00_validation.py  # 設定ファイルのバリデーションテスト
│   ├── test_01_static_file.py # 静的ファイルサービングのテスト
│   └── test_02_cgi.py         # CGI機能のテスト
├── conftest.py          # pytestの共通フィクスチャ
├── pytest.ini           # pytestの設定
└── requirements.txt     # Python依存関係
```

## セットアップ

### 1. 仮想環境の作成と依存関係のインストール

```bash
# プロジェクトルートから
make pyinit
```

### 2. サーバーのビルド

```bash
# プロジェクトルートから
make
```

## テストの実行

### すべてのテストを実行

```bash
# プロジェクトルートから
make test
```

または、仮想環境をアクティベートしてから：

```bash
cd test
. ../venv/bin/activate
pytest
```

### 特定のテストファイルを実行

```bash
pytest test_suite/test_01_static_file.py
```

### 特定のテスト関数を実行

```bash
pytest test_suite/test_01_static_file.py::TestStaticFile::test_basic_static_get
```

### マーカーでフィルタリング

```bash
# コアテストのみ実行
pytest -m core

# HTTPテストのみ実行
pytest -m http

# CGIテストのみ実行
pytest -m cgi
```

## テストの種類

### 1. バリデーションテスト (`test_00_validation.py`)

無効な設定ファイルでサーバーが適切にエラーを返すかをテストします。

**実行例:**
```bash
pytest test_suite/test_00_validation.py
```

### 2. 静的ファイルテスト (`test_01_static_file.py`)

基本的なHTTP GETリクエストと静的ファイルのサービスをテストします。

**テスト項目:**
- 基本的な静的ファイルのGET
- 存在しないファイルへの404レスポンス
- インデックスファイルのサービス
- オートインデックス機能（ON/OFF）

### 3. CGIテスト (`test_02_cgi.py`)

CGIスクリプトの実行をテストします。

**テスト項目:**
- シンプルなCGI GETリクエスト
- CGIへのPOSTリクエスト
- クエリ文字列の処理
- 存在しないCGIスクリプトへの404レスポンス

## テストフィクスチャ

### `managed_server` フィクスチャ

各テストで自動的にWebServを起動・停止します。

**使用例:**
```python
@pytest.mark.config("valid/config_basic_get.yaml")
def test_something(managed_server):
    url = f"{managed_server['base_url']}/path"
    response = requests.get(url)
    assert response.status_code == 200
```

**動作:**
1. 指定された設定ファイルを読み込み
2. サーバーを起動
3. 起動完了まで待機（最大5秒）
4. テスト実行
5. テスト終了後にサーバーを停止

### `build_server` フィクスチャ

テストセッション開始時に自動的にサーバーをビルドします。

## 設定ファイルの配置

### 正常系設定ファイル

`test/confs/valid/` に配置します。

**例:**
- `config_basic_get.yaml` - 基本的なGETテスト用
- `config_index_file.yaml` - インデックスファイルテスト用
- `cgi.yaml` - CGIテスト用

### 異常系設定ファイル

`test/confs/invalid/` に配置します。

**例:**
- `test_invalid_key_server.yaml` - 無効なキーを含むサーバーブロック
- `test_invalid_key_location.yaml` - 無効なキーを含むロケーションブロック

## 既存テストからの移行

`test.bk/` ディレクトリのBashスクリプトから移行したテスト：

| 元のディレクトリ | 新しいテストファイル |
|-----------------|---------------------|
| `validation_test/` | `test_00_validation.py` |
| `static_file_test/` | `test_01_static_file.py` |
| `cgi_test/` | `test_02_cgi.py` |
| `post_test/` | *(今後実装)* |
| `redirect_test/` | *(今後実装)* |
| `session_test/` | *(今後実装)* |

## トラブルシューティング

### ポートが使用中エラー

複数のテストを同時に実行するとポート競合が発生する可能性があります。テストは順次実行してください。

### サーバーが起動しない

1. サーバーがビルドされているか確認：`ls -l webserv`
2. 設定ファイルのパスが正しいか確認
3. ログファイルを確認：`logs/error.log`

### CGIテストが失敗する

1. CGIスクリプトが実行可能か確認：`chmod +x test/test_www/cgi-bin/*.py`
2. Pythonのパスが正しいか確認（設定ファイルの`interpreterPath`）
3. CGIスクリプトに適切なshebangがあるか確認

## 今後の拡張

以下のテストの実装が予定されています：

- [ ] POST/ファイルアップロードテスト
- [ ] リダイレクトテスト
- [ ] セッション管理テスト
- [ ] ログローテーションテスト
- [ ] 負荷テスト（stress/）
- [ ] プロトコル堅牢性テスト（protocol/）

## 参考資料

- [pytest ドキュメント](https://docs.pytest.org/)
- [requests ドキュメント](https://requests.readthedocs.io/)
- [PyYAML ドキュメント](https://pyyaml.org/)
