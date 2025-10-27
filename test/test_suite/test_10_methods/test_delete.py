"""
DELETE メソッドのテスト（雛形）
"""
import pytest
import requests
from pathlib import Path


class TestDELETE:
    @pytest.mark.config("valid/post.yaml")
    def test_delete_existing_file(self, managed_server):
        # 事前にPOSTで作成
        base = managed_server['base_url']
        url_create = f"{base}/upload/to_delete.txt"
        data = b"delete me\n"
        r = requests.post(url_create, data=data, headers={"Content-Type": "text/plain"})
        assert r.status_code in (200, 201)
        # DELETE 実行
        url_del = f"{base}/upload/to_delete.txt"
        resp = requests.delete(url_del)
        assert resp.status_code in (200, 204)
        # 実体消去
        assert not (Path("test/post_test/uploads") / "to_delete.txt").exists()

    @pytest.mark.config("valid/post.yaml")
    def test_delete_not_found(self, managed_server):
        url = f"{managed_server['base_url']}/upload/no_such.txt"
        resp = requests.delete(url)
        assert resp.status_code == 404

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_delete_method_not_allowed(self, managed_server):
        url = f"{managed_server['base_url']}/hello.txt"
        resp = requests.delete(url)
        assert resp.status_code == 405
