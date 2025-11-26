"""
GET メソッドのテスト
"""
import pytest
import requests


class TestGET:
    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_basic_static_get(self, managed_server):
        url = f"{managed_server['base_url']}/hello.txt"
        response = requests.get(url)
        assert response.status_code == 200
        assert "Hello from webserv test!" in response.text
        assert "text/plain" in response.headers.get("Content-Type", "")

        url = f"{managed_server['base_url']}/img.jpg"
        response = requests.get(url)
        assert response.status_code == 200
        assert "image/jpeg" in response.headers.get("Content-Type", "")

        url = f"{managed_server['base_url']}/unknown_ex.unknown"
        response = requests.get(url)
        assert response.status_code == 200
        assert "application/octet-stream" in response.headers.get("Content-Type", "")

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_static_file_not_found(self, managed_server):
        url = f"{managed_server['base_url']}/non-existent-file.txt"
        response = requests.get(url)
        assert response.status_code == 404

    @pytest.mark.config("valid/config_index_file.yaml")
    def test_index_file(self, managed_server):
        url = f"{managed_server['base_url']}"
        response = requests.get(url)
        assert response.status_code == 200
        assert "Welcome!" in response.text
        assert "text/html" in response.headers.get("Content-Type", "")

    @pytest.mark.config("valid/config_autoindex_on.yaml")
    def test_autoindex_on(self, managed_server):
        url = f"{managed_server['base_url']}/"
        response = requests.get(url)
        assert response.status_code == 200

    @pytest.mark.config("valid/config_autoindex_on.yaml")
    def test_autoindex_on_with_index_file(self, managed_server):
        url = f"{managed_server['base_url']}/with_index/"
        response = requests.get(url)
        assert response.status_code == 200
        assert "<html><head><title>Index of " in response.text

    @pytest.mark.config("valid/config_autoindex_on.yaml")
    def test_autoindex_on_with_index_file_no_slush(self, managed_server):
        url = f"{managed_server['base_url']}/with_index"
        response = requests.get(url)
        assert response.status_code == 200
        assert "<html><body><h1>Welcome!</h1></body></html>" in response.text

    @pytest.mark.config("valid/config_autoindex_with_index_root.yaml")
    def test_autoindex_on_with_index_file_no_slush_root(self, managed_server):
        url = f"{managed_server['base_url']}"
        response = requests.get(url)
        assert response.status_code == 200
        assert "<html><body><h1>Welcome!</h1></body></html>" in response.text

    @pytest.mark.config("valid/config_autoindex_with_index_root.yaml")
    def test_autoindex_on_with_index_file_root(self, managed_server):
        # rootにautoindexとindexが両方設定されている場合、
        # リクエストが同一のものになり区別できないため、
        # indexが優先されているかを確認します。
        url = f"{managed_server['base_url']}/"
        response = requests.get(url)
        assert response.status_code == 200
        assert "<html><body><h1>Welcome!</h1></body></html>" in response.text

    @pytest.mark.config("valid/config_autoindex_off.yaml")
    def test_autoindex_off_forbidden(self, managed_server):
        url = f"{managed_server['base_url']}/"
        response = requests.get(url)
        assert response.status_code == 403

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_get_method_not_allowed(self, managed_server):
        url = f"{managed_server['base_url']}/not_allow_get/hello.txt"
        response = requests.get(url)
        assert response.status_code == 405
