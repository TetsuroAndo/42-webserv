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
        # "CONTENT_LENGTH"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/CONTENT_LENGTH.sh"
        response = requests.post(url, "hello")
        assert response.status_code == 200
        assert "5" in response.text
        # "QUERY_STRING"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/QUERY_STRING.sh?id=0"
        response = requests.get(url)
        assert response.status_code == 200
        assert "id=0" in response.text
        # "REQUEST_METHOD"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/REQUEST_METHOD.sh"
        response = requests.get(url)
        assert response.status_code == 200
        assert "GET" in response.text
        # "SCRIPT_NAME"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/SCRIPT_NAME.sh"
        response = requests.get(url)
        assert response.status_code == 200
        assert "SCRIPT_NAME.sh" in response.text
        # "SERVER_PORT"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/SERVER_PORT.sh"
        response = requests.get(url)
        assert response.status_code == 200
        assert "8082" in response.text
        # "SERVER_PROTOCOL"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/SERVER_PROTOCOL.sh"
        response = requests.get(url)
        assert response.status_code == 200
        assert "HTTP/1.1" in response.text
        # "SERVER_SOFTWARE"
        url = f"{managed_server['base_url']}/cgi-bin/env_test_sh/SERVER_SOFTWARE.sh"
        response = requests.get(url)
        assert response.status_code == 200
        assert "Webserv/42" in response.text

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

    @pytest.mark.config("valid/cgi.yaml")
    def test_fail_cgi(self, managed_server):
        url = f"{managed_server['base_url']}/cgi-bin/fail.sh"
        response = requests.get(url)
        assert response.status_code == 500