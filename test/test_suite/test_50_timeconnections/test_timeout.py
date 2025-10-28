"""
タイムアウト処理のテスト
"""
import pytest
import socket
import time


class TestTimeout:
    @pytest.mark.config("valid/config_timeout.yaml")
    def test_request_header_timeout(self, managed_server):
        """
        リクエストラインを送信した後、ヘッダーを送信する前にタイムアウトすることを確認
        """
        # ソケット接続を確立
        port = 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect(("127.0.0.1", port))

        try:
            # リクエストラインのみ送信（ヘッダーは送信しない）
            sock.sendall(b"GET / HTTP/1.1\r\n")

            # 設定されたタイムアウト時間（2秒）+ 余裕を見て3秒待つ
            # タイムアウトが発生すれば接続が閉じられる
            time.sleep(3)

            # 接続が閉じられているか確認
            # recvまたはsendでエラーが出ることを期待
            try:
                sock.sendall(b"test")
                # 接続が開いていた場合はfail
                sock.close()
                pytest.fail("Connection was not closed after timeout")
            except (BrokenPipeError, ConnectionResetError, OSError):
                # 接続が閉じられていることが確認できた
                pass
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_request_body_timeout(self, managed_server):
        """
        リクエストボディーを受信する間にタイムアウトすることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        try:
            # リクエストヘッダーを送信
            sock.sendall(b"POST / HTTP/1.1\r\n")
            sock.sendall(b"Host: 127.0.0.1:8080\r\n")
            sock.sendall(b"Content-Type: text/plain\r\n")
            sock.sendall(b"Content-Length: 100\r\n")
            sock.sendall(b"\r\n")

            # ボディーの一部のみ送信して待つ
            sock.sendall(b"A" * 50)  # ボディーの半分だけ送信

            # タイムアウトが発生するまで待つ
            time.sleep(3)

            # 接続が閉じられているか確認
            try:
                sock.sendall(b"test")
                sock.close()
                pytest.fail("Connection was not closed after timeout")
            except (BrokenPipeError, ConnectionResetError, OSError):
                # 接続が閉じられている
                pass
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_keepalive_idle_timeout(self, managed_server):
        """
        リクエスト完了後、Keep-Alive接続でアイドル状態がタイムアウトすることを確認
        """
        host, port = "127.0.0.1", 8080
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((host, port))

        try:
            # 正常なリクエストを送信して完了させる
            request = b"GET / HTTP/1.1\r\n"
            request += b"Host: 127.0.0.1:8080\r\n"
            request += b"Connection: keep-alive\r\n"
            request += b"\r\n"

            sock.sendall(request)

            # レスポンスを受信
            response = sock.recv(4096)
            assert len(response) > 0, "Response should be received"

            # 少し待ってから、アイドルタイムアウトを待つ
            # 設定されたタイムアウト（3秒）+ 余裕を見て4秒待つ
            time.sleep(4)

            # 接続が閉じられているか確認
            try:
                # 新しいリクエストを送信してみる
                sock.sendall(request)
                # 接続が生きていれば、レスポンスが返ってくるはず
                # ただし、タイムアウト後のため接続が閉じられている
                try:
                    sock.recv(4096)
                    sock.close()
                    pytest.fail("Connection was not closed after idle timeout")
                except (BrokenPipeError, ConnectionResetError, socket.timeout):
                    # 接続が閉じられている
                    pass
            except (BrokenPipeError, ConnectionResetError, OSError):
                # 接続が閉じられている
                pass
        finally:
            sock.close()

    @pytest.mark.config("valid/config_timeout.yaml")
    def test_slow_request_not_affecting_other_clients(self, managed_server):
        """
        遅いリクエストが他のクライアントに影響を与えないことを確認（ノンブロッキング）
        """
        # 1つ目のクライアント - 遅いリクエストを送信
        host, port = "127.0.0.1", 8080
        slow_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        slow_sock.settimeout(5)
        slow_sock.connect((host, port))

        try:
            # リクエストの一部だけ送信して遅延させる
            slow_sock.sendall(b"GET /")
            time.sleep(1)  # 少し遅延

            # 2つ目のクライアント - 通常の高速リクエスト
            fast_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            fast_sock.settimeout(5)
            fast_sock.connect((host, port))

            try:
                request = b"GET / HTTP/1.1\r\n"
                request += b"Host: 127.0.0.1:8080\r\n"
                request += b"\r\n"

                # 高速リクエストを送信
                fast_sock.sendall(request)

                # すぐにレスポンスが返ってくることを確認
                response = fast_sock.recv(4096)
                assert len(response) > 0, "Fast request should get immediate response"

            finally:
                fast_sock.close()
        finally:
            slow_sock.close()
