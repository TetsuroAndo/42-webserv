"""
設定ファイルのバリデーション（異常系）
"""
import pytest
import subprocess
from pathlib import Path


class TestInvalidConfig:
    """不正な設定ファイルでの起動失敗テスト"""

    @pytest.fixture
    def webserv_bin(self):
        """webservバイナリのパスを返す"""
        test_dir = Path(__file__).parent.parent.parent
        project_root = test_dir.parent
        return str(project_root / "webserv")

    def test_invalid_key_server(self, webserv_bin):
        """無効なキーがserverブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_server.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"
        assert "unknown directive" in result.stderr.lower() or result.returncode != 0

    def test_invalid_key_location(self, webserv_bin):
        """無効なキーがlocationブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_location.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"

    def test_invalid_key_listen(self, webserv_bin):
        """無効なキーがlistenブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_listen.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"

    def test_invalid_key_log(self, webserv_bin):
        """無効なキーがlogブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_log.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"
