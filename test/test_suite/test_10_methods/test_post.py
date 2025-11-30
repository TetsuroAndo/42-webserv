"""
POST メソッドのテスト（雛形）
"""
import pytest
import requests


class TestPOST:
    pass


from pathlib import Path

@pytest.mark.config("valid/post.yaml")
def test_post_created_and_file_saved(managed_server):
    # 最小データを送信
    url = f"{managed_server['base_url']}/upload"
    data = b"hello world\n"
    resp = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
    assert resp.status_code in (200, 201)

    upload_dir = Path("test/test_www/uploads")
    txt_files = list(upload_dir.glob("*.txt"))

    assert txt_files, f"No .txt file found in {upload_dir}"

    saved = txt_files[0]
    assert saved.read_bytes() == data

    saved.unlink()


@pytest.mark.config("valid/post_test.yaml")
def test_post_payload_too_large(managed_server):
    url = f"{managed_server['base_url']}/upload"

    # 2KB データ生成
    body = b"x" * 2048
    resp = requests.post(url, data=body, headers={"Content-Type": "application/octet-stream"})
    assert resp.status_code == 413

    # アップロードディレクトリ確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))

    # .txt ファイルが作成されていないことを確認
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"

    # 万が一残っていたら削除してクリーンアップ
    for f in txt_files:
        try:
            f.unlink()
        except Exception as e:
            print(f"Warning: failed to delete {f}: {e}")

@pytest.mark.config("valid/config_basic_get.yaml")
def test_post_method_not_allowed(managed_server):
    url = f"{managed_server['base_url']}/"
    resp = requests.post(url, data=b"x", headers={"Content-Type": "text/plain"})
    assert resp.status_code == 405

@pytest.mark.config("valid/post_test.yaml")
def test_post_no_content_length(managed_server):
    url = f"{managed_server['base_url']}/upload"
    # Content-Length: 0 にも関わらずボディを送る（不整合）。
    # 生ソケットで送信して Content-Length を厳密に制御する。
    import socket
    host = "127.0.0.1"
    port = managed_server["port"]

    req = (
        "POST /upload HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n"
        "HELLO"  # 実体は5バイト送る
    ).encode()

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5)
    s.connect((host, port))
    try:
        s.sendall(req)
        resp = s.recv(4096)
    finally:
        s.close()

    # ステータスコード 400 を期待（不正なリクエスト）
    assert resp.startswith(b"HTTP/") and b" 400 " in resp.split(b"\r\n", 1)[0]

    # アップロードディレクトリ確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    # .txt ファイルが作成されていないことを確認
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"

    # 万が一残っていたら削除してクリーンアップ
    for f in txt_files:
        try:
            f.unlink()
        except Exception as e:
            print(f"Warning: failed to delete {f}: {e}")

@pytest.mark.config("valid/post_test.yaml")
def test_post_no_content(managed_server):
    # Content-Length に反してボディを送らないケース。
    # タイムアウトを避けるためヘッダー送信後に直ちに切断し、作成物が無いことのみ確認。
    import socket
    host = "127.0.0.1"
    port = managed_server["port"]

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(3)
    s.connect((host, port))
    try:
        headers = (
            "POST /upload HTTP/1.1\r\n"
            f"Host: {host}:{port}\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 10\r\n"
            "Connection: close\r\n"
            "\r\n"
        ).encode()
        s.sendall(headers)
        # ボディは送らずに切断
    finally:
        s.close()

    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    # .txt ファイルが作成されていないことを確認
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"


@pytest.mark.config("valid/post_test.yaml")
def test_post_different_content_length_and_body(managed_server):
    # Content-Length とボディサイズが不一致のケース（小さすぎる値を宣言）。
    # 不正リクエストとして 400 を期待。
    import socket
    host = "127.0.0.1"
    port = managed_server["port"]

    body = b"ABCDEFGHIJ"  # 実際は10バイト
    content_length = 5      # 故意に小さく宣言

    req = (
        "POST /upload HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        "Content-Type: text/plain\r\n"
        f"Content-Length: {content_length}\r\n"
        "Connection: close\r\n"
        "\r\n"
    ).encode() + body

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(5)
    s.connect((host, port))
    try:
        s.sendall(req)
        resp = s.recv(4096)
    finally:
        s.close()

    assert resp.startswith(b"HTTP/") and b" 400 " in resp.split(b"\r\n", 1)[0]

    # ファイルが作成されていないことを確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"

@pytest.mark.config("valid/post_test.yaml")
def test_post_no_ex_dir(managed_server):
    url = f"{managed_server['base_url']}/EX-upload"
    # 存在しないディレクトリへの POST は 404 を期待
    resp = requests.post(url, data=b"hello", headers={"Content-Type": "text/plain"})
    assert resp.status_code == 404

    # 念のため、アップロードディレクトリにファイルが作られていないことも確認
    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"
