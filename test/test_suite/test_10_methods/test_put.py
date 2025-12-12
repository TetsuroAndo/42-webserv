"""
PUT メソッドのテスト
"""
import pytest
import requests


class TestPOST:
    pass

from pathlib import Path

@pytest.mark.config("valid/put.yaml")
def test_put(managed_server):
    # ファイルを作成
    url = f"{managed_server['base_url']}/upload/test.txt"

    # 一旦削除
    resp = requests.delete(url)

    data = "hello world\n"
    resp = requests.put(url, data=data, headers={"Content-Type": "text/plain"})
    assert resp.status_code == 201

    # 存在確認
    resp = requests.get(url)
    assert resp.status_code == 200
    assert resp.text == data

    # 更新
    data = "hello world 2\n"
    resp = requests.put(url, data=data, headers={"Content-Type": "text/plain"})
    assert resp.status_code in (200, 204)

    # 内容確認
    resp = requests.get(url)
    assert resp.status_code == 200
    assert resp.text == data

    # ファイル削除
    resp = requests.delete(url)
    assert resp.status_code == 204


@pytest.mark.config("valid/put.yaml")
def test_put_payload_too_large(managed_server):
    url = f"{managed_server['base_url']}/upload/too_large.txt"

    # 2KB データ生成
    body = b"x" * 2048
    resp = requests.put(url, data=body, headers={"Content-Type": "application/octet-stream"})
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

@pytest.mark.config("valid/put.yaml")
def test_put_no_ex_dir(managed_server):
    url = f"{managed_server['base_url']}/EX-upload/test.txt"
    resp = requests.put(url, data=b"hello", headers={"Content-Type": "text/plain"})
    assert resp.status_code == 405

    upload_dir = Path("test/www_test/uploads")
    txt_files = list(upload_dir.glob("*.txt"))
    assert not txt_files, f"Unexpected .txt file(s) found: {[f.name for f in txt_files]}"
