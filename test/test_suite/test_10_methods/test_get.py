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

        assert response.headers.get("Allow") in "HEAD"

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_prevent_directory_traversal(self, managed_server):
        # ../によるアクセスを検証（ルート外は参照できない）
        base = managed_server["base_url"]

        # さらに上位を狙うパスも 404（ルート脱出は不可）
        r2 = requests.get(f"{base}/../../README.md")
        assert r2.status_code == 404

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_prevent_encoded_directory_traversal(self, managed_server):
        # `%2e%2e/` を含むエンコード済みの親参照は無効化される（404）
        base = managed_server["base_url"]

        r = requests.get(f"{base}/%2e%2e/index.html")
        assert r.status_code == 404

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_safe_url_decode_special_chars(self, managed_server):
        # %2F（スラッシュ）や%20（スペース）のデコード安全性を検証
        base = managed_server["base_url"]

        # %2F はディレクトリ区切りとして扱われず、404 となること
        r1 = requests.get(f"{base}/hello%2Fworld.txt")
        assert r1.status_code == 404

        # %20 を含むパス（該当ファイルなし）は 404 となること
        r2 = requests.get(f"{base}/hello%20.txt")
        assert r2.status_code == 404
