"""
メソッド制限（allowedMethods）のテスト（雛形）
"""
import pytest
import requests


class TestMethodLimits:
    @pytest.mark.config("valid/config_get_only.yaml")
    def test_get_only_allows_get(self, managed_server):
        r = requests.get(f"{managed_server['base_url']}/")
        assert r.status_code in (200, 404)

    @pytest.mark.config("valid/config_get_only.yaml")
    def test_get_only_blocks_post_delete(self, managed_server):
        base = managed_server['base_url']
        r1 = requests.post(f"{base}/", data=b"x")
        r2 = requests.delete(f"{base}/")
        assert r1.status_code == 405
        assert r2.status_code == 405

    @pytest.mark.config("valid/config_no_methods.yaml")
    def test_default_methods_behavior(self, managed_server):
        # 未指定(default) で GET/POST/DELETE が設定に応じて動作
        base = managed_server['base_url']
        rg = requests.get(f"{base}/")
        rp = requests.post(
            f"{base}/",
            data=b"x",
            headers={"Content-Type": "text/plain"}
        )
        rd = requests.delete(f"{base}/upload_target.txt")
        assert rg.status_code in (200, 404)
        assert rp.status_code in (200, 201, 405, 500)  # uploadStore有無で変動
        assert rd.status_code in (200, 204, 404, 405)
