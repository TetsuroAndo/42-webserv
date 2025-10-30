"""
ディレクティブ適用のテスト（雛形）
"""
import pytest
import requests


class TestCoreDirectives:
    @pytest.mark.config("valid/config_index_file.yaml")
    def test_index_applied(self, managed_server):
        r = requests.get(f"{managed_server['base_url']}/")
        assert r.status_code == 200
        assert "Welcome!" in r.text

    @pytest.mark.config("valid/config_autoindex_on.yaml")
    def test_autoindex_on(self, managed_server):
        r = requests.get(f"{managed_server['base_url']}/")
        assert r.status_code == 200

    @pytest.mark.config("valid/config_autoindex_off.yaml")
    def test_autoindex_off(self, managed_server):
        r = requests.get(f"{managed_server['base_url']}/")
        assert r.status_code == 403

    @pytest.mark.config("valid/config_autoindex_off.yaml")
    def test_trailing_slash_redirect(self, managed_server):
        # 末尾スラなしでディレクトリへアクセスした時 301 で / を付ける
        url = f"{managed_server['base_url']}/cgi-bin"
        r = requests.get(url, allow_redirects=False)
        assert r.status_code in (301, 308)
        assert r.headers.get("Location", "").endswith("/cgi-bin/")
