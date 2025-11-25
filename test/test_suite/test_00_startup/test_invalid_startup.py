"""
起動時の異常系（ファイル権限）
"""
import os
import stat
import time
import pytest
import subprocess
from pathlib import Path


class TestInvalidStartup:
    """無効な起動ケースのテスト"""

    @pytest.fixture
    def webserv_bin(self):
        """webservバイナリのパスを返す"""
        test_dir = Path(__file__).parent.parent.parent
        project_root = test_dir.parent
        return str(project_root / "webserv")

    def test_config_no_permission(self, webserv_bin):
        """
        先頭の有効設定ファイルの読み取り権限を剥奪→起動→終了コード0以外を期待→権限復元
        """
        test_dir = Path(__file__).parent.parent.parent
        valid_dir = test_dir / "confs" / "valid"
        yaml_files = sorted(valid_dir.glob("*.yaml"))
        if not yaml_files:
            pytest.skip(f"有効な設定ファイルが見つかりません: {valid_dir}")

        target = yaml_files[0]
        original_mode = stat.S_IMODE(os.lstat(target).st_mode)

        # 読み取り権限を剥奪（所有者／グループ／その他）
        try:
            try:
                new_mode = original_mode & ~(stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH)
                os.chmod(target, new_mode)
            except PermissionError:
                pytest.skip("権限変更が許可されていない環境のためスキップ")
            assert not os.access(target, os.R_OK)

            # サーバー起動と終了コード評価（0以外で終了するべき）
            result = subprocess.run(
                [webserv_bin, str(target)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=5,
            )
            assert result.returncode != 0, (
                f"終了コードが0です: {result.returncode}\n"
                f"stderr:\n{result.stderr}"
            )
        finally:
            try:
                os.chmod(target, original_mode)
            except Exception:
                print("fail chmod")
                pass

    def test_duplicate_start_same_config_fails(self, webserv_bin):
        """
        先頭の有効設定でプロセスAを起動中に、同じ設定でプロセスBを起動→非0終了を期待
        （ポート重複などにより起動に失敗すること）
        """
        test_dir = Path(__file__).parent.parent.parent
        valid_dir = test_dir / "confs" / "valid"
        yaml_files = sorted(valid_dir.glob("*.yaml"))
        if not yaml_files:
            pytest.skip(f"有効な設定ファイルが見つかりません: {valid_dir}")

        target = yaml_files[0]

        proc1 = subprocess.Popen(
            [webserv_bin, str(target)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            time.sleep(0.5)
            if proc1.poll() is not None:
                out1, err1 = proc1.communicate()
                pytest.fail(f"一次起動が継続しませんでした。stdout:\n{out1}\nstderr:\n{err1}")

            # 二重起動を試み、非0終了を期待
            result = subprocess.run(
                [webserv_bin, str(target)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=5,
            )
            assert result.returncode != 0, (
                "同一設定の二重起動が成功してしまいました\n"
                f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
            )
        finally:
            if proc1.poll() is None:
                proc1.terminate()
                try:
                    proc1.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    proc1.kill()
