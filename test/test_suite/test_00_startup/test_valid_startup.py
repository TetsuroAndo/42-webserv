"""
サーバーの正常起動テスト
"""
import pytest
import subprocess
import time
from pathlib import Path


class TestValidStartup:
    """正常な起動、基本ケース"""

    @pytest.fixture
    def webserv_bin(self):
        """webservバイナリのパスを返す"""
        test_dir = Path(__file__).parent.parent.parent
        project_root = test_dir.parent
        return str(project_root / "webserv")

    @pytest.fixture
    def test_valid_config_basic_startup(self, webserv_bin, str=None):
        """有効な設定ファイルは正常に起動できる"""
        test_dir = Path(__file__).parent.parent.parent
        config_path = str(test_dir / "confs" / "valid" / "config_basic_get.yaml")

        proc = subprocess.Popen(
            [webserv_bin, config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        time.sleep(0.5)

        if proc.poll() is None:
            proc.terminate()
            proc.wait()
            assert True, "有効な設定ファイルは起動可能"
        else:
            stdout, stderr = proc.communicate()
            pytest.fail(f"有効な設定ファイルが起動に失敗: {stderr}")

    def test_no_config_file(self, webserv_bin):
        proc = subprocess.Popen(
            [webserv_bin],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        time.sleep(0.5)

        if proc.poll() is None:
            proc.terminate()
            proc.wait()
            assert True, "起動に成功"
        else:
            stdout, stderr = proc.communicate()
            pytest.fail(f"起動に失敗: {stderr}")
