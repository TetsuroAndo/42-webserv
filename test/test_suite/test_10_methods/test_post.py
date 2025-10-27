"""
POST メソッドのテスト（雛形）
"""
import pytest
import requests
from pathlib import Path


class TestPOST:
    @pytest.mark.config("valid/post.yaml")
    def test_post_created_and_file_saved(self, managed_server, tmp_path):
        # 最小データを送信
        url = f"{managed_server['base_url']}/upload/test_upload.txt"
        data = b"hello world\n"
        resp = requests.post(url, data=data, headers={"Content-Type": "text/plain"})
        assert resp.status_code in (200, 201)
        # 実体確認
        upload_dir = Path("test/post_test/uploads")
        saved = upload_dir / "test_upload.txt"
        assert saved.exists()
        assert saved.read_bytes() == data

    @pytest.mark.config("valid/post_test.yaml")
    def test_post_payload_too_large(self, managed_server):
        url = f"{managed_server['base_url']}/upload/too_large.bin"
        # 2KB 生成
        body = b"x" * 2048
        resp = requests.post(url, data=body, headers={"Content-Type": "application/octet-stream"})
        assert resp.status_code == 413
        # ファイル未作成
        upload_dir = Path("test/post_test/uploads")
        assert not (upload_dir / "too_large.bin").exists()

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_post_method_not_allowed(self, managed_server):
        url = f"{managed_server['base_url']}/hello.txt"
        resp = requests.post(url, data=b"x", headers={"Content-Type": "text/plain"})
        assert resp.status_code == 405

    @pytest.mark.config("valid/post.yaml")
    def test_post_chunked_transfer(self, managed_server):
        # TODO: chunked 送信（requests は自動で CL を付けるため、raw socket 実装が必要）
        # ここでは保留のスキップ
        pytest.skip("chunked POST は raw 実装で後続対応")
