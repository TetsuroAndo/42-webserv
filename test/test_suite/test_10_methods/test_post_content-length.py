"""
POST メソッドの Content-Length 検証テスト
Content-Length と実際のボディサイズが一致しない場合のテスト
"""
import pytest
import subprocess
import re


@pytest.mark.config("valid/post_test_content-length.yaml")
def test_post_content_length_mismatch_too_large(managed_server):
    """
    Content-Length: 0~20 と指定しているのに、実際にはボディを送信する不正なリクエスト。
    サーバーは 400 Bad Request を返すべき。
    """
    host = "127.0.0.1"
    port = managed_server["port"]

    for length in range(21):

        # printf と curl を使ってリクエストを送信
        # Content-Length: 0 と指定しているが、実際には ABCDEFG というボディを送信
        # printf の %b フォーマットを使って \r\n を正しく処理
        http_request = (
            f"POST /upload HTTP/1.1\\r\\n"
            f"Host: {host}:{port}\\r\\n"
            f"Content-Type: text/plain\\r\\n"
            f"Content-Length: {length}\\r\\n"
            f"Connection: close\\r\\n"
            f"\\r\\n"
            f"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        )

        # printf %b と curl をパイプでつなげて実行
        cmd = f"printf '%b' '{http_request}' | curl --no-buffer --raw -v telnet://{host}:{port}"

        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=5
        )

        # curl の出力から HTTP ステータスコードを抽出
        output = result.stderr + result.stdout

        # HTTP/1.1 400 のようなパターンを検索
        status_match = re.search(r'HTTP/1\.\d+\s+(\d+)', output)
        assert status_match is not None, f"HTTP status code not found in output:\n{output}"

        status_code = int(status_match.group(1))
        assert status_code == 400, (
            f"Expected 400 Bad Request, but got {status_code}.\n"
            f"Output:\n{output}"
        )


@pytest.mark.config("valid/post_test_content-length.yaml")
def test_post_content_length_mismatch_no_body(managed_server):
    """
    Content-Length: 7 と指定しているのに、実際にはボディを送信しない不正なリクエスト。
    サーバーはタイムアウトで接続を閉じるべき。
    """
    host = "127.0.0.1"
    port = managed_server["port"]

    # Content-Length: 7 と指定しているが、ボディが空
    http_request = (
        f"POST /upload HTTP/1.1\\r\\n"
        f"Host: {host}:{port}\\r\\n"
        f"Content-Type: text/plain\\r\\n"
        f"Content-Length: 7\\r\\n"
        f"Connection: close\\r\\n"
        f"\\r\\n"
    )

    cmd = f"printf '%b' '{http_request}' | curl --no-buffer --raw -v telnet://{host}:{port}"

    try:
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=5
        )

        output = result.stderr + result.stdout
        status_match = re.search(r'HTTP/1\.\d+\s+(\d+)', output)

        if status_match is not None:
            status_code = int(status_match.group(1))
            pytest.fail(
                f"Expected timeout, but got {status_code}. Output:\n{output}"
            )

    except subprocess.TimeoutExpired:
        # タイムアウトで接続を閉じることは許容される動作
        # サーバーがContent-Lengthで指定されたバイト数を待ち続けてタイムアウトし、
        # 接続を閉じた場合は正常な動作として扱う
        pass


@pytest.mark.config("valid/post_test_content-length.yaml")
def test_post_content_length_mismatch_too_small(managed_server):
    """
    Content-Length: 10 と指定しているのに、実際には3バイトしかボディを送信しない不正なリクエスト。
    サーバーはタイムアウトで接続を閉じるべき。
    """
    host = "127.0.0.1"
    port = managed_server["port"]

    # Content-Length: 10 と指定しているが、実際には "ABC" (3バイト) しか送信しない
    http_request = (
        f"POST /upload HTTP/1.1\\r\\n"
        f"Host: {host}:{port}\\r\\n"
        f"Content-Type: text/plain\\r\\n"
        f"Content-Length: 10\\r\\n"
        f"Connection: close\\r\\n"
        f"\\r\\n"
        f"ABC"
    )

    cmd = f"printf '%b' '{http_request}' | curl --no-buffer --raw -v telnet://{host}:{port}"

    try:
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=5
        )

        output = result.stderr + result.stdout
        status_match = re.search(r'HTTP/1\.\d+\s+(\d+)', output)

        if status_match is not None:
            status_code = int(status_match.group(1))
            pytest.fail(
                f"Expected timeout, but got {status_code}. Output:\n{output}"
            )
    except subprocess.TimeoutExpired:
        # タイムアウトで接続を閉じることは許容される動作
        # サーバーがContent-Lengthで指定されたバイト数を待ち続けてタイムアウトし、
        # 接続を閉じた場合は正常な動作として扱う
        pass
