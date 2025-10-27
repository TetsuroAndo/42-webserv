"""
基本的な静的ファイルサービングのテスト
"""
import pytest
import requests


class TestStaticFile:
    """静的ファイルサービングのテストスイート"""

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_basic_static_get(self, managed_server):
        """基本的な静的ファイルのGETリクエスト"""
        url = f"{managed_server['base_url']}/hello.txt"
        response = requests.get(url)

        assert response.status_code == 200
        assert "Hello from webserv test!" in response.text
        assert "text/plain" in response.headers.get("Content-Type", "")

    @pytest.mark.config("valid/config_basic_get.yaml")
    def test_static_file_not_found(self, managed_server):
        """存在しないファイルへのリクエスト"""
        url = f"{managed_server['base_url']}/non-existent-file.txt"
        response = requests.get(url)

        assert response.status_code == 404

    @pytest.mark.config("valid/config_index_file.yaml")
    def test_index_file(self, managed_server):
        """インデックスファイルのサービス"""
        url = f"{managed_server['base_url']}/"
        response = requests.get(url)

        assert response.status_code == 200
        assert "Welcome!" in response.text
        assert "text/html" in response.headers.get("Content-Type", "")

    @pytest.mark.config("valid/config_autoindex_on.yaml")
    def test_autoindex_on(self, managed_server):
        """オートインデックスONの場合"""
        url = f"{managed_server['base_url']}/autoindex_test_dir/"
        response = requests.get(url)

        # オートインデックスがONの場合、ディレクトリリストが返される
        assert response.status_code == 200

    @pytest.mark.config("valid/config_autoindex_off.yaml")
    def test_autoindex_off_forbidden(self, managed_server):
        """オートインデックスOFFのディレクトリアクセス"""
        url = f"{managed_server['base_url']}/no_autoindex_dir/"
        response = requests.get(url)

        # オートインデックスがOFFの場合、403を返す
        assert response.status_code == 403
