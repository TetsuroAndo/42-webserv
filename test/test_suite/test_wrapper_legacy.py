"""
既存のBashテストスクリプトをpytestでラップするテスト
段階的な移行戦略：最終的にはtest_*_legacy.pyの各テストをネイティブ実装に置き換える
"""
import pytest
import subprocess
from pathlib import Path


# プロジェクトルートを取得
PROJECT_ROOT = Path(__file__).parent.parent.parent
TEST_BK_DIR = PROJECT_ROOT / "test.bk"


class TestLegacyWrapper:
    """test.bkのBashスクリプトをラップするテストクラス"""

    @pytest.fixture(autouse=True)
    def setup(self):
        """テスト前にサーバーをビルド"""
        # Makefileのルールと同じく、ビルドはセッション開始時に自動で行われる
        pass

    def test_validation_scripts(self):
        """test.bk/validation_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "validation_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=30
        )

        if result.returncode != 0:
            print("\n--- STDOUT ---")
            print(result.stdout)
            print("\n--- STDERR ---")
            print(result.stderr)

        assert result.returncode == 0, f"validation_test/run.sh が失敗しました (exit code: {result.returncode})"

    def test_static_file_scripts(self):
        """test.bk/static_file_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "static_file_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=30
        )

        if result.returncode != 0:
            print("\n--- STDOUT ---")
            print(result.stdout)
            print("\n--- STDERR ---")
            print(result.stderr)

        assert result.returncode == 0, f"static_file_test/run.sh が失敗しました (exit code: {result.returncode})"

    def test_cgi_scripts(self):
        """test.bk/cgi_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "cgi_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=30
        )

        if result.returncode != 0:
            print("\n--- STDOUT ---")
            print(result.stdout)
            print("\n--- STDERR ---")
            print(result.stderr)

        assert result.returncode == 0, f"cgi_test/run.sh が失敗しました (exit code: {result.returncode})"

    def test_post_scripts(self):
        """test.bk/post_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "post_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=30
        )

        if result.returncode != 0:
            print("\n--- STDOUT ---")
            print(result.stdout)
            print("\n--- STDERR ---")
            print(result.stderr)

        assert result.returncode == 0, f"post_test/run.sh が失敗しました (exit code: {result.returncode})"

    def test_redirect_scripts(self):
        """test.bk/redirect_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "redirect_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=30
        )

        if result.returncode != 0:
            print("\n--- STDOUT ---")
            print(result.stdout)
            print("\n--- STDERR ---")
            print(result.stderr)

        assert result.returncode == 0, f"redirect_test/run.sh が失敗しました (exit code: {result.returncode})"

    @pytest.mark.skip(reason="セッションテストは実装中")
    def test_session_scripts(self):
        """test.bk/session_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "session_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=30
        )

        assert result.returncode == 0, f"session_test/run.sh が失敗しました (exit code: {result.returncode})"

    @pytest.mark.skip(reason="load_testは別途実装予定")
    def test_load_scripts(self):
        """test.bk/load_test/run.sh を実行"""
        script_path = TEST_BK_DIR / "load_test" / "run.sh"

        if not script_path.exists():
            pytest.skip(f"スクリプトが存在しません: {script_path}")

        result = subprocess.run(
            ["bash", str(script_path)],
            capture_output=True,
            text=True,
            cwd=str(PROJECT_ROOT),
            timeout=60  # ロードテストは時間がかかる
        )

        assert result.returncode == 0, f"load_test/run.sh が失敗しました (exit code: {result.returncode})"
