"""
設定ファイルのバリデーション（異常系）
"""
import time
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

    def test_no_exist_file(self, webserv_bin):
        proc = subprocess.Popen(
            [webserv_bin, "no_exist.yaml"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        time.sleep(0.5)

        if proc.poll() is None:
            proc.terminate()
            proc.wait()
            assert False, "起動に成功"
        else:
            assert True, "起動に失敗"

    def test_invalid_config(self, webserv_bin):
        """無効な設定ファイルは絶対に起動できない"""
        test_dir = Path(__file__).parent.parent.parent
        valid_dir = test_dir / "confs" / "invalid"
        yaml_files = list(valid_dir.glob("*.yaml"))
        if not yaml_files:
            pytest.skip(f"無効な設定ファイルが見つかりません: {valid_dir}")
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
                pytest.fail(f"無効な設定ファイル {yaml_file.name} が起動に成功")


