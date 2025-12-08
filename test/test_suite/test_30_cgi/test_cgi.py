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

    @pytest.mark.config("valid/cgi.yaml")
    def test_check_cgi_env(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/test.py"
        response = requests.get(url)
        assert response.status_code == 200
        # TODO:環境変数のテストを書く
        # "CONTENT_LENGTH"
        # "CONTENT_TYPE"
        # "QUERY_STRING"
        # "REMOTE_HOST"
        # "REMOTE_IDENT"
        # "REQUEST_METHOD"
        # "SCRIPT_NAME"
        # "SERVER_PORT"
        # "SERVER_PROTOCOL"
        # "SERVER_SOFTWARE"
    @pytest.mark.config("valid/cgi.yaml")
    def test_check_cgi_env_content_length(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/test.py"
        response = requests.get(url)
        assert response.status_code == 200
        # TODO:環境変数のテストを書く
        # "CONTENT_LENGTH"

    @pytest.mark.config("valid/cgi.yaml")
    def test_shell_script_cgi(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/shell_script.sh"
        response = requests.get(url)
        assert response.status_code == 200
        assert "Webserv/42" in response.text
    @pytest.mark.config("valid/cgi.yaml")
    def test_cgi_wait_2sec(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/wait_2sec.py"
        response = requests.get(url)
        assert response.status_code == 200
        assert  "Hello from CGI!" in response.text
