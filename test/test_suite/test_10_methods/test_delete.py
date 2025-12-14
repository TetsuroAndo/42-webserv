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
        assert not (Path("test/post_test/uploads") / "delete_me.txt").exists()

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

    @pytest.mark.config("valid/post.yaml")
    def test_delete_no_permission_file(self, managed_server):
        base = managed_server['base_url']

        here = Path(__file__).resolve().parent
        file_path = here / "../../test_www/uploads/delete_me.txt"
        file_path = file_path.resolve()

        with open(file_path, "wb") as f:
            f.write(b"delete me\n")
        # DELETE 実行
        url_del = f"{base}/upload/delete_me.txt"

        upload_dir = here / "../../test_www/uploads/"

        # ディレクトリの x 権限を抜く
        old_mode = upload_dir.stat().st_mode
        upload_dir.chmod(0o666)  # x 無し：ディレクトリ内アクセス不能

        try:
            # DELETE 実行
            resp = requests.delete(url_del)

            # 評価 禁止 or 見えない
            assert resp.status_code in (403, 404)

        finally:
            # 後始末：権限を戻す
            upload_dir.chmod(old_mode)
            # ファイルが残っていることを確認
            assert file_path.exists()
            # ファイル消す
            if file_path.exists():
                file_path.unlink()

    @pytest.mark.config("valid/post.yaml")
    def test_delete_dir(self, managed_server):
        base = managed_server['base_url']
        # DELETE 実行
        url_del = f"{base}/upload/"
        resp = requests.delete(url_del)
        assert resp.status_code in (403, 404)

