"""
CGI機能のテスト
"""
import pytest
import requests


class TestCGI:
    """CGI機能のテストスイート"""

    @pytest.mark.config("valid/cgi.yaml")
    def test_simple_cgi_get(self, managed_server):
        """シンプルなCGI GETリクエスト"""
        url = f"{managed_server['base_url']}/cgi-bin/simple.py"
        response = requests.get(url)

        assert response.status_code == 200
        assert "Hello from CGI!" in response.text or "Content-Type" in response.headers

    @pytest.mark.config("valid/cgi.yaml")
    def test_echo_cgi_with_get(self, managed_server):
        """Echo CGIへのGETリクエスト"""
        url = f"{managed_server['base_url']}/cgi-bin/echo.py"
        response = requests.get(url)

        assert response.status_code == 200

    @pytest.mark.config("valid/cgi.yaml")
    def test_echo_cgi_with_post(self, managed_server):
        """Echo CGIへのPOSTリクエスト"""
        url = f"{managed_server['base_url']}/cgi-bin/echo.py"
        data = {"test_data": "hello", "name": "world"}
        response = requests.post(url, data=data)

        assert response.status_code == 200

    @pytest.mark.config("valid/cgi.yaml")
    def test_echo_cgi_with_query_string(self, managed_server):
        """クエリ文字列付きのCGIリクエスト"""
        url = f"{managed_server['base_url']}/cgi-bin/echo.py"
        params = {"foo": "bar", "baz": "qux"}
        response = requests.get(url, params=params)

        assert response.status_code == 200
        assert "foo=bar" in response.text or params.get("foo") is not None

    @pytest.mark.config("valid/cgi.yaml")
    def test_nonexistent_cgi(self, managed_server):
        """存在しないCGIスクリプトへのリクエスト"""
        url = f"{managed_server['base_url']}/cgi-bin/nonexistent.py"
        response = requests.get(url)

        assert response.status_code == 404
