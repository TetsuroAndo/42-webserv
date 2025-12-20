"""
CGI ログ出力のテスト
- CGIへの送信内容（リクエストボディ）のログ
- CGIからの標準エラー出力のログ
"""
import pytest
import requests
import time
from pathlib import Path


class TestCGILogging:
    """CGIログ出力のテストクラス"""
    LOG_FILE = "test/test_www/uploads/cgi_logging_test.log"

    @pytest.fixture(autouse=True)
    def cleanup_log_file(self):
        """テスト後にログファイルをクリーンアップするfixture"""
        yield
        log_file_path = Path(self.LOG_FILE)
        # テスト後にログファイルを削除
        if log_file_path.exists():
            log_file_path.unlink()

    @pytest.mark.config("valid/cgi_logging.yaml")
    def test_cgi_stderr_logging(self, managed_server):
        """CGIの標準エラー出力がログに記録されることを確認"""
        url = f"{managed_server['base_url']}/cgi-bin/stderr_test.py"
        response = requests.get(url)

        assert response.status_code == 200
        assert "This is stdout output" in response.text

        # ログファイルを確認
        log_file_path = Path(self.LOG_FILE)
        if not log_file_path.exists():
            pytest.skip(f"Log file not found: {log_file_path}")

        # ログファイルの内容を読み込む
        time.sleep(0.5)  # ログが書き込まれるのを待つ
        with open(log_file_path, 'r') as f:
            log_content = f.read()

        # 標準エラー出力がログに記録されていることを確認
        assert "CGI stderr output" in log_content
        assert "This is stderr output line 1" in log_content
        assert "This is stderr output line 2" in log_content
        assert "Error message: Something went wrong" in log_content

    @pytest.mark.config("valid/cgi_logging.yaml")
    def test_cgi_request_body_logging(self, managed_server):
        """CGIへの送信内容（リクエストボディ）がログに記録されることを確認"""
        url = f"{managed_server['base_url']}/cgi-bin/post_body_test.py"
        test_body = "test request body data 42/67"

        response = requests.post(url, data=test_body)

        assert response.status_code == 200

        # ログファイルを確認
        log_file_path = Path(self.LOG_FILE)
        if not log_file_path.exists():
            pytest.skip(f"Log file not found: {log_file_path}")

        # ログファイルの内容を読み込む
        time.sleep(0.5)  # ログが書き込まれるのを待つ
        with open(log_file_path, 'r') as f:
            log_content = f.read()

        # リクエストボディがログに記録されていることを確認
        assert "Sending request body to CGI" in log_content
        assert test_body in log_content
        assert "bodySize" in log_content

    @pytest.mark.config("valid/cgi_logging.yaml")
    def test_cgi_stderr_not_in_response(self, managed_server):
        """CGIの標準エラー出力がHTTPレスポンスに含まれないことを確認"""
        url = f"{managed_server['base_url']}/cgi-bin/stderr_test.py"
        response = requests.get(url)

        assert response.status_code == 200
        response_text = response.text
        # 標準エラー出力の内容がレスポンスに含まれていないことを確認
        assert "This is stderr output line 1" not in response_text
        assert "This is stderr output line 2" not in response_text
        assert "Error message: Something went wrong" not in response_text
        # 標準出力の内容は含まれていることを確認
        assert "This is stdout output" in response_text
        # CGIスクリプトのソースコードが返されていないことを確認
        assert "#!/usr/bin/env python3" not in response_text
        assert "import sys" not in response_text
