"""
リダイレクトのテスト（雛形）
"""
import pytest
import requests


class TestRedirects:
    @pytest.mark.config("valid/config_301.yaml")
    def test_redirect_301(self, managed_server):
        url = f"{managed_server['base_url']}/old-path"
        r = requests.get(url, allow_redirects=False)
        assert r.status_code == 301
        assert r.headers.get("Location", "").endswith("/new-path")

    @pytest.mark.config("valid/config_302.yaml")
    def test_redirect_302(self, managed_server):
        url = f"{managed_server['base_url']}/temp-old"
        r = requests.get(url, allow_redirects=False)
        assert r.status_code == 302
        assert r.headers.get("Location", "").endswith("/temp-new")

    @pytest.mark.config("valid/config_external.yaml")
    def test_redirect_external(self, managed_server):
        url = f"{managed_server['base_url']}/external"
        r = requests.get(url, allow_redirects=False)
        assert r.status_code == 302
        assert r.headers.get("Location", "").startswith("http://example.com")

    @pytest.mark.config("valid/config_prefix.yaml")
    def test_redirect_prefix_rules(self, managed_server):
        base = managed_server['base_url']
        r1 = requests.get(f"{base}/prefix", allow_redirects=False)
        r2 = requests.get(f"{base}/prefix/path", allow_redirects=False)
        assert r1.status_code == 301 and r1.headers.get("Location", "").endswith("/new-prefix")
        assert r2.status_code == 301 and r2.headers.get("Location", "").endswith("/new-prefix/path-specific")
