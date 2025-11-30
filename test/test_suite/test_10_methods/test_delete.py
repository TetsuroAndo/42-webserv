"""
DELETE メソッドのテスト（雛形）
"""
import pytest
import requests
import os
from pathlib import Path


class TestDELETE:
    @pytest.mark.config("valid/post.yaml")
    def test_delete_existing_file(self, managed_server):
        # ファイルを作成
        base = managed_server['base_url']

        here = Path(__file__).resolve().parent
        file_path = here / "../../test_www/uploads/delete_me.txt"
        file_path = file_path.resolve()

        with open(file_path, "wb") as f:
            f.write(b"delete me\n")
        # DELETE 実行
        url_del = f"{base}/upload/delete_me.txt"
        resp = requests.delete(url_del)
        assert resp.status_code in (200, 204)

        res = requests.get(url_del)
        assert res.status_code == 404
        # 実体消去
        assert not (Path("test/post_test/uploads") / "to_delete.txt").exists()

    @pytest.mark.config("valid/post.yaml")
    def test_delete_not_found(self, managed_server):
        url = f"{managed_server['base_url']}/upload/no_such_file.png"
        resp = requests.delete(url)
        assert resp.status_code == 404

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_delete_method_not_allowed(self, managed_server):
        url = f"{managed_server['base_url']}/hello.txt"
        resp = requests.delete(url)
        assert resp.status_code == 405
