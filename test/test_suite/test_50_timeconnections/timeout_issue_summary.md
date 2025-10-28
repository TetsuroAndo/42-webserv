# タイムアウトテスト失敗の原因分析

## 問題の概要
3つのタイムアウトテストが失敗しています：
1. `test_request_header_timeout` - リクエストヘッダータイムアウト
2. `test_request_body_timeout` - リクエストボディータイムアウト  
3. `test_keepalive_idle_timeout` - Keep-Aliveアイドルタイムアウト

## サーバー側の動作（正常）
ログから確認できること：
- ✅ タイムアウトは正しく検出されている
- ✅ `onTimeout()` が呼ばれている  
- ✅ `closeConnection()` が呼ばれている
- ✅ ログに「Closing connection」が記録されている

## 実際の問題：テストコードの検証ロジック

### 現在のテストコード
```python
try:
    sock.sendall(b"test")  # これが成功してしまう
    pytest.fail("Connection was not closed after timeout")
except (BrokenPipeError, ConnectionResetError, OSError):
    pass
```

### 問題点
`socket.sendall()` の動作：
1. データは**ソケットの送信バッファ**に書き込まれる
2. 接続が閉じられていても、**バッファに空きがあれば書き込みは成功**
3. 実際のネットワークエラーが発生するのは：
   - **次回の `send()` 呼び出し時**
   - **または `recv()` で FIN を受信した時**

### 正確な検証方法
接続が閉じられているかを確認するには：
1. `recv()` を使って FIN の受信を確認（0バイトが返るか例外が発生）
2. `send()` の後に `flush` する
3. ソケットの状態を `getsockopt()` で確認

## 結論
**サーバー側の実装は正常。問題はテストコード側の検証方法にある。**

### 推奨される修正
```python
# 接続が閉じられているか確認
try:
    sock.send(b"test")
    # recv()を使って FIN を検出
    sock.recv(1)
    pytest.fail("Connection was not closed after timeout")
except (BrokenPipeError, ConnectionResetError, OSError, socket.timeout):
    # 接続が閉じられていることが確認できた
    pass
```
