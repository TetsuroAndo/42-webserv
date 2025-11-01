"""
サーバーの正常起動テスト
"""
import proc
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

    def test_without_config_file(self, webserv_bin):
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
            assert False, "起動に失敗"


    def test_valid_config(self, webserv_bin):
        """有効な設定ファイルは正常に起動できる"""
        test_dir = Path(__file__).parent.parent.parent
        valid_dir = test_dir / "confs" / "valid"
        yaml_files = list(valid_dir.glob("*.yaml"))
        if not yaml_files:
            pytest.skip(f"有効な設定ファイルが見つかりません: {valid_dir}")
        for yaml_file in yaml_files:
            proc = subprocess.Popen(
                [webserv_bin, str(yaml_file)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            time.sleep(0.5)
            if proc.poll() is None:
                proc.terminate()
                proc.wait()
            else:
                stdout, stderr = proc.communicate()
                pytest.fail(f"有効な設定ファイル {yaml_file.name} が起動に失敗: {stderr}")
