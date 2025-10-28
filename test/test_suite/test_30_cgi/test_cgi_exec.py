"""
CGI 実行の基本テスト
"""
import pytest
import requests


class TestCGIExec:
    @pytest.mark.config("valid/cgi.yaml")
    def test_simple_cgi_get(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/simple.py"
        response = requests.get(url)
        assert response.status_code == 200
        assert "Hello from CGI!" in response.text or "Content-Type" in response.headers

    @pytest.mark.config("valid/cgi.yaml")
    def test_echo_cgi_with_get(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/echo.py"
        response = requests.get(url)
        assert response.status_code == 200

    @pytest.mark.config("valid/cgi.yaml")
    def test_echo_cgi_with_post(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/echo.py"
        data = {"test_data": "hello", "name": "world"}
        response = requests.post(url, data=data)
        assert response.status_code == 200

    @pytest.mark.config("valid/cgi.yaml")
    def test_nonexistent_cgi(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/nonexistent.py"
        response = requests.get(url)
        assert response.status_code == 404
