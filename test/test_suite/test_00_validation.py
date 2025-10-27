"""
設定ファイルのバリデーションテスト
"""
import pytest
import subprocess
import yaml
import os
from pathlib import Path


class TestConfigValidation:
    """設定ファイルのバリデーションテスト"""

    @pytest.fixture
    def webserv_bin(self):
        """webservバイナリのパスを返す"""
        # test/ディレクトリから見て、プロジェクトルートのwebservを指す
        test_dir = Path(__file__).parent.parent
        project_root = test_dir.parent
        return str(project_root / "webserv")

    def test_invalid_key_server(self, webserv_bin):
        """無効なキーがserverブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_server.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"
        assert "unknown directive" in result.stderr.lower() or result.returncode != 0

    def test_invalid_key_location(self, webserv_bin):
        """無効なキーがlocationブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_location.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"

    def test_invalid_key_listen(self, webserv_bin):
        """無効なキーがlistenブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_listen.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"

    def test_invalid_key_log(self, webserv_bin):
        """無効なキーがlogブロックに含まれる場合"""
        test_dir = Path(__file__).parent.parent
        config_path = str(test_dir / "confs" / "invalid" / "test_invalid_key_log.yaml")

        result = subprocess.run(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        assert result.returncode != 0, "サーバーは無効なキーで起動に失敗すべき"

    def test_valid_config(self, webserv_bin):
        """有効な設定ファイルは正常に起動できる"""
        test_dir = Path(__file__).parent.parent
        config_path = str(test_dir / "confs" / "valid" / "config_basic_get.yaml")

        # サーバーを起動してすぐに終了させる
        proc = subprocess.Popen(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # 少し待ってからkill
        import time
        time.sleep(0.5)

        if proc.poll() is None:
            # プロセスが生きている場合（正常に起動した）
            proc.terminate()
            proc.wait()
            assert True, "有効な設定ファイルは起動可能"
        else:
            # プロセスが死んでいる場合（起動失敗）
            stdout, stderr = proc.communicate()
            pytest.fail(f"有効な設定ファイルが起動に失敗: {stderr}")
